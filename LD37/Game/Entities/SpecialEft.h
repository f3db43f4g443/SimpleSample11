#pragma once
#include "Entity.h"
#include "Render/DrawableGroup.h"
#include "Common/Rand.h"

class CEnemyPileEft : public CEntity
{
	friend void RegisterGameClasses();
public:
	CEnemyPileEft( const SClassCreateContext& context ) : CEntity( context ), m_onTick( this, &CEnemyPileEft::OnTick ) { SET_BASEOBJECT_ID( CEnemyPileEft ); }
	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;
private:
	void OnTick();
	uint32 m_nEft;
	uint32 m_nEftBegin[4];
	uint32 m_nEftCount[4];
	uint32 m_nFrames;

	int32 m_nImg;
	CReference<CRenderObject2D> m_pImg[4];
	TClassTrigger<CEnemyPileEft> m_onTick;
};

class CLimbsEft : public CEntity
{
	friend void RegisterGameClasses();
public:
	CLimbsEft( const SClassCreateContext& context ) : CEntity( context ), m_onTick( this, &CLimbsEft::OnTick ) { SET_BASEOBJECT_ID( CLimbsEft ); }

	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;
	void SetMask( uint8 nMask );
	CRenderObject2D* GetImg( uint8 n ) { return m_pImg[n]; }
private:
	void OnTick();

	CVector2 m_ofs;
	float m_fSize;
	uint32 m_nFrames;

	uint8 m_nMask;
	int32 m_nTick;
	CRectangle m_texRect[4];
	int32 m_nSubFrames[4];
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

	void CreateAttackEft( CLimbsEft* pLimbsEft, uint8 nMask = 0 ) { CreateAttackEft( pLimbsEft->GetImg( 1 ), pLimbsEft->GetImg( 3 ), nMask ); }
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

class CManChunkEft : public CEntity
{
	friend void RegisterGameClasses();
public:
	CManChunkEft( const SClassCreateContext& context ) : CEntity( context ), m_onTick( this, &CManChunkEft::OnTick ) { SET_BASEOBJECT_ID( CManChunkEft ); }

	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;
	virtual void UpdateRendered( double dTime ) override;
	virtual void Render( CRenderContext2D& context ) override;
	float GetRadius() { return m_fRadius; }
	void Set( float fRadius );
	void Kill();
private:
	TResourceRef<CPrefab> m_pKillEft;
	TResourceRef<CSoundFile> m_pKillSound;
	float m_fKillEftSize;
	float m_fKillSpeed;

	struct SElem
	{
		SElem() : nType( -1 )
		{
			m_element2D.worldMat.Identity();
		}
		CElement2D m_element2D;
		CVector2 p;
		uint8 nType;
		int8 nAnim;
	};
	bool SetElem( SElem& elem, int32 nX, int32 nY, int32 nSize, float fRad, float fRad1 );
	void CalcElemTex( SElem& elem );
	int8 GetAnim( int32 nX, int32 nY, int32 nType );
	void OnTick();
	vector<SElem> m_elems;
	int32 m_nElem;
	vector<int8> m_elemAnim;
	CReference<CRenderObject2D> m_pImg;
	float m_fTime;
	float m_fRadius;
	float m_fKillRadius;
	int32 m_nKillEft;
	TClassTrigger<CManChunkEft> m_onTick;
};

class CEyeEft : public CEntity
{
	friend void RegisterGameClasses();
public:
	CEyeEft( const SClassCreateContext& context ) : CEntity( context ), m_onTick( this, &CEyeEft::OnTick ) { SET_BASEOBJECT_ID( CEyeEft ); }
	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;
	virtual void UpdateRendered( double dTime ) override;
	virtual void Render( CRenderContext2D& context ) override;
	void SetTarget( const CVector2& p );
	void SetEyeColor( uint8 nEyeColor ) { m_nEyeColor = nEyeColor; }
	CVector2 GetCenter() { return m_elems[0].p; }
private:
	uint8 m_nElems;
	bool m_bAuto;
	struct SElem
	{
		SElem()
		{
			m_element2D.worldMat.Identity();
		}
		CElement2D m_element2D;
		CVector2 p;
		int8 nAnim;
	};
	void OnTick();
	void CalcElemTex( SElem& elem, int32 i );

	uint8 m_nEyeColor;
	SElem m_elems[4];
	float m_fTime;
	CReference<CRenderObject2D> m_pImg;
	TClassTrigger<CEyeEft> m_onTick;
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