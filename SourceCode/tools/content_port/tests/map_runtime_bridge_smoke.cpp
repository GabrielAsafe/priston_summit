#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

struct LegacyField;
struct LegacyGate { int x=0,y=0,z=0; LegacyField *lpsField=0; };
struct LegacyWarpGate { int x=0,y=0,z=0,size=0,height=0,LimitLevel=0,SpecialEffect=0,OutGateCount=0; LegacyGate OutGate[12]; };
struct LegacyAmbient { int x=0,y=0,z=0,round=0,code=0; };
struct LegacyPoint { int x=0,y=0,z=0; };
struct LegacyStartPoint { int x=0,y=0; };

static const char *gFieldDir="field//";
static const char *gMapDir="field//map//";
static const char *gTitleDir="field//title//";

struct LegacyField
{
    unsigned int head=0;
    char szName[64]={};
    char szNameMap[64]={};
    char szNameTitle[64]={};
    int State=0;
    int BackImageCode[3]={};
    int BackMusicCode=0;
    int FieldEvent=0;
    int GateCount=0;
    LegacyGate FieldGate[12];
    int WarpGateCount=0;
    int WarpGateActiveNum=0;
    LegacyWarpGate WarpGate[12];
    LegacyPoint PosWarpOut;
    LegacyAmbient AmbientPos[80];
    int AmbentCount=0;
    int LimitLevel=0;
    int FieldSight=0;
    char *lpStageObjectName[50]={};
    unsigned int StgObjBip[50]={};
    int StgObjCount=0;
    int cX=0,cZ=0;
    int FieldCode=0;
    int ServerCode=0;
    LegacyStartPoint StartPoint[8];
    int StartPointCnt=0;

    int SetName(char *stage,char *map=0)
    {
        std::snprintf(szName,sizeof(szName),"%s%s",gFieldDir,stage?stage:"");
        if(map)
        {
            std::snprintf(szNameMap,sizeof(szNameMap),"%s%s.tga",gMapDir,map);
            std::snprintf(szNameTitle,sizeof(szNameTitle),"%s%st.tga",gTitleDir,map);
        }
        else szNameMap[0]=szNameTitle[0]=0;
        return 1;
    }
    int SetCenterPos(int x,int z) { cX=x;cZ=z;return 1; }
    int AddStageObject(char *name,int bip=0)
    {
        if(StgObjCount>=50) return 0;
        lpStageObjectName[StgObjCount]=name;StgObjBip[StgObjCount]=static_cast<unsigned int>(bip);return ++StgObjCount;
    }
    int AddGate2(LegacyField *field,int x,int z,int y)
    {
        if(GateCount>=12) return 0;
        LegacyGate &g=FieldGate[GateCount++];g.x=x;g.y=y;g.z=z;g.lpsField=field;return 1;
    }
    int AddGate(LegacyField *field,int x,int z,int y)
    {
        if(GateCount>=12) return 0;
        LegacyGate &g=FieldGate[GateCount++];g.x=x;g.y=y;g.z=z;g.lpsField=field;
        field->AddGate2(this,x,z,y);return 1;
    }
    int AddWarpGate(int x,int z,int y,int size,int height)
    {
        if(WarpGateCount>=12) return 0;
        WarpGateActiveNum=WarpGateCount;
        LegacyWarpGate &w=WarpGate[WarpGateCount++];
        w.x=x;w.z=z;w.y=y;w.size=size;w.height=height;w.OutGateCount=0;return WarpGateCount;
    }
    int AddWarpOutGate(LegacyField *field,int x,int z,int y)
    {
        LegacyWarpGate &w=WarpGate[WarpGateActiveNum];
        if(w.OutGateCount>=12) return 0;
        LegacyGate &g=w.OutGate[w.OutGateCount++];g.x=x;g.z=z;g.y=y;g.lpsField=field;
        if(field==this){PosWarpOut.x=x;PosWarpOut.y=y;PosWarpOut.z=z;}
        return w.OutGateCount;
    }
    int AddStartPoint(int x,int z)
    {
        if(StartPointCnt>=8) return 0;
        StartPoint[StartPointCnt].x=x;
        StartPoint[StartPointCnt].y=z;
        return ++StartPointCnt;
    }
    int AddAmbientPos(int x,int y,int z,int round,int code)
    {
        if(AmbentCount>=80) return 0;
        LegacyAmbient &a=AmbientPos[AmbentCount++];
        a.x=x;a.y=y;a.z=z;a.round=round;a.code=code;
        return 1;
    }
};

#include "content/v1/map/legacy/LegacyMapAdapter.h"

static int fail(const char *message)
{
    std::fprintf(stderr,"FAIL: %s\n",message);return 1;
}

static int verifyCatalog(const summit::content::map::MapCatalogDefinition &catalog,
                         unsigned int featureMask,LegacyField *fields)
{
    using namespace summit::content::map;
    struct ExpectedGate { int target,x,z,y; };
    std::vector<std::vector<ExpectedGate> > gates(256);
    for(std::size_t i=0;i<catalog.maps.size();++i)
    {
        const MapDefinition &m=catalog.maps[i];
        for(std::size_t g=0;g<m.gates.size();++g)
        {
            const MapGateDefinition &item=m.gates[g];
            gates[static_cast<std::size_t>(m.legacyFieldCode)].push_back({item.targetLegacyFieldCode,item.x,item.z,item.y});
            gates[static_cast<std::size_t>(item.targetLegacyFieldCode)].push_back({m.legacyFieldCode,item.x,item.z,item.y});
        }
    }

    for(std::size_t i=0;i<catalog.maps.size();++i)
    {
        const MapDefinition &m=catalog.maps[i];
        LegacyField &f=fields[m.legacyFieldCode];
        char expected[128];
        std::snprintf(expected,sizeof(expected),"%s%s",gFieldDir,m.stagePath.c_str());
        if(std::strcmp(f.szName,expected)!=0) return fail("catalog stage path parity");
        if(m.hasMapImageId)
        {
            std::snprintf(expected,sizeof(expected),"%s%s.tga",gMapDir,m.mapImageId.c_str());
            if(std::strcmp(f.szNameMap,expected)!=0) return fail("catalog map image parity");
            std::snprintf(expected,sizeof(expected),"%s%st.tga",gTitleDir,m.mapImageId.c_str());
            if(std::strcmp(f.szNameTitle,expected)!=0) return fail("catalog title image parity");
        }
        else if(f.szNameMap[0] || f.szNameTitle[0]) return fail("catalog null map image parity");
        if(f.State!=m.state || f.BackImageCode[0]!=m.backImageCode[0] || f.BackImageCode[1]!=m.backImageCode[1] ||
           f.BackImageCode[2]!=m.backImageCode[2] || f.BackMusicCode!=m.backMusicCode || f.FieldEvent!=m.fieldEvent ||
           f.ServerCode!=m.serverCode || f.LimitLevel!=m.limitLevel || f.FieldSight!=m.fieldSight ||
           f.cX!=m.centerX || f.cZ!=m.centerZ) return fail("catalog scalar parity");

        std::size_t enabled=0;
        for(std::size_t x=0;x<m.stageObjects.size();++x)
            if(MapStageObjectEnabled(m.stageObjects[x],featureMask))
            {
                if(enabled>=static_cast<std::size_t>(f.StgObjCount)) return fail("catalog stage-object count parity");
                if(!f.lpStageObjectName[enabled] || std::strcmp(f.lpStageObjectName[enabled],m.stageObjects[x].assetPath.c_str())!=0 ||
                   f.StgObjBip[enabled]!=static_cast<unsigned int>(m.stageObjects[x].bipAnimation))
                    return fail("catalog stage-object parity");
                ++enabled;
            }
        if(enabled!=static_cast<std::size_t>(f.StgObjCount)) return fail("catalog enabled stage-object count parity");

        if(f.StartPointCnt!=static_cast<int>(m.startPoints.size())) return fail("catalog start-point count parity");
        for(std::size_t x=0;x<m.startPoints.size();++x)
            if(f.StartPoint[x].x!=m.startPoints[x].x || f.StartPoint[x].y!=m.startPoints[x].z)
                return fail("catalog start-point parity");

        const std::vector<ExpectedGate> &expectedGates=gates[static_cast<std::size_t>(m.legacyFieldCode)];
        if(f.GateCount!=static_cast<int>(expectedGates.size())) return fail("catalog reciprocal gate count parity");
        for(std::size_t x=0;x<expectedGates.size();++x)
        {
            const ExpectedGate &g=expectedGates[x];
            if(f.FieldGate[x].lpsField!=&fields[g.target] || f.FieldGate[x].x!=g.x || f.FieldGate[x].z!=g.z || f.FieldGate[x].y!=g.y)
                return fail("catalog reciprocal gate parity");
        }

        if(f.WarpGateCount!=static_cast<int>(m.warpGates.size())) return fail("catalog warp-gate count parity");
        for(std::size_t x=0;x<m.warpGates.size();++x)
        {
            const MapWarpGateDefinition &w=m.warpGates[x]; const LegacyWarpGate &actual=f.WarpGate[x];
            if(actual.x!=w.x || actual.z!=w.z || actual.y!=w.y || actual.size!=w.size || actual.height!=w.height ||
               actual.LimitLevel!=w.limitLevel || actual.SpecialEffect!=w.specialEffect || actual.OutGateCount!=static_cast<int>(w.outGates.size()))
                return fail("catalog warp-gate parity");
            for(std::size_t y=0;y<w.outGates.size();++y)
            {
                const MapGateDefinition &g=w.outGates[y]; const LegacyGate &a=actual.OutGate[y];
                if(a.lpsField!=&fields[g.targetLegacyFieldCode] || a.x!=g.x || a.z!=g.z || a.y!=g.y)
                    return fail("catalog warp-out parity");
            }
        }
        if(m.hasLegacyOrphanWarpLimitLevel && f.WarpGate[f.WarpGateActiveNum].LimitLevel!=m.legacyOrphanWarpLimitLevel)
            return fail("catalog orphan warp parity");

        if(f.AmbentCount!=static_cast<int>(m.ambient.size())) return fail("catalog ambient count parity");
        for(std::size_t x=0;x<m.ambient.size();++x)
        {
            const MapAmbientDefinition &a=m.ambient[x]; const LegacyAmbient &actual=f.AmbientPos[x];
            if(actual.x!=a.x || actual.y!=a.y || actual.z!=a.z || actual.round!=a.radius || actual.code!=a.soundCode)
                return fail("catalog ambient parity");
        }
    }
    return 0;
}

int main(int argc,char **argv)
{
    if(argc!=3) return fail("usage: smoke <pack> <client|server>");
    const bool client=std::strcmp(argv[2],"client")==0;
    const summit::content::map::MapProfile profile=client?summit::content::map::MapProfileClient:summit::content::map::MapProfileServer;
    LegacyField fields[256];
    for(int i=0;i<256;++i) fields[i].FieldCode=i;
    std::string error;
    summit::content::map::LegacyMapPathContext paths(gFieldDir,gMapDir,gTitleDir);
    std::uint32_t staticFieldCount=0;
    std::int32_t dynamicFieldCode=-1;
    if(!summit::content::map::LoadCompiledMapsIntoLegacyFields(argv[1],profile,0,fields,256,paths,&staticFieldCount,&dynamicFieldCode,&error))
    { std::fprintf(stderr,"FAIL load: %s\n",error.c_str()); return 1; }
    if(staticFieldCount<53 || dynamicFieldCode!=static_cast<std::int32_t>(staticFieldCount)) return fail("content-driven field-count contract");

    if(std::strcmp(fields[0].szName,"field//forest//fore-3.ase")!=0) return fail("field 0 stage name");
    if(fields[3].StgObjCount!=14) return fail("WORLDCUP objects must be absent with feature mask 0");
    if(fields[20].State!=(client?512:768)) return fail("profile-specific field 20 state");
    if(fields[34].LimitLevel!=(client?70:115)) return fail("profile-specific field 34 level");
    if(fields[52].LimitLevel!=(client?1:100)) return fail("profile-specific field 52 level");
    if(fields[45].WarpGateCount!=0 || fields[45].WarpGate[0].LimitLevel!=40) return fail("orphan warp limit preservation");
    if(fields[47].WarpGate[0].LimitLevel!=(client?118:135)) return fail("profile-specific orphan warp limit");
    if(fields[0].GateCount<1 || fields[1].GateCount<1) return fail("reciprocal field gates");
    if(fields[0].FieldGate[0].lpsField!=&fields[1]) return fail("field 0 gate target");
    bool reciprocal=false;
    for(int i=0;i<fields[1].GateCount;++i) if(fields[1].FieldGate[i].lpsField==&fields[0]) reciprocal=true;
    if(!reciprocal) return fail("field 1 reverse gate target");
    if(fields[0].StgObjCount<1 || !fields[0].lpStageObjectName[0] || std::strcmp(fields[0].lpStageObjectName[0],"forest//3ani-01.ASE")!=0)
        return fail("persistent stage-object string");

    summit::content::map::CompiledMapContentSource paritySource(argv[1]);
    summit::content::map::MapCatalogDefinition parityCatalog;
    if(!paritySource.LoadCatalog(parityCatalog,&error)) return fail("reload catalog for full parity");
    if(verifyCatalog(parityCatalog,0,fields)) return 1;

    LegacyField worldcup[256];
    for(int i=0;i<256;++i) worldcup[i].FieldCode=i;
    summit::content::map::CompiledMapContentSource source(argv[1]);
    summit::content::map::MapCatalogDefinition catalog;
    if(!source.LoadCatalog(catalog,&error)) return fail("reload catalog for feature test");
    if(!summit::content::map::MaterializeLegacyMapCatalog(catalog,profile,summit::content::map::MapFeatureWorldCupField,worldcup,256,paths,&error))
    { std::fprintf(stderr,"FAIL feature materialize: %s\n",error.c_str()); return 1; }
    if(worldcup[3].StgObjCount!=19) return fail("WORLDCUP field 3 objects");
    if(worldcup[21].StgObjCount<6) return fail("WORLDCUP field 21 objects");
    if(verifyCatalog(catalog,summit::content::map::MapFeatureWorldCupField,worldcup)) return 1;

    std::puts("PASS map runtime bridge smoke");
    return 0;
}
