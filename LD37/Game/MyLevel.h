#pragma once
#include "Character.h"
#include "Common/StringUtil.h"
#include "Render/DrawableGroup.h"
#include "Block.h"
#include "Render/Sound.h"

class CMyLevel : public CEntity
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

	void KillChunk( SChunk* pChunk, bool bCrush = false );

	void RemoveChunk( SChunk* pChunk );
	void SplitChunks( SChunk* pOldChunk, vector< pair<SChunk*, TVector2<int32> > > newChunks );

	static uint32 GetBlockSize() { return 32; }
	float GetFallDistPerSpeedFrame() { return m_fFallDistPerSpeedFrame; }
	float GetShakeStrength() { return m_basements[0].fShakeStrength; }
	void AddShakeStrength( float fShakeStrength );

	float GetLastScrollPos() { return m_fLastScrollPos; }
	float GetCurScrollPos() { return m_fCurScrollPos; }
	uint32 GetCurHeightTag() { return m_vecBarrierHeightTags.size() ? m_vecBarrierHeightTags.back() : 0; }
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

	static CMyLevel* GetInst() { return s_pLevel; }
	
	bool IsLevelDesignTest() { return m_bIsLevelDesignTest; }
	CRectangle GetBound() { return CRectangle( 0, 0, m_nWidth * GetBlockSize(), m_nSpawnHeight * GetBlockSize() ); }
	CRectangle GetBoundWithLvBarrier() { return CRectangle( 0, 0, m_nWidth * GetBlockSize(), Min<float>( m_fCurLvBarrierHeight, m_nSpawnHeight * GetBlockSize() ) ); }
	CRectangle GetLargeBound() { auto bound = GetBound(); return CRectangle( bound.x - 1024, bound.y - 1024, bound.width + 2048, bound.height + 2048 ); }
	float GetHighGravityHeight() { return 256.0f; }
	CEntity* GetScrollObjRoot( uint32 i ) { return m_pScrollObjRoot[i]; }
	CEntity* GetChunkRoot() { return m_pChunkRoot; }
	CEntity* GetChunkRoot1() { return m_pChunkRoot1; }
	CEntity* GetChunkEffectRoot() { return m_pChunkEffectRoot; }
	CEntity* GetBulletRoot( uint8 nLevel ) { return m_pBulletRoot[nLevel]; }
	CRenderObject2D* GetBack0() { return m_pBack0; }
	CRenderObject2D* GetCrosshair() { return m_pCrosshair; }
	void UpdateBack0Position( const CVector2& pos );
	virtual CVector2 GetCamPos();

	void UpdateBlocksMovement();

	CReference<CPrefab> pChunkUIPrefeb;
protected:
	void CreateGrids( const char* szNode );
	void CacheNextLevel();
	uint32 m_nCurLevel;

	bool m_bIsLevelDesignTest;
	uint32 m_nWidth;
	uint32 m_nHeight;
	uint32 m_nSpawnHeight;
	float m_fCurLvBarrierHeight;
	float m_fLastScrollPos;
	float m_fCurScrollPos;
	vector<int32> m_vecBarrierHeightTags;
	uint32 m_nCurBarrierHeightTag;

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

		float fShakeStrength;
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

	static CMyLevel* s_pLevel;

	static int8 s_nTypes[eBlockType_Count];
};