#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

#include "content/v1/map/content/CompiledMapContentSource.h"

namespace summit
{
namespace content
{
namespace map
{

struct LegacyMapPathContext
{
    const char *fieldDirectory;
    const char *mapDirectory;
    const char *titleDirectory;

    LegacyMapPathContext(const char *fieldDir=0,const char *mapDir=0,const char *titleDir=0)
        : fieldDirectory(fieldDir?fieldDir:""), mapDirectory(mapDir?mapDir:""), titleDirectory(titleDir?titleDir:"") {}
};

inline bool MapStageObjectEnabled(const MapStageObjectDefinition &item,std::uint32_t runtimeFeatureMask)
{
    return (item.requireFeatureMask & runtimeFeatureMask)==item.requireFeatureMask &&
           (item.forbidFeatureMask & runtimeFeatureMask)==0;
}

inline bool MapAdapterFail(std::string *error,const char *message)
{
    if(error) *error=message?message:"legacy map materialization failed";
    return false;
}

template <typename LegacyField>
inline bool ValidateLegacyMapCatalog(const MapCatalogDefinition &catalog,
                                     MapProfile expectedProfile,
                                     std::uint32_t runtimeFeatureMask,
                                     LegacyField *fields,
                                     int fieldCapacity,
                                     const LegacyMapPathContext &paths,
                                     std::string *error=0)
{
    if(!fields || fieldCapacity<=0) return MapAdapterFail(error,"invalid legacy field storage");
    if(catalog.profile!=expectedProfile) return MapAdapterFail(error,"compiled map profile does not match runtime profile");
    if(catalog.dynamicFieldCode<0 || catalog.dynamicFieldCode>=fieldCapacity)
        return MapAdapterFail(error,"compiled map dynamic field code is outside legacy storage");
    if(catalog.staticFieldCount==0 || catalog.staticFieldCount>static_cast<std::uint32_t>(fieldCapacity))
        return MapAdapterFail(error,"compiled map static field count is invalid");
    if(catalog.maps.size()!=catalog.staticFieldCount)
        return MapAdapterFail(error,"compiled map catalog does not contain exactly one record per static field");
    if((runtimeFeatureMask & ~catalog.featureMask)!=0)
        return MapAdapterFail(error,"runtime enables a map feature not declared by the compiled catalog");

    std::vector<const MapDefinition*> byCode(catalog.staticFieldCount,static_cast<const MapDefinition*>(0));
    std::vector<std::size_t> finalGateCounts(static_cast<std::size_t>(fieldCapacity),0);

    const std::size_t fieldNameCapacity=sizeof(fields[0].szName);
    const std::size_t mapNameCapacity=sizeof(fields[0].szNameMap);
    const std::size_t titleNameCapacity=sizeof(fields[0].szNameTitle);
    const std::size_t stageObjectCapacity=sizeof(fields[0].lpStageObjectName)/sizeof(fields[0].lpStageObjectName[0]);
    const std::size_t startPointCapacity=sizeof(fields[0].StartPoint)/sizeof(fields[0].StartPoint[0]);
    const std::size_t gateCapacity=sizeof(fields[0].FieldGate)/sizeof(fields[0].FieldGate[0]);
    const std::size_t warpGateCapacity=sizeof(fields[0].WarpGate)/sizeof(fields[0].WarpGate[0]);
    const std::size_t ambientCapacity=sizeof(fields[0].AmbientPos)/sizeof(fields[0].AmbientPos[0]);

    const std::size_t fieldPrefix=std::strlen(paths.fieldDirectory);
    const std::size_t mapPrefix=std::strlen(paths.mapDirectory);
    const std::size_t titlePrefix=std::strlen(paths.titleDirectory);

    for(std::size_t i=0;i<catalog.maps.size();++i)
    {
        const MapDefinition &map=catalog.maps[i];
        if(map.legacyFieldCode<0 || static_cast<std::uint32_t>(map.legacyFieldCode)>=catalog.staticFieldCount)
            return MapAdapterFail(error,"compiled map field code is outside the static range");
        if(byCode[static_cast<std::size_t>(map.legacyFieldCode)])
            return MapAdapterFail(error,"compiled map contains a duplicate field code");
        byCode[static_cast<std::size_t>(map.legacyFieldCode)]=&map;

        if(fieldPrefix+map.stagePath.size()+1>fieldNameCapacity)
            return MapAdapterFail(error,"compiled map stage path exceeds legacy field-name capacity");
        if(map.hasMapImageId)
        {
            if(mapPrefix+map.mapImageId.size()+4+1>mapNameCapacity)
                return MapAdapterFail(error,"compiled map image id exceeds legacy map-name capacity");
            if(titlePrefix+map.mapImageId.size()+5+1>titleNameCapacity)
                return MapAdapterFail(error,"compiled map image id exceeds legacy title-name capacity");
        }

        std::size_t enabledStageObjects=0;
        for(std::size_t s=0;s<map.stageObjects.size();++s)
        {
            const MapStageObjectDefinition &item=map.stageObjects[s];
            if((item.requireFeatureMask|item.forbidFeatureMask) & ~catalog.featureMask)
                return MapAdapterFail(error,"compiled map stage object references an undeclared feature bit");
            if(!MapStageObjectEnabled(item,runtimeFeatureMask)) continue;
            ++enabledStageObjects;
            if(fieldPrefix+item.assetPath.size()+1>fieldNameCapacity)
                return MapAdapterFail(error,"compiled map stage-object path exceeds legacy capacity");
        }
        if(enabledStageObjects>stageObjectCapacity)
            return MapAdapterFail(error,"compiled map exceeds legacy stage-object capacity");
        if(map.startPoints.size()>startPointCapacity)
            return MapAdapterFail(error,"compiled map exceeds legacy start-point capacity");
        if(map.gates.size()>gateCapacity)
            return MapAdapterFail(error,"compiled map exceeds legacy gate capacity");
        if(map.warpGates.size()>warpGateCapacity)
            return MapAdapterFail(error,"compiled map exceeds legacy warp-gate capacity");
        if(map.ambient.size()>ambientCapacity)
            return MapAdapterFail(error,"compiled map exceeds legacy ambient capacity");
        if(map.hasLegacyOrphanWarpLimitLevel && warpGateCapacity==0)
            return MapAdapterFail(error,"legacy field has no warp-gate slot for preserved orphan property");

        finalGateCounts[static_cast<std::size_t>(map.legacyFieldCode)]+=map.gates.size();
        for(std::size_t g=0;g<map.gates.size();++g)
        {
            const int target=map.gates[g].targetLegacyFieldCode;
            if(target<0 || target>=fieldCapacity)
                return MapAdapterFail(error,"compiled map gate target is outside legacy field storage");
            if(static_cast<std::uint32_t>(target)>=catalog.staticFieldCount && target!=catalog.dynamicFieldCode)
                return MapAdapterFail(error,"compiled map gate targets an unknown non-static field");
            ++finalGateCounts[static_cast<std::size_t>(target)];
        }
        for(std::size_t w=0;w<map.warpGates.size();++w)
        {
            if(map.warpGates[w].outGates.size()>gateCapacity)
                return MapAdapterFail(error,"compiled map warp gate exceeds legacy out-gate capacity");
            for(std::size_t g=0;g<map.warpGates[w].outGates.size();++g)
            {
                const int target=map.warpGates[w].outGates[g].targetLegacyFieldCode;
                if(target<0 || target>=fieldCapacity)
                    return MapAdapterFail(error,"compiled map warp-out target is outside legacy field storage");
                if(static_cast<std::uint32_t>(target)>=catalog.staticFieldCount && target!=catalog.dynamicFieldCode)
                    return MapAdapterFail(error,"compiled map warp-out targets an unknown non-static field");
            }
        }
    }

    for(std::size_t i=0;i<byCode.size();++i)
        if(!byCode[i]) return MapAdapterFail(error,"compiled map static field range is not contiguous");
    for(std::size_t i=0;i<finalGateCounts.size();++i)
        if(finalGateCounts[i]>gateCapacity)
            return MapAdapterFail(error,"compiled map reciprocal gates exceed legacy gate capacity");

    if(error) error->clear();
    return true;
}

template <typename LegacyField>
inline bool MaterializeLegacyMapCatalog(const MapCatalogDefinition &catalog,
                                        MapProfile expectedProfile,
                                        std::uint32_t runtimeFeatureMask,
                                        LegacyField *fields,
                                        int fieldCapacity,
                                        const LegacyMapPathContext &paths,
                                        std::string *error=0)
{
    if(!ValidateLegacyMapCatalog(catalog,expectedProfile,runtimeFeatureMask,fields,fieldCapacity,paths,error))
        return false;

    std::vector<const MapDefinition*> byCode(catalog.staticFieldCount,static_cast<const MapDefinition*>(0));
    for(std::size_t i=0;i<catalog.maps.size();++i)
        byCode[static_cast<std::size_t>(catalog.maps[i].legacyFieldCode)]=&catalog.maps[i];

    for(std::size_t code=0;code<byCode.size();++code)
    {
        const MapDefinition &src=*byCode[code];
        LegacyField &dst=fields[code];
        dst.SetName(const_cast<char*>(src.stagePath.c_str()),
                    src.hasMapImageId ? const_cast<char*>(src.mapImageId.c_str()) : 0);
        dst.State=src.state;
        dst.BackImageCode[0]=src.backImageCode[0];
        dst.BackImageCode[1]=src.backImageCode[1];
        dst.BackImageCode[2]=src.backImageCode[2];
        dst.BackMusicCode=src.backMusicCode;
        dst.FieldEvent=src.fieldEvent;
        dst.ServerCode=src.serverCode;
        dst.LimitLevel=src.limitLevel;
        dst.FieldSight=src.fieldSight;
        dst.SetCenterPos(src.centerX,src.centerZ);

        for(std::size_t s=0;s<src.stageObjects.size();++s)
        {
            const MapStageObjectDefinition &item=src.stageObjects[s];
            if(!MapStageObjectEnabled(item,runtimeFeatureMask)) continue;
            if(!dst.AddStageObject(const_cast<char*>(item.assetPath.c_str()),item.bipAnimation))
                return MapAdapterFail(error,"legacy AddStageObject rejected compiled map data");
        }
        for(std::size_t p=0;p<src.startPoints.size();++p)
            if(!dst.AddStartPoint(src.startPoints[p].x,src.startPoints[p].z))
                return MapAdapterFail(error,"legacy AddStartPoint rejected compiled map data");
        for(std::size_t g=0;g<src.gates.size();++g)
        {
            const MapGateDefinition &gate=src.gates[g];
            if(!dst.AddGate(&fields[gate.targetLegacyFieldCode],gate.x,gate.z,gate.y))
                return MapAdapterFail(error,"legacy AddGate rejected compiled map data");
        }
        for(std::size_t w=0;w<src.warpGates.size();++w)
        {
            const MapWarpGateDefinition &warp=src.warpGates[w];
            if(!dst.AddWarpGate(warp.x,warp.z,warp.y,warp.size,warp.height))
                return MapAdapterFail(error,"legacy AddWarpGate rejected compiled map data");
            for(std::size_t g=0;g<warp.outGates.size();++g)
            {
                const MapGateDefinition &out=warp.outGates[g];
                if(!dst.AddWarpOutGate(&fields[out.targetLegacyFieldCode],out.x,out.z,out.y))
                    return MapAdapterFail(error,"legacy AddWarpOutGate rejected compiled map data");
            }
            dst.WarpGate[dst.WarpGateActiveNum].LimitLevel=warp.limitLevel;
            dst.WarpGate[dst.WarpGateActiveNum].SpecialEffect=warp.specialEffect;
        }
        for(std::size_t a=0;a<src.ambient.size();++a)
        {
            const MapAmbientDefinition &ambient=src.ambient[a];
            if(!dst.AddAmbientPos(ambient.x,ambient.y,ambient.z,ambient.radius,ambient.soundCode))
                return MapAdapterFail(error,"legacy AddAmbientPos rejected compiled map data");
        }
        if(src.hasLegacyOrphanWarpLimitLevel)
            dst.WarpGate[dst.WarpGateActiveNum].LimitLevel=src.legacyOrphanWarpLimitLevel;
    }

    if(error) error->clear();
    return true;
}

template <typename LegacyField>
inline bool LoadCompiledMapsIntoLegacyFields(const char *packPath,
                                             MapProfile expectedProfile,
                                             std::uint32_t runtimeFeatureMask,
                                             LegacyField *fields,
                                             int fieldCapacity,
                                             const LegacyMapPathContext &paths,
                                             std::int32_t expectedDynamicFieldCode,
                                             std::uint32_t expectedStaticFieldCount,
                                             std::string *error=0)
{
    if(!packPath || !*packPath) return MapAdapterFail(error,"compiled map pack path is empty");
    CompiledMapContentSource source(packPath);
    if(!source.IsValid()) return MapAdapterFail(error,source.LastError().c_str());

    MapCatalogDefinition staged;
    if(!source.LoadCatalog(staged,error)) return false;
    if(staged.dynamicFieldCode!=expectedDynamicFieldCode)
        return MapAdapterFail(error,"compiled map dynamic field code does not match runtime contract");
    if(staged.staticFieldCount!=expectedStaticFieldCount)
        return MapAdapterFail(error,"compiled map static field count does not match runtime contract");

    // AddStageObject stores the supplied char pointer instead of copying it.
    // Keep the selected compiled catalog alive for the lifetime of this process.
    static MapCatalogDefinition persistentCatalog;
    persistentCatalog=staged;
    return MaterializeLegacyMapCatalog(persistentCatalog,expectedProfile,runtimeFeatureMask,
                                       fields,fieldCapacity,paths,error);
}

} // namespace map
} // namespace content
} // namespace summit
