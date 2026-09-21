#include "systems/gameplay/companions/CompanionProfileStore.h"
#include "systems/gameplay/companions/CompanionSystem.h"

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

static const uint32_t COMPANION_PROFILE_MAGIC = 0x32504D43u; // "CMP2" little-endian
static const uint16_t COMPANION_PROFILE_VERSION = 2;
static const uint16_t COMPANION_PROFILE_LEGACY_VERSION = 1;
static const uint32_t COMPANION_PROFILE_LOADOUT_ACTIVE = 0x00000001u;

#pragma pack(push, 1)
struct CompanionProfileDiskV1
{
    uint32_t magic;
    uint16_t version;
    uint16_t size;
    uint32_t accountHash;
    uint32_t characterHash;
    uint32_t flags;
    uint32_t rightItemCode;
    uint32_t leftItemCode;
    int32_t mode;
    int32_t tactic;
    int32_t retreatHpPercent;
    int32_t preferredRange;
    int32_t formationSlot;
    uint32_t reserved[8];
    uint32_t checksum;
};
#pragma pack(pop)

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

uint32_t ComputeChecksum(const CompanionProfileDiskV1 &disk)
{
    const unsigned char *bytes = reinterpret_cast<const unsigned char *>(&disk);
    const size_t length = offsetof(CompanionProfileDiskV1, checksum);
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

bool IsModeValid(int mode)
{
    return mode >= static_cast<int>(COMPANION_MODE_AGGRESSIVE) &&
        mode <= static_cast<int>(COMPANION_MODE_PASSIVE);
}

bool IsTacticValid(int tactic)
{
    return tactic >= static_cast<int>(COMPANION_TACTIC_BALANCED) &&
        tactic <= static_cast<int>(COMPANION_TACTIC_HOLD_POSITION);
}

bool BuildProfilePath(const char *account, const char *characterName, char (&path)[MAX_PATH])
{
    if(!account || !account[0] || !characterName || !characterName[0])
        return false;

    const uint32_t accountHash = HashIdentity(account);
    const uint32_t characterHash = HashIdentity(characterName);

    CreateDirectoryA("DataServer", NULL);
    CreateDirectoryA("DataServer\\CompanionProfiles", NULL);

    char bucket[MAX_PATH];
    _snprintf_s(bucket, sizeof(bucket), _TRUNCATE,
        "DataServer\\CompanionProfiles\\%08X", static_cast<unsigned>(accountHash));
    if(!CreateDirectoryA(bucket, NULL))
    {
        const DWORD error = GetLastError();
        if(error != ERROR_ALREADY_EXISTS)
            return false;
    }

    _snprintf_s(path, sizeof(path), _TRUNCATE,
        "%s\\%08X.cmp", bucket, static_cast<unsigned>(characterHash));
    return path[0] != 0;
}

} // namespace

CompanionProfile::CompanionProfile()
    : loadoutActive(false), rightItemCode(0), leftItemCode(0),
      mode(static_cast<int>(COMPANION_MODE_AGGRESSIVE)),
      tactic(COMPANION_TACTIC_BALANCED), retreatHpPercent(0),
      preferredRange(0), formationSlot(-1)
{
}

bool LoadCompanionProfile(const char *account, const char *characterName, CompanionProfile *profile)
{
    if(!profile)
        return false;

    *profile = CompanionProfile();

    char path[MAX_PATH] = { 0 };
    if(!BuildProfilePath(account, characterName, path))
        return false;

    FILE *file = NULL;
    if(fopen_s(&file, path, "rb") != 0 || !file)
        return false;

    CompanionProfileDiskV1 disk;
    ZeroMemory(&disk, sizeof(disk));
    const size_t read = fread(&disk, 1, sizeof(disk), file);
    fclose(file);

    if(read != sizeof(disk) ||
        disk.magic != COMPANION_PROFILE_MAGIC ||
        (disk.version != COMPANION_PROFILE_VERSION &&
         disk.version != COMPANION_PROFILE_LEGACY_VERSION) ||
        disk.size != sizeof(disk) ||
        disk.accountHash != HashIdentity(account) ||
        disk.characterHash != HashIdentity(characterName) ||
        disk.checksum != ComputeChecksum(disk))
    {
        return false;
    }

    profile->loadoutActive = (disk.flags & COMPANION_PROFILE_LOADOUT_ACTIVE) != 0;
    profile->rightItemCode = disk.rightItemCode;
    profile->leftItemCode = disk.leftItemCode;
    profile->mode = IsModeValid(disk.mode) ? disk.mode : static_cast<int>(COMPANION_MODE_AGGRESSIVE);
    profile->tactic = IsTacticValid(disk.tactic) ?
        static_cast<CompanionTacticPreset>(disk.tactic) : COMPANION_TACTIC_BALANCED;
    // V1 shipped with an implicit 25% retreat default. Treat that legacy
    // value as disabled during migration: retreat becomes opt-in only. Any
    // explicit value saved after this build is written as V2 and is honored.
    profile->retreatHpPercent = disk.version >= COMPANION_PROFILE_VERSION ?
        ClampInt(disk.retreatHpPercent, 0, 90) : 0;
    profile->preferredRange = ClampInt(disk.preferredRange, 0, 600);
    profile->formationSlot = ClampInt(disk.formationSlot, -1, COMPANION_MAX_ACTIVE - 1);
    return true;
}

bool SaveCompanionProfile(const char *account, const char *characterName, const CompanionProfile &profile)
{
    char path[MAX_PATH] = { 0 };
    if(!BuildProfilePath(account, characterName, path))
        return false;

    CompanionProfileDiskV1 disk;
    ZeroMemory(&disk, sizeof(disk));
    disk.magic = COMPANION_PROFILE_MAGIC;
    disk.version = COMPANION_PROFILE_VERSION;
    disk.size = sizeof(disk);
    disk.accountHash = HashIdentity(account);
    disk.characterHash = HashIdentity(characterName);
    if(profile.loadoutActive)
        disk.flags |= COMPANION_PROFILE_LOADOUT_ACTIVE;
    disk.rightItemCode = profile.rightItemCode;
    disk.leftItemCode = profile.leftItemCode;
    disk.mode = IsModeValid(profile.mode) ? profile.mode : static_cast<int>(COMPANION_MODE_AGGRESSIVE);
    disk.tactic = IsTacticValid(static_cast<int>(profile.tactic)) ?
        static_cast<int>(profile.tactic) : static_cast<int>(COMPANION_TACTIC_BALANCED);
    disk.retreatHpPercent = ClampInt(profile.retreatHpPercent, 0, 90);
    disk.preferredRange = ClampInt(profile.preferredRange, 0, 600);
    disk.formationSlot = ClampInt(profile.formationSlot, -1, COMPANION_MAX_ACTIVE - 1);
    disk.checksum = ComputeChecksum(disk);

    char tempPath[MAX_PATH] = { 0 };
    _snprintf_s(tempPath, sizeof(tempPath), _TRUNCATE, "%s.tmp", path);

    FILE *file = NULL;
    if(fopen_s(&file, tempPath, "wb") != 0 || !file)
        return false;

    const bool wrote = fwrite(&disk, 1, sizeof(disk), file) == sizeof(disk);
    const bool flushed = fflush(file) == 0;
    fclose(file);

    if(!wrote || !flushed)
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
