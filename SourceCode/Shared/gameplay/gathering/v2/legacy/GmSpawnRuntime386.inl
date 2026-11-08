#include "systems/gameplay/gathering/v2/GmCombatResource396.h"
// SUMMIT386A: GM XYZ placements from the signed SFLD GMSP section.
// Included by VisualNodeCombatRuntime.inl after the legacy entity types exist.
// Never mutates stage SPM/SPP/SPC definitions or SQLite SpawnPlan361.
#include <cstdint>
#include <cstring>
#include <cstdio>
#include <string>
#include <vector>
#include "content/v1/runtime/RuntimeContentPath.h"
#include "content/v1/field_runtime/content/FieldRuntimePack.h"

struct SummitGM386Instance
{
    std::uint32_t serial;
    std::uint32_t retryAt;
    SummitGM386Instance() : serial(0), retryAt(0) {}
};
struct SummitGM386Plan
{
    std::uint32_t id, map;
    int x,y,z;
    std::uint32_t maxAlive, radius, respawnSeconds;
    std::string monsterName;
    std::vector<SummitGM386Instance> active;
};
struct SummitGM386State
{
    bool loaded;
    std::vector<SummitGM386Plan> plans;
    std::vector<std::uint32_t> sentPlayerSerial;
    std::vector<std::uint32_t> sentMonsterSerial;
    std::vector<std::uint32_t> sentAt;
    std::vector<unsigned char> attempts;
    SummitGM386State() : loaded(false) {}
};
static SummitGM386State& SummitGM386GetState() { static SummitGM386State s; return s; }
static std::uint32_t SummitGM386U32(const unsigned char* p)
{
    return static_cast<std::uint32_t>(p[0]) | (static_cast<std::uint32_t>(p[1])<<8) |
        (static_cast<std::uint32_t>(p[2])<<16) | (static_cast<std::uint32_t>(p[3])<<24);
}
static std::uint16_t SummitGM386U16(const unsigned char* p)
{ return static_cast<std::uint16_t>(p[0] | (static_cast<std::uint16_t>(p[1])<<8)); }
static bool SummitGM386Load()
{
    SummitGM386State& state=SummitGM386GetState();
    if(state.loaded) return !state.plans.empty();
    state.loaded=true;
    const std::string path=summit::content::runtime::ResolveRuntimePackPath("field_runtime.sfld");
    summit::content::field_runtime::FieldRuntimePack pack(path.c_str());
    if(!pack.IsValid())
    { std::fprintf(stderr,"[SUMMIT_GM_386] PACK_INVALID %s\n",pack.LastError().c_str()); return false; }
    std::vector<unsigned char> raw;
    std::uint16_t sectionVersion=0;
    if(!pack.GetSection("GMSP",raw,&sectionVersion))
    { std::fprintf(stderr,"[SUMMIT_GM_386] NO_GMSP_SECTION (not published)\n");return false; }
    if(sectionVersion!=1 || raw.size()<48 || std::memcmp(&raw[0],"GMS1",4)!=0 || SummitGM386U16(&raw[4])!=1 || SummitGM386U16(&raw[6])!=0)
    { std::fprintf(stderr,"[SUMMIT_GM_386] BAD_GMSP_HEADER\n");return false; }
    const std::uint32_t count=SummitGM386U32(&raw[8]);
    const std::uint32_t stride=SummitGM386U32(&raw[12]);
    if(stride!=96 || count>96 || raw.size()!=48u+static_cast<std::size_t>(count)*stride)
    { std::fprintf(stderr,"[SUMMIT_GM_386] BAD_GMSP_LAYOUT\n");return false; }
    // Outer SFLD and GMSP section digests checked by FieldRuntimePack.
    // Check the inner GMS1 records digest too, before allocating entities.
    if(count==0) { std::fprintf(stderr,"[SUMMIT_GM_386] NO_PUBLISHED_POINTS\n");return false; }
    unsigned char innerDigest[32];
    summit::content::common::Sha256(&raw[48],raw.size()-48u,innerDigest);
    if(std::memcmp(innerDigest,&raw[16],32)!=0)
    { std::fprintf(stderr,"[SUMMIT_GM_386] BAD_GMSP_INNER_SHA\n");return false; }
    std::uint32_t total=0;
    std::vector<SummitGM386Plan> staged;
    for(std::uint32_t i=0;i<count;++i)
    {
        const unsigned char* p=&raw[48u+static_cast<std::size_t>(i)*96u];
        SummitGM386Plan q;
        q.id=SummitGM386U32(p);
        q.map=SummitGM386U32(p+4);
        q.x=static_cast<std::int32_t>(SummitGM386U32(p+8));
        q.y=static_cast<std::int32_t>(SummitGM386U32(p+12));
        q.z=static_cast<std::int32_t>(SummitGM386U32(p+16));
        q.maxAlive=SummitGM386U32(p+20);
        q.radius=SummitGM386U32(p+24);
        q.respawnSeconds=SummitGM386U32(p+28);
        const char* name=reinterpret_cast<const char*>(p+32);
        const void* nul=std::memchr(name,0,64);
        if(!q.id || q.map>=static_cast<std::uint32_t>(StageAreaCount) || !nul ||
           q.maxAlive==0 || q.maxAlive>12 || q.radius<30 || q.radius>1500 ||
           q.respawnSeconds<10 || q.respawnSeconds>3600 || total+q.maxAlive>96)
        { std::fprintf(stderr,"[SUMMIT_GM_386] BAD_ROW id=%u\n",q.id);return false; }
        q.monsterName.assign(name,static_cast<const char*>(nul));
        if(q.monsterName.empty()) return false;
        for(std::size_t k=0;k<staged.size();++k)
            if(staged[k].map==q.map && staged[k].id==q.id) return false;
        total+=q.maxAlive;
        q.active.resize(q.maxAlive);
        staged.push_back(q);
    }
    state.plans.swap(staged);
    state.sentPlayerSerial.resize(static_cast<std::size_t>(total)*CONNECTMAX,0);
    state.sentMonsterSerial.resize(static_cast<std::size_t>(total)*CONNECTMAX,0);
    state.sentAt.resize(static_cast<std::size_t>(total)*CONNECTMAX,0);
    state.attempts.resize(static_cast<std::size_t>(total)*CONNECTMAX,0);
    std::fprintf(stderr,"[SUMMIT_GM_386] LOADED plans=%u capacity=%u pack=%s\n",
        static_cast<unsigned>(count),static_cast<unsigned>(total),path.c_str());
    return true;
}
static smCHAR* SummitGM386Find(std::uint32_t map,std::uint32_t serial)
{
    if(!serial || !StageArea || map>=static_cast<std::uint32_t>(StageAreaCount))return NULL;
    STG_AREA& area=StageArea[map];
    for(int k=0;k<area.MonsterLastPoint;++k)
    {
        smCHAR* m=area.lpCharMonster[k];
        if(m && m->Flag && m->dwObjectSerial==serial)return m;
    }
    return NULL;
}
static smCHAR* SummitGM386Spawn(const SummitGM386Plan& p,int dx,int dz,const char** failure)
{
    if(!StageArea || p.map>=static_cast<std::uint32_t>(StageAreaCount))
    { *failure="invalid-map";return NULL; }
    STG_AREA& a=StageArea[p.map];
    if(!a.lpStage || a.FindEmptyMonTable()<0)
    { *failure="stage-not-ready-or-full";return NULL; }
    int templateIndex=-1;
    for(int k=0;k<chrMonsterCnt;++k)
        if(lstrcmpi(chrMonsterList[k].szName,p.monsterName.c_str())==0)
        { templateIndex=k;break; }
    if(templateIndex<0 || !chrMonsterList[templateIndex].lpMonInfo)
    { *failure="unresolved-runtime-monster-template";return NULL; }
    smCHAR* m=srFindEmptyChar();
    if(!m){*failure="global-monster-pool-full";return NULL;}
    const smCHAR_INFO& definition=chrMonsterList[templateIndex];
    m->Init();
    m->lpStage=a.lpStage;
    const int fx=gameplay::WorldGridToFixed(p.x+dx);
    const int fy=gameplay::WorldGridToFixed(p.y);
    const int fz=gameplay::WorldGridToFixed(p.z+dz);
    m->SetPosi(fx,0,fz,0,0,0);m->pY=fy;
    m->PosiAround.x=fx;m->PosiAround.y=fy;m->PosiAround.z=fz;
    m->OpenStartPostion=0;
    m->lpExt1=(void*)&a.Head;
    m->dwObjectSerial=GetNewObjectSerial();
    m->ActionPattern=5;
    memcpy(&m->smCharInfo,&definition,sizeof(smCHAR_INFO));
    memcpy(&m->smMonsterInfo,definition.lpMonInfo,sizeof(smCHAR_MONSTER_INFO));
    m->smCharInfo.StatePoint=definition.lpMonInfo->ClassCode;
    m->AttackUserFlag=TRUE;
    m->lpExt2=0;m->lpExpAttackPlayInfo=0;m->ExpAttackLife=0;
    m->SetLoadPattern(m->smCharInfo.szModelName);
    m->Flag=TRUE;
    if(!a.AddMonTable(m))
    {
        if(a.StartPointMonCount[m->OpenStartPostion]>0)--a.StartPointMonCount[m->OpenStartPostion];
        m->Close();*failure="area-entity-table-registration-failed";return NULL;
    }
    ++a.MonsterCount;
    m->ReopenCount=REOPEN_COUNT_NUM;
    m->dwLastTransTime=dwPlayServTime;
    if(!m->lpAttackDamageList)
    {
        m->lpAttackDamageList=new ATTACK_DAMAGE_LIST[ATTACK_DAMAGE_LIST_MAX];
        if(m->lpAttackDamageList)ZeroMemory(m->lpAttackDamageList,sizeof(ATTACK_DAMAGE_LIST)*ATTACK_DAMAGE_LIST_MAX);
    }
    ++rsMonOpenCount;
    m->DistAroundDbl=static_cast<int>(p.radius*p.radius);
    m->SetMotionFromCode(CHRMOTION_STATE_STAND);
    m->TargetMoveCount=0;
    m->NearPlayCount=256;
    m->SavePlayBuff2();
    return m;
}
static void SummitGM386Announce(smCHAR* m,rsPLAYINFO* player)
{
    char buf[sizeof(smPLAYDATA_GROUP)+sizeof(smPLAYDATA)];
    ZeroMemory(buf,sizeof(buf));
    smPLAYDATA_GROUP* grp=reinterpret_cast<smPLAYDATA_GROUP*>(buf);
    smPLAYDATA* data=reinterpret_cast<smPLAYDATA*>(buf+sizeof(smPLAYDATA_GROUP));
    grp->size=sizeof(buf);grp->code=OpCode::OPCODE_PLAYDATAGROUP;grp->PlayDataCnt=1;
    grp->dwSendCode=0x80000000u | (player->SendPlayGroupCount & 0x00FFFFFFu);
    data->size=sizeof(smPLAYDATA);data->code=OpCode::OPCODE_PLAYDATA1;
    data->dwObjectSerial=m->dwObjectSerial;
    data->x=m->pX;data->y=m->pY;data->z=m->pZ;
    data->angle[0]=m->Angle.x;data->angle[1]=m->Angle.y;data->angle[2]=m->Angle.z;
    data->dwAutoCharCode=m->smMonsterInfo.dwAutoCharCode;
    SendOnServerLegacySocket(player->lpsmSock,buf,grp->size);
    smTRNAS_PLAYERINFO info;ZeroMemory(&info,sizeof(info));
    memcpy(&info.smCharInfo,&m->smCharInfo,sizeof(smCHAR_INFO));
    info.size=sizeof(info);info.code=OpCode::OPCODE_PLAYERINFO;
    info.dwObjectSerial=m->dwObjectSerial;info.x=m->pX;info.y=m->pY;info.z=m->pZ;
    SendOnServerLegacySocket(player->lpsmSock,reinterpret_cast<char*>(&info),info.size);
}
static void SummitGMTick386(std::uint32_t now)
{
    // Bound the 96-slot/CONNECTMAX scan to one pass per second.
    // The global server tick remains unchanged for the four legacy profession nodes.
    static std::uint32_t lastTick=0;
    if(lastTick && static_cast<std::uint32_t>(now-lastTick)<1000u) return;
    lastTick=now;
    SummitGM386State& s=SummitGM386GetState();
    if(!SummitGM386Load())return;
    std::size_t flat=0;
    int spawnedThisTick=0;
    for(std::size_t i=0;i<s.plans.size();++i)
    {
        SummitGM386Plan& plan=s.plans[i];
        bool playersInMap=false;
        for(int j=0;j<CONNECTMAX;++j)
            if(rsPlayInfo[j].lpsmSock && rsPlayInfo[j].dwObjectSerial && rsPlayInfo[j].Position.Area==static_cast<int>(plan.map))
            { playersInMap=true;break; }
        for(std::size_t k=0;k<plan.active.size();++k,++flat)
        {
            SummitGM386Instance& item=plan.active[k];
            smCHAR* m=SummitGM386Find(plan.map,item.serial);
            if(!m && item.serial)
            { item.serial=0;item.retryAt=now+plan.respawnSeconds*1000u; }
            if(!m && playersInMap && spawnedThisTick<4 &&
                static_cast<std::int32_t>(now-item.retryAt)>=0)
            {
                // Deterministic small spread for density, not new GM coordinates.
                const int dx=(static_cast<int>(k%3)-1)*24;
                const int dz=(static_cast<int>((k/3)%3)-1)*24;
                const char* why="unknown";
                m=SummitGM386Spawn(plan,dx,dz,&why);
                if(m)
                {
                    item.serial=m->dwObjectSerial;item.retryAt=0;
                    ++spawnedThisTick;
    // SUMMIT395A_GM58_INSTANCE_TRAINING_PROFILE: only the smCHAR instance, never the shared monster definition.
    // This code executes inside the GM386 SPAWN path, before its SPAWN log.
    // The stage pointer is additionally checked to avoid affecting other maps.
    if (m && StageAreaCount > 58 && StageArea[58].lpStage &&
        m->lpStage == StageArea[58].lpStage)
    {
        m->smCharInfo.Level = 1;
        m->smCharInfo.Life[0] = 1;
        m->smCharInfo.Life[1] = 1;
        m->smCharInfo.Attack_Damage[0] = 0;
        m->smCharInfo.Attack_Damage[1] = 0;
        m->smCharInfo.Defence = 0;
        m->smCharInfo.Exp = 0;
        m->smMonsterInfo.GetExp = 0;
        m->smMonsterInfo.SkillDamage[0] = 0;
        m->smMonsterInfo.SkillDamage[1] = 0;
        m->smMonsterInfo.FallItemCount = 0;
        m->smMonsterInfo.FallItemPerMax = 0;
        m->smMonsterInfo.FallItemMax = 0;
        m->smMonsterInfo.FallItemPlusCount = 0;
        m->smMonsterInfo.dwEvnetItem = 0;
        m->smMonsterInfo.PotionCount = 0;
        m->smMonsterInfo.PotionPercent = 0;
        m->smMonsterInfo.SellAttackItemCount = 0;
        m->smMonsterInfo.SellDefenceItemCount = 0;
        m->smMonsterInfo.SellEtcItemCount = 0;
        m->SavePlayBuff2();
        fprintf(stderr, "[SUMMIT_GM_395] PROFILE serial=%u map=58 hp=1 damage=0 normal_drops=0\n",
            static_cast<unsigned int>(m->dwObjectSerial));
        // SUMMIT396A_GM58_COMBAT_MATERIALS: register only verified GM map-58 instance.
        summit_gm396::RegisterSpawn(static_cast<unsigned int>(m->dwObjectSerial),
                                   m->smCharInfo.szModelName, m->smCharInfo.szName);
        fflush(stderr);
    }
    else
    {
        fprintf(stderr, "[SUMMIT_GM_395] PROFILE_SKIPPED map=58 reason=stage_mismatch\n");
        fflush(stderr);
    }
                    std::fprintf(stderr,"[SUMMIT_GM_386] SPAWN plan=%u serial=%u map=%u name=%s\n",plan.id,item.serial,plan.map,plan.monsterName.c_str());
                }
                else
                {
                    item.retryAt=now+15000u;
                    std::fprintf(stderr,"[SUMMIT_GM_386] SPAWN_FAIL plan=%u reason=%s\n",plan.id,why);
                }
            }
            if(!m) continue;
            m->NearPlayCount=256;
            for(int j=0;j<CONNECTMAX;++j)
            {
                rsPLAYINFO* player=&rsPlayInfo[j];
                if(!player->lpsmSock || !player->dwObjectSerial || player->Position.Area!=static_cast<int>(plan.map))continue;
                const std::int64_t dx=(static_cast<std::int64_t>(m->pX)-player->Position.x)>>FLOATNS;
                const std::int64_t dz=(static_cast<std::int64_t>(m->pZ)-player->Position.z)>>FLOATNS;
                if(dx<=-TRANS_VIEW_LIMIT || dx>=TRANS_VIEW_LIMIT || dz<=-TRANS_VIEW_LIMIT || dz>=TRANS_VIEW_LIMIT || dx*dx+dz*dz>=DIST_TRANSLEVEL_HIGH)continue;
                const std::size_t idx=flat*static_cast<std::size_t>(CONNECTMAX)+static_cast<std::size_t>(j);
                if(s.sentMonsterSerial[idx]!=item.serial || s.sentPlayerSerial[idx]!=player->dwObjectSerial)
                {s.sentMonsterSerial[idx]=item.serial;s.sentPlayerSerial[idx]=player->dwObjectSerial;s.attempts[idx]=0;s.sentAt[idx]=0;}
                if(s.attempts[idx]>=2 || (s.attempts[idx] && static_cast<std::uint32_t>(now-s.sentAt[idx])<6000u))continue;
                s.sentAt[idx]=now;++s.attempts[idx];
                SummitGM386Announce(m,player);
            }
        }
    }
}
