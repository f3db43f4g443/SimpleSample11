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

class CArmEft : public CEntity
{
	friend void RegisterGameClasses();
public:
	CArmEft( const SClassCreateContext& context ) : CEntity( context ) { SET_BASEOBJECT_ID( CArmEft ); }
	virtual void UpdateRendered( double dTime ) override;
	virtual void Render( CRenderContext2D& context ) override;
	void Init( const CVector2& begin, const CVector2& end, const CVector2& space, const CVector2& ofs, const TVector2<int32>& dim, int8* pMask );
	void Set( const CVector2& begin, const CVector2& end );
private:
	struct SElem
	{
		vector<CElement2D> m_element2D;
		CVector2 ofs0;
		CVector2 ofs;
		int8 nTex;
		int8 nTick;
		bool bMask;
	};
	void SetLen( SElem& elem, float fLen );

	float m_fSize;

	vector<SElem> m_elems;
	TVector2<int32> m_dim;
	CVector2 m_begin;
	CVector2 m_end;
	CVector2 m_space;
	CVector2 m_ofs;
	CMatrix2D m_mat0;
	float m_fTime;
	CReference<CRenderObject2D> m_pImg;
	TClassTrigger<CArmEft> m_onTick;
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
	void SetBound( const CRectangle& bound ) { m_bound = bound; }
	void Kill();
private:
	TResourceRef<CPrefab> m_pKillEft;
	TResourceRef<CSoundFile> m_pKillSound;
	float m_fKillEftSize;
	float m_fKillSpeed;
	CReference<CEntity> m_pFangEft;
	uint8 m_nFangEftType;

	struct SElem
	{
		SElem() : nType( -1 )
		{
			m_element2D.worldMat.Identity();
		}
		CElement2D m_element2D;
		CRectangle rect0;
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
	CRectangle m_bound;
	float m_fTime;
	float m_fRadius;
	float m_fKillRadius;
	int32 m_nKillEft;
	TClassTrigger<CManChunkEft> m_onTick;
};

class CManChunkFangEft : public CEntity
{
	friend void RegisterGameClasses();
public:
	CManChunkFangEft( const SClassCreateContext& context ) : CEntity( context ) { SET_BASEOBJECT_ID( CManChunkFangEft ); }

	virtual void OnAddedToStage() override;
	virtual void UpdateRendered( double dTime ) override;
	virtual void Render( CRenderContext2D& context ) override;
	void Set( float fHalfLen );
private:
	struct SElem
	{
		SElem()
		{
			for( int i = 0; i < 3; i++ )
				m_element2D[i].worldMat.Identity();
		}
		CElement2D m_element2D[3];
		int8 nAnim[2];
	};
	vector<SElem> m_elems;
	int32 m_nElem;
	void CalcElemTex( SElem& elem );

	CReference<CRenderObject2D> m_pImg;
	float m_fTime;
};

class CManBlobEft : public CEntity
{
	friend void RegisterGameClasses();
public:
	CManBlobEft( const SClassCreateContext& context ) : CEntity( context ), m_onTick( this, &CManBlobEft::OnTick ) { SET_BASEOBJECT_ID( CManBlobEft ); }

	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;
	virtual void UpdateRendered( double dTime ) override;
	virtual void Render( CRenderContext2D& context ) override;
	void Set( int32 nMapSize, uint8* pMap );
	void Kill( int32 nMapSize, uint8* pMap );
private:
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
	bool SetElem( SElem& elem, int32 nX, int32 nY, int32 nX1, int32 nY1, uint8 n );
	void CalcElemTex( SElem& elem );
	int8 GetAnim( int32 nX, int32 nY );
	void OnTick();

	int32 m_nType;
	TResourceRef<CPrefab> m_pKillEft;
	float m_fKillSpeed;

	vector<SElem> m_elems;
	int32 m_nElem;
	vector<int8> m_elemAnim;
	CReference<CRenderObject2D> m_pImg;
	float m_fTime;
	vector<uint8> m_tempMap;
	int32 m_nTempMapSize;
	TClassTrigger<CManBlobEft> m_onTick;
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