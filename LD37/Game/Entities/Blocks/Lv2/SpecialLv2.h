#pragma once
#include "Block.h"
#include "Entities/BlockItems/BlockItemsLv2.h"

class CBarrel : public CExplosiveChunk
{
	friend void RegisterGameClasses();
public:
	CBarrel( const SClassCreateContext& context ) : CExplosiveChunk( context ) { SET_BASEOBJECT_ID( CBarrel ); }
	virtual void Damage( SDamageContext& context ) override;
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
	virtual void Damage( SDamageContext& context ) override;
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
protected:
	virtual void OnKilled() override;

	void OnTick();
private:
	uint32 m_nHpPerSize;

	TResourceRef<CPrefab> m_pInitCharPrefabs[4];
	float m_fInitCharPerGrid[4];

	TResourceRef<CPrefab> m_pThrowObjPrefabs[4];
	uint32 m_nThrowObjMin[4];
	uint32 m_nThrowObjMax[4];

	TResourceRef<CPrefab> m_pExp;
	TResourceRef<CPrefab> m_pExpEft;
	TResourceRef<CPrefab> m_pEft1;

	bool m_bAnyoneEntered;
	bool m_bExploding;
	bool m_bExploded;
	int32 m_nCount;
	vector<pair<CReference<CCharacter>, int32> > m_characters;
	vector<CReference<CHousePart> > m_houseParts;
	vector<CReference<CHouseEntrance> > m_houseEntrances;
	vector<int8> m_throwObjs;

	TClassTrigger<CHouse> m_onTick;
};

class CControlRoom : public CChunkObject
{
	friend void RegisterGameClasses();
public:
	CControlRoom( const SClassCreateContext& context ) : CChunkObject( context ) { SET_BASEOBJECT_ID( CControlRoom ); }

	virtual void OnSetChunk( SChunk * pChunk, CMyLevel * pLevel ) override;
	virtual void OnCreateComplete( class CMyLevel* pLevel ) override;

	virtual void Damage( SDamageContext& context ) override;
protected:
	virtual void OnKilled() override;
private:
	uint8 m_nType;
	uint8 m_nAltX, m_nAltY;
	uint32 m_nHpPerSize;
	TResourceRef<CPrefab> m_pWindow[4];
	CReference<CEntity> m_pDmgParticles[4];
	int32 m_nDmgParticles;
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
	virtual void Damage( SDamageContext& context ) override;
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