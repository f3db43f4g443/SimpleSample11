#pragma once
#include "Block.h"
#include "Entities/BlockItems/BlockItemsLv2.h"

class CLv2RandomChunk1 : public CChunkObject
{
	friend void RegisterGameClasses();
public:
	CLv2RandomChunk1( const SClassCreateContext& context ) : CChunkObject( context ) { SET_BASEOBJECT_ID( CLv2RandomChunk1 ); }
	virtual void OnSetChunk( SChunk* pChunk, class CMyLevel* pLevel ) override;
	virtual void OnCreateComplete( class CMyLevel* pLevel ) override;
private:
	uint32 m_nHpPerSize;
};

class CGarbageBin1 : public CChunkObject
{
	friend void RegisterGameClasses();
public:
	CGarbageBin1( const SClassCreateContext& context ) : CChunkObject( context ) { SET_BASEOBJECT_ID( CGarbageBin1 ); }
	virtual void OnSetChunk( SChunk* pChunk, class CMyLevel* pLevel ) override;
	virtual void OnLandImpact( uint32 nPreSpeed, uint32 nCurSpeed ) override;
protected:
	virtual void OnKilled() override;
	void Trigger();
	uint32 m_nTriggerImpact;
	uint32 m_nMaxTriggerCount;
	uint32 m_nMaxDeathTriggerCount;
	float m_h;
	float m_fMaxTriggerHeight;
	TResourceRef<CPrefab> m_pBullet;
	TResourceRef<CPrefab> m_pBullet1;
};

class CBarrel : public CExplosiveChunk
{
	friend void RegisterGameClasses();
public:
	CBarrel( const SClassCreateContext& context ) : CExplosiveChunk( context ) { SET_BASEOBJECT_ID( CBarrel ); }
	virtual bool Damage( SDamageContext& context ) override;
protected:
	TResourceRef<CPrefab> m_strBullet;
	TResourceRef<CPrefab> m_strExp;

	virtual void OnKilled() override { m_pChunk->bStopMove = true; CExplosiveChunk::OnKilled(); }
	virtual void Explode() override;
};

class CBarrel1 : public CExplosiveChunk
{
	friend void RegisterGameClasses();
public:
	CBarrel1( const SClassCreateContext& context ) : CExplosiveChunk( context ) { SET_BASEOBJECT_ID( CBarrel ); }
	virtual bool Damage( SDamageContext& context ) override;
protected:
	TResourceRef<CPrefab> m_strBullet;
	TResourceRef<CPrefab> m_strBullet1;
	TResourceRef<CPrefab> m_strExp;

	virtual void OnKilled() override;
	virtual void Explode() override;
};

class CHousePart : public CChunkObject
{
	friend void RegisterGameClasses();
	friend class CHouse;
public:
	CHousePart( const SClassCreateContext& context ) : CChunkObject( context ) { SET_BASEOBJECT_ID( CHousePart ); }
	virtual void OnSetChunk( SChunk* pChunk, class CMyLevel* pLevel ) override;
	void Explode();
protected:
	virtual void OnKilled() override;
private:
	uint32 m_nHpPerSize;
	TResourceRef<CPrefab> m_pEntrancePrefabs[4];
	TResourceRef<CPrefab> m_pExp;

	vector<CReference<CHouseEntrance> > m_houseEntrances;
};

class CHouse : public CChunkObject
{
	friend void RegisterGameClasses();
public:
	CHouse( const SClassCreateContext& context ) : CChunkObject( context ), m_onTick( this, &CHouse::OnTick ){ SET_BASEOBJECT_ID( CHouse ); }
	virtual void OnSetChunk( SChunk* pChunk, class CMyLevel* pLevel ) override;
	virtual void OnRemovedFromStage() override;
	bool CanEnter( CCharacter* pCharacter );
	bool Enter( CCharacter* pCharacter );
	virtual void OnCreateComplete( class CMyLevel* pLevel ) override;
	void DelayExplode();
	void Explode();
	CCharacter* GetOneThrowObj();
	CVector4* GetParam( int8 i ) { return m_params[i]; }
protected:
	virtual void OnKilled() override;

	void OnTick();
private:
	uint8 m_nType;
	uint32 m_nHpPerSize;

	TResourceRef<CPrefab> m_pInitCharPrefabs[4];
	float m_fInitCharPerGrid[4];

	TResourceRef<CPrefab> m_pThrowObjPrefabs[4];
	uint32 m_nThrowObjMin[4];
	uint32 m_nThrowObjMax[4];

	TResourceRef<CPrefab> m_pExp;
	TResourceRef<CPrefab> m_pExp1;
	TResourceRef<CPrefab> m_pExpEft;
	TResourceRef<CPrefab> m_pEft1;

	float m_fHeightOfs;
	float m_fHeightOfs1;
	uint8 m_nTag1;
	uint8 m_nTag2;

	bool m_bAnyoneEntered;
	bool m_bExploding;
	bool m_bExploded;
	int32 m_nCount;
	vector<pair<CReference<CCharacter>, int32> > m_characters;
	vector<CReference<CHouseEntrance> > m_houseEntrances;
	vector<CReference<CHouseWindow> > m_windows;
	vector<int8> m_throwObjs;

	int32 m_nY[3];
	CVector4 m_params[3][3];

	TClassTrigger<CHouse> m_onTick;
};

class CCargo1 : public CChunkObject
{
	friend void RegisterGameClasses();
public:
	CCargo1( const SClassCreateContext& context ) : CChunkObject( context ), m_onTick( this, &CCargo1::OnTick ) { SET_BASEOBJECT_ID( CCargo1 ); }
	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;
	virtual void OnSetChunk( SChunk* pChunk, CMyLevel* pLevel ) override;
protected:
	virtual void OnKilled() override;
private:
	void GenLayer1();
	void GenLayer2();
	void GenObj( const TRectangle<int32>& rect, uint8 nType );
	void OnTick();
	uint32 m_nHpPerSize;
	bool m_bType;
	CRectangle m_texRect1;
	TResourceRef<CDrawableGroup> m_pDeco;
	TResourceRef<CDrawableGroup> m_pDeco1;
	TResourceRef<CPrefab> m_pPrefab[10];

	struct SMapItem
	{
		SMapItem() : n( 0 ), n1( 0 ), bFlag( 0 ) {}
		uint16 n;
		uint8 n1;
		uint8 bFlag;
	};
	vector<SMapItem> m_blobMap;
	int32 GetMapItem( int32 x, int32 y );
	uint8 GetMapItemValue( int32 x, int32 y );
	vector<TVector2<int32> > m_q;
	int32 m_nMaxIndex;
	TClassTrigger<CCargo1> m_onTick;
	CReference<CEntity> m_pBlob;
};

class CCargoAutoColor : public CDecorator
{
	friend void RegisterGameClasses();
public:
	CCargoAutoColor( const SClassCreateContext& context ) : CDecorator( context ) { SET_BASEOBJECT_ID( CCargoAutoColor ); }

	virtual void Init( const CVector2& size, SChunk* pPreParent ) override;
	CVector3* GetColors() { return m_colors; }
private:
	CVector3 m_colors[3];
};

class CRoadSign : public CDecorator
{
	friend void RegisterGameClasses();
public:
	CRoadSign( const SClassCreateContext& context ) : CDecorator( context ) { SET_BASEOBJECT_ID( CRoadSign ); }

	virtual void Init( const CVector2& size, SChunk* pPreParent ) override;
};

class CBillboardDeco : public CDecorator
{
	friend void RegisterGameClasses();
public:
	CBillboardDeco( const SClassCreateContext& context ) : CDecorator( context ) { SET_BASEOBJECT_ID( CBillboardDeco ); }

	virtual void Init( const CVector2& size, SChunk* pPreParent ) override;
};

class CControlRoomSubChunk : public CChunkObject
{
	friend void RegisterGameClasses();
	friend class CControlRoom;
public:
	CControlRoomSubChunk( const SClassCreateContext& context ) : CChunkObject( context ), m_nDist( -1 )
		, m_onBlobKilled( this, &CControlRoomSubChunk::OnBlobKilled ), m_onTick( this, &CControlRoomSubChunk::OnTick ) { SET_BASEOBJECT_ID( CControlRoomSubChunk ); }

	virtual void OnSetChunk( SChunk* pChunk, CMyLevel* pLevel ) override;
	virtual void Kill() override;
	virtual void OnRemovedFromStage() override;
	void DelayKill();
private:
	void OnBlobKilled();
	void OnTick() { if( m_pBlob ) m_pBlob->Kill(); }
	CReference<CEnemy> m_pBlob;
	CReference<CRenderObject2D> m_pBlobRenderObject;
	uint8 m_nType;
	uint8 m_nType1;
	CRectangle m_tex1;

	int32 m_nDist;
	vector<CRectangle> m_vecTexs;
	TClassTrigger<CControlRoomSubChunk> m_onBlobKilled;
	TClassTrigger<CControlRoomSubChunk> m_onTick;
};

class CControlRoom : public CChunkObject
{
	friend void RegisterGameClasses();
public:
	CControlRoom( const SClassCreateContext& context ) : CChunkObject( context ) { SET_BASEOBJECT_ID( CControlRoom ); }

	virtual void OnRemovedFromStage() override;
	virtual void OnSetChunk( SChunk * pChunk, CMyLevel * pLevel ) override;
	virtual void OnCreateComplete( class CMyLevel* pLevel ) override;
	CRenderObject2D* GetLayer1() { return m_pLayer1; }
	CWindow3Controller* GetController() { return m_pController; }

	void OnSubChunkAdded( CControlRoomSubChunk* p );
	void OnSubChunkKilled( CControlRoomSubChunk* p );
private:
	uint32 m_nHpPerSize;
	TResourceRef<CDrawableGroup> m_pDeco;
	TResourceRef<CPrefab> m_pWindow[4];
	vector<CReference<CControlRoomSubChunk> > m_vecSub;

	CReference<CRenderObject2D> m_pLayer1;
	CReference<CWindow3Controller> m_pController;
};

class CBillboard1 : public CChunkObject
{
	friend void RegisterGameClasses();
public:
	CBillboard1( const SClassCreateContext& context ) : CChunkObject( context ) { SET_BASEOBJECT_ID( CBillboard1 ); }
	virtual void OnSetChunk( SChunk* pChunk, class CMyLevel* pLevel ) override;
private:
	uint32 m_nHpPerSize;
	TResourceRef<CDrawableGroup> m_pDeco;
};

class CHouse2 : public CChunkObject
{
	friend void RegisterGameClasses();
public:
	CHouse2( const SClassCreateContext& context ) : CChunkObject( context ) { SET_BASEOBJECT_ID( CHouse2 ); }
	virtual void OnSetChunk( SChunk* pChunk, class CMyLevel* pLevel ) override;
	virtual void OnCreateComplete( class CMyLevel* pLevel ) override;
	virtual void Kill() override;
	virtual void Crush() override;
	virtual bool Damage( SDamageContext& context ) override;
protected:
	virtual void OnKilled() override;
private:
	void GenObjs();
	void GenObj( const CVector2& p, uint8 nType );

	void AIFunc();
	class AI : public CAIObject
	{
	protected:
		virtual void AIFunc() override { static_cast<CHouse2*>( GetParentEntity() )->AIFunc(); }
	};
	AI* m_pAI;

	uint32 m_nHp1;
	uint32 m_nHpPerSize;
	uint32 m_nHpPerSize1;
	CRectangle m_texRect;
	CRectangle m_texRect1;
	CRectangle m_texRect2;
	TResourceRef<CPrefab> m_pObjPrefab[3];
	CReference<CEntity> m_pDecorator;
	uint32 m_nDecoTexSize;
	CReference<CEntity> m_pObj[3];
	CReference<CEntity> m_pObj1[3];
	CReference<CEntity> m_pDoor;
	TResourceRef<CPrefab> m_pBullet[4];

	static void CreateBarrage( uint32 nType, CEntity* pRef, const CVector2& ofs, CPrefab* pBullets[4] );

	struct SObj
	{
		CReference<CRenderObject2D> pImg;
		CReference<CRenderObject2D> pImg1;
		CVector2 ofs;
		uint8 nType;
	};
	vector<SObj> m_vecObj;
	vector<CReference<CRenderObject2D> > m_vecBlock1;
	uint8 m_nState;
	bool m_bDamaged;
};