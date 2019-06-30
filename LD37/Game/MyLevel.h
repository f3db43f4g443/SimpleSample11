#pragma once
#include "Character.h"
#include "Common/StringUtil.h"
#include "Render/DrawableGroup.h"
#include "Block.h"
#include "Render/Sound.h"
#include "Navigation.h"

class CMyLevel : public CEntity, public INavigationProvider
{
	friend void RegisterGameClasses();
	friend struct SLevelBuildContext;
public:
	CMyLevel( const SClassCreateContext& context ) : CEntity( context ), m_fLastScrollPos( 0 ), m_fCurScrollPos( 0 ), m_nCurBarrierHeightTag( 0 ) { SET_BASEOBJECT_ID( CMyLevel ); }

	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;
	virtual void StartUp();
	void BeginGenLevel( int32 nLevel );
	virtual void OnPlayerKilled( class CPlayer* pPlayer );
	virtual void OnPlayerEntered( class CPlayer* pPlayer );
	void Clear();

	void KillChunk( SChunk* pChunk, bool bCrush = false, CChunkObject* pPreObject = NULL );

	void RemoveChunk( SChunk* pChunk );
	void SplitChunks( SChunk* pOldChunk, vector< pair<SChunk*, TVector2<int32> > >& newChunks, CChunkObject* pPreObject = NULL );

	void RemoveChain( SChain* pChain );

	FORCE_INLINE static uint32 GetBlockSize() { return 32; }
	FORCE_INLINE float GetFallDistPerSpeedFrame() { return m_fFallDistPerSpeedFrame; }
	FORCE_INLINE float GetShakeStrength() { return m_fShakeStrength + m_fShakeStrengthCurFrame; }
	FORCE_INLINE void AddShakeStrength( float fShakeStrength ) { m_fShakeStrength += fShakeStrength; }
	FORCE_INLINE void AddShakeStrengthCurFrame( float fShakeStrength ) { m_fShakeStrengthCurFrame += fShakeStrength; }

	FORCE_INLINE float GetLastScrollPos() { return m_fLastScrollPos; }
	FORCE_INLINE float GetCurScrollPos() { return m_fCurScrollPos; }
	FORCE_INLINE uint32 GetCurHeightTag() { return m_vecBarrierHeightTags.size() ? m_vecBarrierHeightTags.back() : 0; }
	bool IsReachEnd();

	enum
	{
		eGridFlag_BlockMove = 1,
		eGridFlag_BlockSight = 2,
	};

	enum
	{
		eBulletLevel_Player,
		eBulletLevel_Enemy,

		eBulletLevel_Count,
	};

	FORCE_INLINE static CMyLevel* GetInst() { return s_pLevel; }
	
	FORCE_INLINE bool IsLevelDesignTest() { return m_bIsLevelDesignTest; }
	FORCE_INLINE uint32 GetWidth() { return m_nWidth; }
	FORCE_INLINE CRectangle GetBound() { return CRectangle( 0, 0, m_nWidth * GetBlockSize(), m_nSpawnHeight * GetBlockSize() ); }
	FORCE_INLINE float GetLvBarrierHeight() { return m_fCurLvBarrierHeight; }
	FORCE_INLINE CRectangle GetBoundWithLvBarrier() { return CRectangle( 0, 0, m_nWidth * GetBlockSize(), Min<float>( m_fCurLvBarrierHeight, m_nSpawnHeight * GetBlockSize() ) ); }
	FORCE_INLINE CRectangle GetLargeBound() { auto bound = GetBound(); return CRectangle( bound.x - 512, bound.y - 512, bound.width + 1024, bound.height + 1024 ); }
	FORCE_INLINE float GetHighGravityHeight() { return 256.0f; }
	FORCE_INLINE CEntity* GetScrollObjRoot( uint32 i ) { return m_pScrollObjRoot[i]; }
	FORCE_INLINE CEntity* GetChunkRoot() { return m_pChunkRoot; }
	FORCE_INLINE CEntity* GetChunkRoot1() { return m_pChunkRoot1; }
	FORCE_INLINE CEntity* GetChunkEffectRoot() { return m_pChunkEffectRoot; }
	FORCE_INLINE CEntity* GetBulletRoot( uint8 nLevel ) { return m_pBulletRoot[nLevel]; }
	FORCE_INLINE CRenderObject2D* GetBack0() { return m_pBack0; }
	FORCE_INLINE CRenderObject2D* GetCrosshair() { return m_pCrosshair; }
	void UpdateBack0Position( const CVector2& pos );
	virtual CVector2 GetCamPos();
	FORCE_INLINE bool IsBonusStage() { return m_bIsBonusStage; }
	FORCE_INLINE bool IsBonusStageEnd() { return m_bIsBonusStageEnd; }
	void BeginBonusStage();
	void EndBonusStage();
	void OnPlayerBonusStageCrushed();
	SChunk* GetCurLevelBarrier();

	void UpdateBlocksMovement();
	void UpdateBlockRT();

	FORCE_INLINE void OnBlockObjectRemoved( CBlockObject* pBlockObject )
	{
		if( pBlockObject->m_nBlockRTIndex >= 0 )
		{
			FreeBlockRTRect( pBlockObject->m_nBlockRTIndex );
			pBlockObject->m_nBlockRTIndex = -1;
			if( pBlockObject->m_pBlockRTObject )
			{
				pBlockObject->m_pBlockRTObject->RemoveThis();
				pBlockObject->m_pBlockRTObject = NULL;
			}
		}
	}
	void AddBlockRTElem( CRenderObject2D* pElem ) { pElem->SetRenderParent( m_pBlockElemRoot ); }

	virtual uint8 GetNavigationData( const TVector2<int32>& pos ) override;
	virtual TRectangle<int32> GetMapRect() override;
	virtual CVector2 GetGridSize() override { return CVector2( GetBlockSize(), GetBlockSize() ); }

	CChunkObject* TrySpawnAt( CVector2& pos, SHitProxy* pHitProxy );
protected:
	SChunk* CreateGrids( const char* szNode, SChunk* pLevelBarrier = NULL );
	SChunk* CacheNextLevel( SChunk* pLevelBarrier );
	void UpdateBlocksMovementNormal();
	void UpdateBlocksMovementBonusStage();
	void UpdateBlocksMovementBonusStageEnd();
	uint32 m_nCurLevel;
	uint32 m_nGenLevel;

	bool m_bBeginGenLevel;
	bool m_bIsLevelDesignTest;
	bool m_bIsBonusStage;
	bool m_bIsBonusStageEnd;
	uint32 m_nWidth;
	uint32 m_nHeight;
	uint32 m_nSpawnHeight;
	float m_fCurLvBarrierHeight;
	float m_fLastScrollPos;
	float m_fCurScrollPos;
	vector<int32> m_vecBarrierHeightTags;
	uint32 m_nCurBarrierHeightTag;
	float m_fShakeStrength;
	float m_fShakeStrengthCurFrame;

	struct SBasement
	{
		SBasement() { memset( this, 0, sizeof( SBasement ) ); }

		struct SBasementLayer
		{
			SBlockLayer* pSpawnedBlock;

			SBlockLayer* pVisitedBlock;
			int32 nCurMargin;
			int32 nCurShakeStrength;
			int32 nShakeHeight;

			LINK_LIST_HEAD( pBlock, SBlockLayer, BlockLayer )
		};

		SBasementLayer layers[2];
	};
	vector<SBasement> m_basements;
	float m_fFallDistPerSpeedFrame;

	void UpdateShake();
	void CheckSpawn();

	TClassTrigger<CMyLevel> m_onTick;

	CReference<CRenderObject2D> m_pCrosshair;
	CReference<CRenderObject2D> m_pBack0;

	CReference<CEntity> m_pScrollObjRoot[3];
	CReference<CEntity> m_pChunkRoot;
	CReference<CEntity> m_pChunkRoot1;
	CReference<CEntity> m_pChunkEffectRoot;
	CReference<CEntity> m_pBulletRoot[eBulletLevel_Count];

	TResourceRef<CTextureFile> m_pBlockRT;

	CReference<CRenderObject2D> m_pBlockElemRoot;
	FORCE_INLINE int16 AllocBlockRTRect()
	{
		int16 n = m_freedBlockRTRects.back();
		m_freedBlockRTRects.pop_back();
		return n;
	}
	FORCE_INLINE void FreeBlockRTRect( int16 n )
	{
		m_freedBlockRTRects.push_back( n );
	}
	vector<int16> m_freedBlockRTRects;

	vector<SBlockLayer*> m_vecUpdatedBlocks;
	vector<SChunk*> m_vecUpdatedChunks;

	static CMyLevel* s_pLevel;

	static int8 s_nTypes[eBlockType_Count];
};