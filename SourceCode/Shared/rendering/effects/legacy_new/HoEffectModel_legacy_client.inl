#include "systems/rendering/effects/legacy_new/core/HoEffectModel.h"
#include "systems/rendering/effects/legacy_new/core/HoEffectView.h"
#include "systems/rendering/effects/legacy_new/core/HoEffectResMgr.h"

#include "rendering/effects/legacy_new/SummitEffectFrames248.h"

void HoEffectTextureNameProperty::Init(const std::string &fileName)
{
    Clear();
    if (!fileName.empty())
    {
        m_TextureName.push_back(fileName);
        HoEffectResManager::GetInstance()->RequestResTexturePrewarm(fileName);
    }
}

void HoEffectTextureNameProperty::Init(const std::string &fileName, int textureCount, float aniDelayTime)
{
    Clear();
    m_fAniDelayTime = aniDelayTime > 0.f ? aniDelayTime : 0.5f;
    // A malformed effect definition must not generate unbounded allocations,
    // truncate paths into a different asset, or keep a stale animation index.
    if (!summit::effects::BuildEffectFrameNames248(fileName, textureCount, m_TextureName))
        m_TextureName.clear();
    m_iAniIndexCount = static_cast<int>(m_TextureName.size());
    // Schedule the first two frames without allocating materials from Init.
    for (size_t i = 0; i < m_TextureName.size() && i < 2u; ++i)
        HoEffectResManager::GetInstance()->RequestResTexturePrewarm(m_TextureName[i]);
}

bool HoEffectTextureNameProperty::Main(float elapsedTime)
{
    if (m_iAniIndexCount <= 0 || m_TextureName.empty())
        return false;
    if (m_fAniTimer > m_fAniDelayTime)
    {
        m_iAniIndex = (m_iAniIndex + 1) % m_iAniIndexCount;
        // Next frames are queued before they become visible. A one-frame
        // first-use fallback remains for unusually fast animations.
        for (int lookahead = 1; lookahead <= 2 && lookahead < m_iAniIndexCount; ++lookahead)
            HoEffectResManager::GetInstance()->RequestResTexturePrewarm(
                m_TextureName[(m_iAniIndex + lookahead) % m_iAniIndexCount]);
        m_fAniTimer = 0.f;
    }
    else
        m_fAniTimer += elapsedTime;
    return true;
}

int HoEffectTextureNameProperty::GetMatNum()
{
    if (m_iAniIndex < 0 || static_cast<size_t>(m_iAniIndex) >= m_TextureName.size())
        return -1;
    return HoEffectResManager::GetInstance()->GetResTexture(m_TextureName[m_iAniIndex]);
}

void HoEffectTextureNameProperty::Clear()
{
    m_TextureName.clear();
    m_iAniIndex = 0;
    m_iAniIndexCount = 0;
    m_fAniTimer = 0.f;
}

HoEffectModelInterface::HoEffectModelInterface() : m_Color(255.f, 255.f, 255.f, 255.f),
                                                   m_ColorStep(0.f, 0.f, 0.f, 0.f),
                                                   m_Size(20.f, 20.f, 20.f),
                                                   m_SizeStep(0.f, 0.f, 0.f),
                                                   m_Translate(0.f, 0.f, 0.f),
                                                   m_TranslateStep(0.f, 0.f, 0.f),
												   m_Angle(0.f, 0.f, 0.f),
												   m_AngleStep(0.f, 0.f, 0.f),
                                                   m_fTimer(0.f),
                                                   m_iBlendType(SMMAT_BLEND_LAMP),
                                                   m_fEventTimer(0.f),
                                                   m_CurrentEventIndex(-1),
                                                   m_bLiveFlag(true),
                                                   m_fEndTime(0.f),
                                                   m_fStartDelayTime(0.f)
{
}

void HoEffectModelInterface::Init()
{
    m_Color.Assign(255.f, 255.f, 255.f, 255.f);
    m_ColorStep.Assign(0.f, 0.f, 0.f, 0.f);
    m_Size.Assign(20.f, 20.f, 20.f);
    m_SizeStep.Assign(0.f, 0.f, 0.f);
    m_Translate.Assign(0.f, 0.f, 0.f);
    m_TranslateStep.Assign(0.f, 0.f, 0.f);
    m_fTimer = 0.f;
    m_iBlendType = SMMAT_BLEND_LAMP;
    m_fEventTimer = 0.f;
	ResetCurrentEvent();
    m_bLiveFlag = true;
    m_fEndTime = 0.f;
    m_fStartDelayTime = 0.f;
}

void HoEffectTextureModel::Init()
{
    HoEffectModelInterface::Init();
    m_Direction.Assign(0.f, 1.f, 0.f);
    m_bLiveFlag = false;
}

void HoEffectTextureModel::Init(const std::string &fileName)
{
    m_TextureNameProperty.Init(fileName);
}

void HoEffectTextureModel::Init(const std::string &fileName, int textureCount, float aniDelayTime)
{
    m_TextureNameProperty.Init(fileName, textureCount, aniDelayTime);
}

bool HoEffectTextureModel::UpdateView(int x, int y, int z, int ax, int ay, int az)
{
    int matNum = -1;
    matNum = m_TextureNameProperty.GetMatNum();

    if(matNum == -1)
        return false;

    if(m_iClassID == CLASS_EFFECT_BILLBOARD)
    {
        SBillboardRenderInfo renderInfo;
        renderInfo.m_Pos   = m_Translate;
        renderInfo.m_Size  = m_Size;
        renderInfo.m_Color = m_Color;
        renderInfo.m_MatID = matNum;
        HoEffectView::UpdateBillboard(&renderInfo, x, y, z, ax, ay, az);
    }
    else if(m_iClassID == CLASS_EFFECT_CREATE_MESH)
    {
        SBillboardRenderInfo renderInfo;
        renderInfo.m_Type   = CREATE_MESH;
        renderInfo.m_Pos    = m_Translate;
        renderInfo.m_Size   = m_Size;
        renderInfo.m_Color  = m_Color;
		
        renderInfo.m_MatID  = matNum;
        HoEffectView::UpdateBillboard(&renderInfo, x, y, z, ax, ay, az);
    }
    else if(m_iClassID == CLASS_EFFECT_BILLBOARD_AXIAL)
    {
        SBillboardRenderInfo renderInfo;
        renderInfo.m_Type   = BILLBOARD_AXIAL;
        renderInfo.m_Pos    = m_Translate + m_Direction;
        renderInfo.m_DesPos = m_Translate + m_Direction * m_Size.y;
        renderInfo.m_Size   = m_Size;
        renderInfo.m_Color  = m_Color;
        renderInfo.m_MatID  = matNum;
        HoEffectView::UpdateBillboard(&renderInfo, x, y, z, ax, ay, az);
    }
    return true;
}

void HoEffectBillboardAxialModel::Init(const std::string &fileName)
{
    m_TextureNameProperty.Init(fileName);
}

void HoEffectBillboardAxialModel::Init(const std::string &fileName, int textureCount, float aniDelayTime)
{
    m_TextureNameProperty.Init(fileName, textureCount, aniDelayTime);
}

bool HoEffectBillboardAxialModel::UpdateView(int x, int y, int z, int ax, int ay, int az)
{
    int matNum = -1;
    matNum = m_TextureNameProperty.GetMatNum();

    if(matNum == -1)
        return false;
    SBillboardRenderInfo renderInfo;
    renderInfo.m_Type   = BILLBOARD_AXIAL;
    renderInfo.m_Pos    = m_CurPos+m_Translate;
    renderInfo.m_DesPos = m_DesPos+m_Translate;
    renderInfo.m_Size   = m_Size;
    renderInfo.m_Color  = m_Color;
    renderInfo.m_MatID  = matNum;
    HoEffectView::UpdateBillboard(&renderInfo, x, y, z, ax, ay, az);
    return true;
}

void HoEffectLineListModel::Init(const std::string &fileName)
{
	m_TextureNameProperty.Init(fileName);
}

void HoEffectLineListModel::AddData(const point3 &pos)
{
	int t = m_TraceList.size();
	if(m_TraceList.size() < m_iMaxCount)
		m_TraceList.push_front(pos);
	else
    {
		if(!m_TraceList.empty())
        {
			std::list<point3>::iterator i;
			i = m_TraceList.end();
            --i;
            m_TraceList.erase(i);
			m_TraceList.push_front(pos);
		}
	}
}

bool HoEffectLineListModel::UpdateView(int x, int y, int z, int ax, int ay, int az)
{
	int matNum = -1;
    matNum = m_TextureNameProperty.GetMatNum();

    if(matNum == -1)
        return false;

	SLineListRenderInfo renderInfo;
	renderInfo.m_Size       = m_Size;
	renderInfo.m_Color      = m_Color;
	renderInfo.m_MatID      = matNum;
    renderInfo.m_iBlendType = m_iBlendType;
	renderInfo.m_pList      = &m_TraceList;
	HoEffectView::UpdateLineList(&renderInfo, x, y, z, ax, ay, az);
	return true;	
}


void HoEffectMeshModel::Init(const std::string &meshName)
{
    m_MeshName = meshName;
    m_BoneName.clear(); // Reused effect instances must not retain previous skeleton.
    HoEffectResManager::GetInstance()->RequestResMesh(m_MeshName, m_BoneName);
    m_bLiveFlag = true;
}

void HoEffectMeshModel::Init(const std::string &meshName, const std::string &boneName)
{
    m_MeshName = meshName;
    m_BoneName = boneName;
    HoEffectResManager::GetInstance()->RequestResMesh(m_MeshName, m_BoneName);
    m_bLiveFlag = true;
}

bool HoEffectMeshModel::UpdateView(int x, int y, int z, int ax, int ay, int az)
{
    if(m_bLiveFlag == false)
        return false;
    smPAT3D *pat3D = HoEffectResManager::GetInstance()->GetReadyResMesh(m_MeshName, m_BoneName);
    if(pat3D == NULL)
        return false;
	
	SMeshRenderInfo renderInfo;
    renderInfo.m_Pos           = m_Translate;
	renderInfo.m_Angle         = m_Angle;
    renderInfo.m_Pat           = pat3D;
    renderInfo.m_Color         = m_Color;
    renderInfo.m_iBlendType    = m_iBlendType;
    renderInfo.m_iCurrentFrame = (int)m_fCurrentFrame;
    HoEffectView::UpdateMesh(&renderInfo, x, y, z, ax, ay, az);
    return true;
}











