#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace summit { namespace content { namespace field_runtime {

struct StageStartPointPlacement
{
    std::uint32_t slot=0;
    std::int32_t state=0,x=0,z=0;
};

struct FixedNpcPlacement
{
    std::uint32_t slot=0;
    std::string npcId;
    std::string definitionPath;
    std::int32_t x=0,y=0,z=0,ax=0,ay=0,az=0,state=0;
};

struct StageStartPointSet
{
    std::string id,logicalPath;
    std::vector<StageStartPointPlacement> points;
};

struct FixedNpcSet
{
    std::string id,logicalPath;
    std::vector<FixedNpcPlacement> npcs;
};

}}}
