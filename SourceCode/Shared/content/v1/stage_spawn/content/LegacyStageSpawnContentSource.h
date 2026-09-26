#pragma once

#include <cctype>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

#include "content/v1/stage_spawn/content/IStageSpawnContentSource.h"

namespace summit
{
namespace content
{
namespace stage_spawn
{

namespace legacy_token
{
static const char Actor[] = "\x2A\xC3\xE2\xBF\xAC\xC0\xDA";
static const char BossActor[] = "\x2A\xC3\xE2\xBF\xAC\xC0\xDA\xB5\xCE\xB8\xF1";
static const char MaxActorPosition[] = "\x2A\xC3\xD6\xB4\xEB\xB5\xBF\xBD\xC3\xC3\xE2\xC7\xF6\xBC\xF6";
static const char Delay[] = "\x2A\xC3\xE2\xC7\xF6\xB0\xA3\xB0\xDD";
static const char MaxActor[] = "\x2A\xC3\xE2\xC7\xF6\xBC\xF6";
}

inline bool AsciiCaseEqual(const char *left, const char *right)
{
    if(!left || !right) return false;
    while(*left && *right)
    {
        unsigned char a = static_cast<unsigned char>(*left++);
        unsigned char b = static_cast<unsigned char>(*right++);
        if(a < 0x80) a = static_cast<unsigned char>(std::tolower(a));
        if(b < 0x80) b = static_cast<unsigned char>(std::tolower(b));
        if(a != b) return false;
    }
    return *left == 0 && *right == 0;
}

inline bool SpawnTokenEquals(const char *value, const char *legacy, const char *ascii)
{
    return value && ((legacy && std::strcmp(value,legacy)==0) || (ascii && AsciiCaseEqual(value,ascii)));
}

// Small byte lexer intentionally independent of the process code page.
class SpawnLineCursor
{
public:
    explicit SpawnLineCursor(char *line) : p_(line ? line : EmptyBuffer()) {}

    bool Next(std::string &out)
    {
        out.clear();
        SkipSpaces();
        if(!*p_ || (p_[0]=='/' && p_[1]=='/')) return false;
        if(*p_=='"')
        {
            ++p_;
            while(*p_ && *p_!='"') out.push_back(*p_++);
            if(*p_=='"') ++p_;
            return true;
        }
        while(*p_ && *p_!=' ' && *p_!='\t' && *p_!='\r' && *p_!='\n')
        {
            if(p_[0]=='/' && p_[1]=='/') break;
            out.push_back(*p_++);
        }
        return !out.empty();
    }

private:
    static char *EmptyBuffer() { static char empty[1] = {0}; return empty; }
    void SkipSpaces() { while(*p_==' ' || *p_=='\t' || *p_=='\r' || *p_=='\n') ++p_; }
    char *p_;
};

class LegacyStageSpawnContentSource : public IStageSpawnContentSource
{
public:
    virtual bool Load(const char *logicalPath,
                      StageSpawnDefinition &out,
                      std::string *error = 0) const
    {
        if(!logicalPath || !*logicalPath)
            return Fail(error,"stage spawn path is empty");

        FILE *fp = std::fopen(logicalPath,"rb");
        if(!fp)
            return Fail(error,"legacy stage spawn file not found");

        StageSpawnDefinition staged;
        staged.logicalPath = logicalPath;
        char line[512];
        while(std::fgets(line,sizeof(line),fp))
        {
            SpawnLineCursor cursor(line);
            std::string command;
            if(!cursor.Next(command)) continue;

            if(SpawnTokenEquals(command.c_str(),legacy_token::Actor,"*ACTOR"))
            {
                StageSpawnActorDefinition actor;
                std::string weight;
                if(!cursor.Next(actor.legacyNameBytes)) { std::fclose(fp); return Fail(error,"ACTOR has no name"); }
                if(cursor.Next(weight)) actor.weight = std::atoi(weight.c_str());
                staged.actors.push_back(actor);
                continue;
            }
            if(SpawnTokenEquals(command.c_str(),legacy_token::BossActor,"*BOSS_ACTOR"))
            {
                StageSpawnBossDefinition boss;
                std::string value;
                if(!cursor.Next(boss.masterLegacyNameBytes) || !cursor.Next(boss.slaveLegacyNameBytes))
                { std::fclose(fp); return Fail(error,"BOSS_ACTOR is incomplete"); }
                if(cursor.Next(value)) boss.slaveCount = std::atoi(value.c_str());
                while(cursor.Next(value)) boss.openHours.push_back(static_cast<std::uint32_t>(std::atoi(value.c_str())));
                staged.bosses.push_back(boss);
                continue;
            }
            if(SpawnTokenEquals(command.c_str(),legacy_token::MaxActorPosition,"*MAX_ACTOR_POS"))
            {
                std::string value; if(cursor.Next(value)) staged.limitMax=std::atoi(value.c_str()); continue;
            }
            if(SpawnTokenEquals(command.c_str(),legacy_token::Delay,"*DELAY"))
            {
                std::string value;
                int shift=0;
                if(cursor.Next(value)) shift=std::atoi(value.c_str());
                if(shift < 0 || shift >= 30) { std::fclose(fp); return Fail(error,"DELAY shift is out of range"); }
                staged.openInterval=(1 << shift);
                if(staged.openInterval > 1) --staged.openInterval;
                if(cursor.Next(value))
                {
                    const long seconds=std::strtol(value.c_str(),0,10);
                    if(seconds < 0 || static_cast<unsigned long>(seconds) > 0xFFFFFFFFu/1000u)
                    { std::fclose(fp); return Fail(error,"DELAY seconds are out of range"); }
                    staged.intervalMilliseconds=static_cast<std::uint32_t>(seconds)*1000u;
                }
                continue;
            }
            if(SpawnTokenEquals(command.c_str(),legacy_token::MaxActor,"*MAX_ACTOR"))
            {
                std::string value; if(cursor.Next(value)) staged.openLimit=std::atoi(value.c_str()); continue;
            }
        }
        std::fclose(fp);
        out = staged;
        if(error) error->clear();
        return true;
    }

private:
    static bool Fail(std::string *error,const char *message)
    {
        if(error) *error=message ? message : "legacy stage spawn load failed";
        return false;
    }
};

} // namespace stage_spawn
} // namespace content
} // namespace summit
