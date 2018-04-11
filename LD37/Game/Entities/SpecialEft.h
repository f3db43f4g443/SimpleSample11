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

class CAuraEft : public CEntity
{
	friend void RegisterGameClasses();
public:
	CAuraEft( const SClassCreateContext& context ) : CEntity( context ) { SET_BASEOBJECT_ID( CAuraEft ); }

	virtual void OnAddedToStage() override;

	virtual void UpdateRendered( double dTime ) override;
	virtual void Render( CRenderContext2D& context ) override;
private:
	uint32 m_nCount;
	uint32 m_nCols;
	uint32 m_nRows;
	float m_fWidth;
	float m_fHeight;
	CRectangle m_ofs;
	float m_fAngularSpeed;

	CReference<CRenderObject2D> m_pImg;
	int32 m_nTime0;
	struct SItem
	{
		float r0;
		float fAngularSpeed;
		CElement2D elem;
	};
	vector<SItem> m_items;
};