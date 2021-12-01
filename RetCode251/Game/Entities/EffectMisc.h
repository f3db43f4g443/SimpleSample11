#pragma once
#include "Entity.h"

class CDropBombEffect : public CEntity
{
	friend void RegisterGameClasses_EffectMisc();
public:
	CDropBombEffect( const SClassCreateContext& context ) : CEntity( context ), m_onTick( this, &CDropBombEffect::OnTick ) { SET_BASEOBJECT_ID( CDropBombEffect ); }
	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;
	void Init();
	virtual void Render( CRenderContext2D& context ) override;
private:
	void OnTick();
	int32 m_nLife;
	float m_fImgWidth;
	CVector3 m_widthParam;
	CVector3 m_heightParam;
	CVector3 m_height1Param;

	int32 m_nTick;
	CReference<CRenderObject2D> m_pOrigRenderObject;
	struct SElem
	{
		int32 nTexX;
		CElement2D elem;
	};
	vector<SElem> m_vecElems;
	TClassTrigger<CDropBombEffect> m_onTick;
};

class CSpinEffect : public CEntity
{
	friend void RegisterGameClasses_EffectMisc();
public:
	CSpinEffect( const SClassCreateContext& context ) : CEntity( context ), m_onTick( this, &CSpinEffect::OnTick ) { SET_BASEOBJECT_ID( CSpinEffect ); }
	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;
	void Init( int32 nSeed );
	virtual void Render( CRenderContext2D& context ) override;
private:
	void OnTick();
	int32 m_nFrameSpeed;
	int32 m_nRows;
	CVector2 m_texOfs;
	int32 m_nLoopImgCount;
	TArray<int8> m_arrLoopAnim;
	CRectangle m_loopOffset;
	int32 m_nSpawnInterval;
	int32 m_nInitTime;
	CRectangle m_spawnRect;
	bool m_bFront;
	bool m_bBack;

	bool m_bInited;
	CReference<CRenderObject2D> m_pOrigRenderObject;
	CRectangle m_origRect;
	CRectangle m_origTexRect;
	int32 m_nSeed;
	struct SItem
	{
		int32 nTime;
		int32 nInitFrame;
		CVector2 initOfs;
		CVector2 loopOfs;
	};
	vector<SItem> m_vecItems;
	int32 m_nItemBegin, m_nItemEnd;
	int32 m_nSpawnTick;
	vector<CElement2D> m_vecElems;
	TClassTrigger<CSpinEffect> m_onTick;
};