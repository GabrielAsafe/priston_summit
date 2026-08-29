#include <cstdio>
#include <cstring>

enum Command
{
    CMD_EQUIPPED_WEAPON = 11,
    CMD_APPLICABLE_JOB  = 12,
    CMD_LOCATION        = 13,
    CMD_APPLICABLE_SKILL = 14,
    CMD_UNKNOWN = -1
};

static Command LegacyCommand(const char *s)
{
    // Bytes CP949 dos comandos originais serão preenchidos
    // depois diretamente a partir do master.
    return CMD_UNKNOWN;
}

static Command EnglishCommand(const char *s)
{
    if (strcmp(s, "*EQUIPPED_WEAPON") == 0)
        return CMD_EQUIPPED_WEAPON;

    if (strcmp(s, "*APPLICABLE_JOB") == 0)
        return CMD_APPLICABLE_JOB;

    if (strcmp(s, "*LOCATION") == 0)
        return CMD_LOCATION;

    if (strcmp(s, "*APPLICABLE_SKILL") == 0)
        return CMD_APPLICABLE_SKILL;

    return CMD_UNKNOWN;
}

enum WeaponSpecial
{
    WEAPON_NORMAL,
    WEAPON_ALL,
    WEAPON_NONE
};

static WeaponSpecial EnglishWeapon(const char *s)
{
    if (strcmp(s, "ALL") == 0)
        return WEAPON_ALL;

    if (strcmp(s, "NONE") == 0)
        return WEAPON_NONE;

    return WEAPON_NORMAL;
}

enum Location
{
    LOCATION_UNKNOWN = 0,
    LOCATION_TOWN,
    LOCATION_FIELD
};

static Location EnglishLocation(const char *s)
{
    if (strcmp(s, "TOWN") == 0)
        return LOCATION_TOWN;

    if (strcmp(s, "FIELD") == 0)
        return LOCATION_FIELD;

    return LOCATION_UNKNOWN;
}

int main()
{
    struct {
        const char *text;
        Command expected;
    } commands[] = {
        { "*EQUIPPED_WEAPON",  CMD_EQUIPPED_WEAPON },
        { "*APPLICABLE_JOB",   CMD_APPLICABLE_JOB },
        { "*LOCATION",         CMD_LOCATION },
        { "*APPLICABLE_SKILL", CMD_APPLICABLE_SKILL },
    };

    int failures = 0;

    for (const auto &t : commands)
    {
        Command got = EnglishCommand(t.text);

        printf("%-20s -> %d expected=%d %s\n",
               t.text,
               (int)got,
               (int)t.expected,
               got == t.expected ? "PASS" : "FAIL");

        if (got != t.expected)
            failures++;
    }

    printf("ALL  -> %d\n",  (int)EnglishWeapon("ALL"));
    printf("NONE -> %d\n",  (int)EnglishWeapon("NONE"));
    printf("TOWN -> %d\n",  (int)EnglishLocation("TOWN"));
    printf("FIELD -> %d\n", (int)EnglishLocation("FIELD"));

    return failures != 0;
}
