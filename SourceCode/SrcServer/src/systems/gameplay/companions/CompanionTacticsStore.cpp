#include "systems/gameplay/companions/CompanionTacticsStore.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

namespace gameplay
{
namespace companions
{
namespace
{

static const uint32_t COMPANION_TACTICS_MAGIC = 0x32435443u; // "CTC2" little-endian
static const uint16_t COMPANION_TACTICS_VERSION = 1;
static const uint32_t COMPANION_TACTICS_ENABLED = 0x00000001u;

#pragma pack(push, 1)
struct CompanionTacticRuleDiskV1
{
    uint8_t enabled;
    uint8_t condition;
    uint8_t action;
    uint8_t threshold;
};

struct CompanionTacticsDiskV1
{
    uint32_t magic;
    uint16_t version;
    uint16_t size;
    uint32_t accountHash;
    uint32_t characterHash;
    uint32_t flags;
    CompanionTacticRuleDiskV1 rules[protocol::COMPANION_UI_TACTIC_RULES];
    uint32_t checksum;
};
#pragma pack(pop)

static_assert(sizeof(CompanionTacticsDiskV1) == 40, "custom tactics sidecar ABI changed unexpectedly");

uint32_t HashIdentity(const char *text)
{
    uint32_t value = 2166136261u;
    if(!text)
        return value;

    for(const unsigned char *p = reinterpret_cast<const unsigned char *>(text); *p; ++p)
    {
        unsigned char c = *p;
        if(c >= 'A' && c <= 'Z')
            c = static_cast<unsigned char>(c - 'A' + 'a');
        value ^= c;
        value *= 16777619u;
    }
    return value;
}

uint32_t ComputeChecksum(const CompanionTacticsDiskV1 &disk)
{
    const unsigned char *bytes = reinterpret_cast<const unsigned char *>(&disk);
    const size_t length = offsetof(CompanionTacticsDiskV1, checksum);
    uint32_t value = 2166136261u;
    for(size_t i = 0; i < length; ++i)
    {
        value ^= bytes[i];
        value *= 16777619u;
    }
    return value;
}

int ClampInt(int value, int minimum, int maximum)
{
    if(value < minimum) return minimum;
    if(value > maximum) return maximum;
    return value;
}

bool BuildTacticsPath(const char *account, const char *characterName, char (&path)[MAX_PATH])
{
    if(!account || !account[0] || !characterName || !characterName[0])
        return false;

    const uint32_t accountHash = HashIdentity(account);
    const uint32_t characterHash = HashIdentity(characterName);

    CreateDirectoryA("DataServer", NULL);
    CreateDirectoryA("DataServer\\CompanionProfiles", NULL);

    char bucket[MAX_PATH] = { 0 };
    _snprintf_s(bucket, sizeof(bucket), _TRUNCATE,
        "DataServer\\CompanionProfiles\\%08X", static_cast<unsigned>(accountHash));
    if(!CreateDirectoryA(bucket, NULL))
    {
        const DWORD error = GetLastError();
        if(error != ERROR_ALREADY_EXISTS)
            return false;
    }

    _snprintf_s(path, sizeof(path), _TRUNCATE,
        "%s\\%08X.tac", bucket, static_cast<unsigned>(characterHash));
    return path[0] != 0;
}

bool IsConditionValid(int condition)
{
    return condition >= protocol::COMPANION_TACTIC_CONDITION_ALWAYS &&
        condition <= protocol::COMPANION_TACTIC_CONDITION_PARTY_THREATENED;
}

bool IsActionValid(int action)
{
    return action >= protocol::COMPANION_TACTIC_ACTION_ATTACK_OWNER_TARGET &&
        action <= protocol::COMPANION_TACTIC_ACTION_HOLD_POSITION;
}

int NormalizeThreshold(int condition, int threshold)
{
    switch(condition)
    {
        case protocol::COMPANION_TACTIC_CONDITION_SELF_HP_BELOW:
        case protocol::COMPANION_TACTIC_CONDITION_OWNER_HP_BELOW:
            return ClampInt(threshold, 5, 95);
        case protocol::COMPANION_TACTIC_CONDITION_ENEMIES_NEARBY_AT_LEAST:
            return ClampInt(threshold, 1, 8);
        default:
            return 0;
    }
}

} // namespace

CompanionTacticsConfig::CompanionTacticsConfig()
    : enabled(false)
{
    ZeroMemory(rules, sizeof(rules));

    // Useful Dragon-Age-style defaults, disabled until the user opts in.
    rules[0].enabled = TRUE;
    rules[0].condition = protocol::COMPANION_TACTIC_CONDITION_SELF_HP_BELOW;
    rules[0].action = protocol::COMPANION_TACTIC_ACTION_RETREAT;
    rules[0].threshold = 25;

    rules[1].enabled = TRUE;
    rules[1].condition = protocol::COMPANION_TACTIC_CONDITION_OWNER_HAS_TARGET;
    rules[1].action = protocol::COMPANION_TACTIC_ACTION_ATTACK_OWNER_TARGET;
    rules[1].threshold = 0;

    rules[2].enabled = TRUE;
    rules[2].condition = protocol::COMPANION_TACTIC_CONDITION_PARTY_THREATENED;
    rules[2].action = protocol::COMPANION_TACTIC_ACTION_DEFEND_PARTY;
    rules[2].threshold = 0;

    rules[3].enabled = TRUE;
    rules[3].condition = protocol::COMPANION_TACTIC_CONDITION_ALWAYS;
    rules[3].action = protocol::COMPANION_TACTIC_ACTION_ATTACK_NEAREST;
    rules[3].threshold = 0;
}

bool LoadCompanionTactics(const char *account, const char *characterName, CompanionTacticsConfig *config)
{
    if(!config)
        return false;

    *config = CompanionTacticsConfig();

    char path[MAX_PATH] = { 0 };
    if(!BuildTacticsPath(account, characterName, path))
        return false;

    FILE *file = NULL;
    if(fopen_s(&file, path, "rb") != 0 || !file)
        return false;

    CompanionTacticsDiskV1 disk;
    ZeroMemory(&disk, sizeof(disk));
    const size_t read = fread(&disk, 1, sizeof(disk), file);
    fclose(file);

    if(read != sizeof(disk) ||
       disk.magic != COMPANION_TACTICS_MAGIC ||
       disk.version != COMPANION_TACTICS_VERSION ||
       disk.size != sizeof(disk) ||
       disk.accountHash != HashIdentity(account) ||
       disk.characterHash != HashIdentity(characterName) ||
       disk.checksum != ComputeChecksum(disk))
        return false;

    config->enabled = (disk.flags & COMPANION_TACTICS_ENABLED) != 0;
    for(int i = 0; i < protocol::COMPANION_UI_TACTIC_RULES; ++i)
    {
        protocol::CompanionUITacticRule &rule = config->rules[i];
        const CompanionTacticRuleDiskV1 &saved = disk.rules[i];
        rule.enabled = saved.enabled ? TRUE : FALSE;
        rule.condition = IsConditionValid(saved.condition) ? saved.condition : protocol::COMPANION_TACTIC_CONDITION_ALWAYS;
        rule.action = IsActionValid(saved.action) ? saved.action : protocol::COMPANION_TACTIC_ACTION_ATTACK_NEAREST;
        rule.threshold = NormalizeThreshold(rule.condition, saved.threshold);
    }
    return true;
}

bool SaveCompanionTactics(const char *account, const char *characterName, const CompanionTacticsConfig &config)
{
    char path[MAX_PATH] = { 0 };
    if(!BuildTacticsPath(account, characterName, path))
        return false;

    CompanionTacticsDiskV1 disk;
    ZeroMemory(&disk, sizeof(disk));
    disk.magic = COMPANION_TACTICS_MAGIC;
    disk.version = COMPANION_TACTICS_VERSION;
    disk.size = sizeof(disk);
    disk.accountHash = HashIdentity(account);
    disk.characterHash = HashIdentity(characterName);
    if(config.enabled)
        disk.flags |= COMPANION_TACTICS_ENABLED;

    for(int i = 0; i < protocol::COMPANION_UI_TACTIC_RULES; ++i)
    {
        const protocol::CompanionUITacticRule &rule = config.rules[i];
        CompanionTacticRuleDiskV1 &saved = disk.rules[i];
        saved.enabled = rule.enabled ? 1 : 0;
        saved.condition = IsConditionValid(rule.condition) ? (uint8_t)rule.condition : (uint8_t)protocol::COMPANION_TACTIC_CONDITION_ALWAYS;
        saved.action = IsActionValid(rule.action) ? (uint8_t)rule.action : (uint8_t)protocol::COMPANION_TACTIC_ACTION_ATTACK_NEAREST;
        saved.threshold = (uint8_t)NormalizeThreshold(saved.condition, rule.threshold);
    }
    disk.checksum = ComputeChecksum(disk);

    char tempPath[MAX_PATH] = { 0 };
    _snprintf_s(tempPath, sizeof(tempPath), _TRUNCATE, "%s.tmp", path);

    FILE *file = NULL;
    if(fopen_s(&file, tempPath, "wb") != 0 || !file)
        return false;
    const size_t written = fwrite(&disk, 1, sizeof(disk), file);
    const int flushResult = fflush(file);
    fclose(file);
    if(written != sizeof(disk) || flushResult != 0)
    {
        DeleteFileA(tempPath);
        return false;
    }

    if(!MoveFileExA(tempPath, path, MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
    {
        DeleteFileA(tempPath);
        return false;
    }
    return true;
}

} // namespace companions
} // namespace gameplay
