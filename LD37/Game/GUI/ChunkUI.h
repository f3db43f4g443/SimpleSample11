#pragma once
#include "Block.h"
#include "Render/DrawableGroup.h"
#include "Render/ParticleSystem.h"

class CChunkUI : public CEntity
{
	friend void RegisterGameClasses();
public:
	CChunkUI( const SClassCreateContext& context ) : CEntity( context ), m_onTick( this, &CChunkUI::OnTick ), m_fBlockBulletEftAlpha( 0 ), m_fRepairPercent( 0 )
	{ SET_BASEOBJECT_ID( CChunkUI ); }

	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;
	void SetChunkObject( CChunkObject* pChunkObject );
	void ShowRepairEffect();
private:
	void OnTick();
	void UpdateHp();
	void UpdateEft();
	void UpdateRepair();

	CReference<CRenderObject2D> m_pFrameImg[8];
	CReference<CRenderObject2D> m_pRepairImg;

	CReference<CEntity> m_pBlockBulletEffect;
	CReference<CEntity> m_pRepairEffect;
	CReference<CDrawableGroup> m_pBlockBulletDrawable;
	CReference<CParticleFile> m_pRepairDrawable;
	vector<CReference<CImage2D> > m_vecBlockBulletEfts;
	vector<CReference<CParticleSystemObject> > m_vecRepairParticles;
	uint32 m_nUsingEftCount;
	uint32 m_nParticleCount;
	float m_fBlockBulletEftAlpha;
	float m_fRepairPercent;

	CChunkObject* m_pChunkObject;
	TClassTrigger<CChunkUI> m_onTick;
};

class CBlockDetectUI : public CEntity
{
	friend void RegisterGameClasses();
public:
	CBlockDetectUI( const SClassCreateContext& context ) : CEntity( context ), m_onTick( this, &CBlockDetectUI::OnTick )
	{
		SET_BASEOBJECT_ID( CBlockDetectUI );
	}

	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;

	void SetShown( bool bShow ) { m_bShow = bShow; }
	void Toggle() { m_bShow = !m_bShow; }
private:
	void OnTick();

	float m_fMaxDetectRange;
	float m_fFadeDist;
	float m_fFadeInSpeed;

	bool m_bShow;
	float m_fDetectRange;

	TResourceRef<CDrawableGroup> m_pUIDrawable;
	vector<CReference<CBlockObject> > m_vecBlocks;
	vector<CReference<CRenderObject2D> > m_vecPool;
	TClassTrigger<CBlockDetectUI> m_onTick;
};