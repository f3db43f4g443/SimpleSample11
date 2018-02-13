#pragma once
#include "Entity.h"
#include "Render/DrawableGroup.h"
#include "Common/Rand.h"

class CLimbsEft : public CEntity
{
	friend void RegisterGameClasses();
public:
	CLimbsEft( const SClassCreateContext& context ) : CEntity( context ), m_onTick( this, &CLimbsEft::OnTick ) { SET_BASEOBJECT_ID( CLimbsEft ); }

	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;

	CRenderObject2D* GetImg( uint8 n ) { return m_pImg[n]; }
private:
	void OnTick();

	CVector2 m_ofs;
	float m_fSize;
	uint32 m_nFrames;

	int32 m_nTick;
	CReference<CRenderObject2D> m_pImg[4];
	TClassTrigger<CLimbsEft> m_onTick;
};

class CLimbsAttackEft : public CEntity
{
	friend void RegisterGameClasses();
public:
	CLimbsAttackEft( const SClassCreateContext& context ) : CEntity( context ), m_onTick( this, &CLimbsAttackEft::OnTick ) { SET_BASEOBJECT_ID( CLimbsAttackEft ); }
	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;

	void CreateAttackEft( CLimbsEft* pLimbsEft, uint8 nMask = 0 ) { CreateAttackEft( pLimbsEft->GetImg( 3 ), pLimbsEft->GetImg( 2 ), nMask ); }
	void CreateAttackEft( CRenderObject2D* pRenderParent1, CRenderObject2D* pRenderParent2, uint8 nMask = 0 );
	void SetAttackEftLen( float fLen );
	void DestroyAttackEft();
private:
	void OnTick();

	float m_fSize;
	uint32 m_nFrames;

	int32 m_nTick;
	float m_fEftLen;
	CReference<CRenderObject2D> m_pAttackEft[8];
	TClassTrigger<CLimbsAttackEft> m_onTick;
};