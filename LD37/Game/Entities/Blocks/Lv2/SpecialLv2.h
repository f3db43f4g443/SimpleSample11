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

	virtual void OnKilled() override { m_pChunk->bStopMove = true; }
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

	bool m_bAnyoneEntered;
	bool m_bExploded;
	int32 m_nEftCount;
	vector<pair<CReference<CCharacter>, int32> > m_characters;
	vector<CReference<CHousePart> > m_houseParts;
	vector<CReference<CHouseEntrance> > m_houseEntrances;
	vector<int8> m_throwObjs;

	TClassTrigger<CHouse> m_onTick;
};