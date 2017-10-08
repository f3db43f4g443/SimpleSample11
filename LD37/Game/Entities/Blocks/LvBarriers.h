#pragma once
#include "Block.h"
#include "RandomBlocks.h"
#include "Entities/AIObject.h"
#include "Entities/Barrage.h"
#include "LevelGenerate.h"

class CLvFloor1 : public CRandomChunkTiled
{
	friend void RegisterGameClasses();
public:
	CLvFloor1( const SClassCreateContext& context ) : CRandomChunkTiled( context ), m_nKilledCrates( 0 ) { SET_BASEOBJECT_ID( CLvFloor1 ); }
	virtual void OnCreateComplete( class CMyLevel* pLevel ) override;
private:
	void OnCrateKilled( int32 i );
	void OnPickUp();

	CString m_strCrate;
	CString m_strItemDrop;
	float m_fWeights[4];
	uint32 m_nKilledCrates;
	vector<CReference<CChunkObject> > m_vecCrates;
	vector<CReference<CEntity> > m_vecPickups;
	vector<CFunctionTrigger> m_triggers;
};

class CLvFloor2 : public CChunkObject
{
	friend void RegisterGameClasses();
public:
	CLvFloor2( const SClassCreateContext& context ) : CChunkObject( context ), m_nKilledCrates( 0 ), m_onTick( this, &CLvFloor2::OnTick ) { SET_BASEOBJECT_ID( CLvFloor2 ); }
	virtual void OnSetChunk( SChunk* pChunk, class CMyLevel* pLevel ) override;
	virtual void OnCreateComplete( class CMyLevel* pLevel ) override;

	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;
protected:
	void OnCrateKilled( int32 i );
	void OnPickUp();
	void OnTick();
	virtual void OnKilled() override;

	CString m_strItemDrop;
	float m_fWeights[4];
	TResourceRef<CPrefab> m_pPrefab;
	TResourceRef<CPrefab> m_pPrefab1;
	TResourceRef<CPrefab> m_pCrate;
	TResourceRef<CPrefab> m_pItemDropPrefab;
	TResourceRef<CPrefab> m_pKillEffect;
	uint32 m_nKillEffectInterval;

	uint32 m_nKilledCrates;
	vector<CReference<CEntity> > m_vecSegs;
	vector<CReference<CEntity> > m_vecCrates;
	vector<CReference<CEntity> > m_vecPickups;
	TClassTrigger<CLvFloor2> m_onTick;
	vector<CFunctionTrigger> m_triggers;
	uint32 m_nKillEffectCDLeft;
	uint8 m_nDir;
	bool m_bPicked;
};

class CLvBarrier1Core : public CChunkObject
{
	friend void RegisterGameClasses();
public:
	CLvBarrier1Core( const SClassCreateContext& context ) : CChunkObject( context ), m_strBullet( context ), m_strBullet1( context ), m_strBullet2( context ), m_nPhase( 0 ), m_nSpecialFires( 0 ) { SET_BASEOBJECT_ID( CLvBarrier1Core ); }

	virtual void OnAddedToStage() override { m_pAI = new AI(); m_pAI->SetParentEntity( this ); }
	virtual void Damage( SDamageContext& context ) override { m_nSpecialFires = 1; CChunkObject::Damage( context ); }

	void SetPhase( uint8 nPhase ) { m_nPhase = nPhase; }
protected:
	void AIFunc();
	class AI : public CAIObject
	{
	protected:
		virtual void AIFunc() override { static_cast<CLvBarrier1Core*>( GetParentEntity() )->AIFunc(); }
	};
	AI* m_pAI;

	CString m_strBullet;
	CString m_strBullet1;
	CString m_strBullet2;
	CReference<CPrefab> m_pBulletPrefab;
	CReference<CPrefab> m_pBulletPrefab1;
	CReference<CPrefab> m_pBulletPrefab2;

	float m_fOpenDist[4];
	float m_fCloseDist[4];

	uint8 m_nPhase;
	uint8 m_nSpecialFires;
};

class CLvBarrier1 : public CRandomChunkTiled
{
	friend void RegisterGameClasses();
public:
	CLvBarrier1( const SClassCreateContext& context ) : CRandomChunkTiled( context ), m_strCore( context ), m_strWall( context ), m_strKillEffect( context )
		, m_nCoreCount( 0 ), m_bKilled( false ), m_deathTick( this, &CLvBarrier1::Tick ) { SET_BASEOBJECT_ID( CLvBarrier1 ); }

	virtual void OnRemovedFromStage() override;
	virtual void OnCreateComplete( class CMyLevel* pLevel ) override;

	void OnCoreDestroyed();
	virtual void Kill() override;
private:
	virtual void OnKilled() {}
	void Tick();
	CString m_strCore;
	CString m_strWall;
	TResourceRef<CPrefab> m_strKillEffect;
	uint32 m_nKillEffectInterval;
	uint32 m_nDeathTime;

	bool m_bKilled;
	vector<CReference<CChunkObject> > m_vecRooms;
	vector<CReference<CChunkObject> > m_vecCores;
	vector<CFunctionTrigger> m_triggers;
	TClassTrigger<CLvBarrier1> m_deathTick;
	uint32 m_nKillEffectCDLeft;
	uint32 m_nCoreCount;
};

class CLvBarrierReward1 : public CRandomChunkTiled
{
	friend void RegisterGameClasses();
public:
	CLvBarrierReward1( const SClassCreateContext& context ) : CRandomChunkTiled( context ) { SET_BASEOBJECT_ID( CLvBarrierReward1 ); }
	virtual void OnCreateComplete( class CMyLevel* pLevel ) override;
	virtual void OnLandImpact( uint32 nPreSpeed, uint32 nCurSpeed ) override;
private:
	void OnPickUp();

	CString m_strItemDrop;
	vector<CReference<CEntity> > m_vecPickups;
	vector<CFunctionTrigger> m_triggers;
	bool m_bPickupCreated;
};

class CLvBarrier2Core : public CRandomChunkTiledSimple
{
	friend class CLvBarrier2;
	friend void RegisterGameClasses();
public:
	CLvBarrier2Core( const SClassCreateContext& context ) : CRandomChunkTiledSimple( context ), m_nIndex( -1 ) { SET_BASEOBJECT_ID( CLvBarrier2Core ); }

	void Set( int32 nIndex ) { m_nIndex = nIndex; }

	virtual void OnRemovedFromStage() override;
	virtual void Damage( SDamageContext& context ) override;
private:
	int32 m_nIndex;
	uint8 m_nDmgTime;
	uint32 m_nFireCD;
};

class CLvBarrier2 : public CChunkObject
{
	friend void RegisterGameClasses();
public:
	CLvBarrier2( const SClassCreateContext& context ) : CChunkObject( context ), m_deathTick( this, &CLvBarrier2::KillTick ) { SET_BASEOBJECT_ID( CLvBarrier2 ); }
	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;
	virtual void OnSetChunk( SChunk* pChunk, class CMyLevel* pLevel ) override;
	virtual void OnCreateComplete( class CMyLevel* pLevel ) override;

	virtual void Kill() override;
	void OnChunkRemove( uint32 nIndex );

protected:
	virtual void OnKilled() {}
	void KillTick();
	void Move( bool bSpawnChunk );
	void Move1();
	void CreateExplosion( int32 iX, int32 iY, CChunkObject* pCreator );

	class CBarrage1 : public CBarrage
	{
	public:
		CBarrage1( const SBarrageContext& context ) : CBarrage( context ), m_nTick( 0 ) {}

		void Update()
		{
			for( int i = 0; i < m_items.size(); i++ )
			{
				if( !m_items[i].bValid )
					continue;
				if( !m_nTick )
					m_items[i].nState++;
				UpdateItem( i );
			}
			m_nTick++;
			if( m_nTick == 32 )
				m_nTick = 0;
		}

		void Fire( const TRectangle<int32>& rect, const CVector2& dir )
		{
			SItem* pItem;
			if( m_freedItems.size() )
			{
				pItem = &m_items[m_freedItems.back()];
				m_freedItems.pop_back();
			}
			else
			{
				m_items.resize( m_items.size() + 1 );
				pItem = &m_items.back();
			}
			pItem->bValid = true;
			pItem->initRect = rect;
			pItem->dir = dir;
			pItem->nState = -1;
		}
	protected:
		virtual void UpdateItem( uint32 nIndex ) {}
		void FreeItem( uint32 nIndex )
		{
			assert( m_items[nIndex].bValid );
			m_items[nIndex].bValid = false;
			m_freedItems.push_back( nIndex );
		}

		uint32 m_nTick;
		struct SItem
		{
			bool bValid;
			TRectangle<int32> initRect;
			CVector2 dir;
			int32 nState;
		};

		vector<SItem> m_items;
		vector<uint32> m_freedItems;
	};
	void InitBarrage1();
	void GenBarrage1( uint32 nType, const TRectangle<int32>& rect, const CVector2& dir );
	int32 GenBarrage2( uint32 nType, const TRectangle<int32>& rect );

	void AIFunc();
	class AI : public CAIObject
	{
	protected:
		virtual void AIFunc() override { static_cast<CLvBarrier2*>( GetParentEntity() )->AIFunc(); }
	};
	AI* m_pAI;

	struct SGrid
	{
		CReference<CChunkObject> pChunkObject;
		TVector2<int32> par;
		bool bDirs[4];
		int8 nColor;
		int8 nType;
		int8 nParType;
	};
	vector<SGrid> m_grids;
	vector<TVector2<int32> > m_vecMovingGrids;
	vector<TVector2<int32> > m_q;

	struct SBigChunk
	{
		CReference<CChunkObject> pChunk;
		TRectangle<int32> rect;
	};
	vector<SBigChunk> m_bigChunks;
	uint32 m_nCoreSize;
	uint32 m_nMaxCoreSize;
	bool m_bKilled;
	uint32 m_nKillEffectCDLeft;
	CReference<CBarrage1> m_pBarrages1[4];
	TClassTrigger<CLvBarrier2> m_deathTick;

	CVector2 m_blockTex;
	CString m_strCreateNode;
	CReference<CLevelGenerateNode> m_pCreateNode;
	TResourceRef<CPrefab> m_strKillEffect;
	TResourceRef<CPrefab> m_pExplosion;
	TResourceRef<CPrefab> m_pBullet;
	TResourceRef<CPrefab> m_pBullet1;
	TResourceRef<CPrefab> m_pLightning;
	TResourceRef<CPrefab> m_pLightning0;
	uint32 m_nKillEffectInterval;
	uint32 m_nDeathTime;
};