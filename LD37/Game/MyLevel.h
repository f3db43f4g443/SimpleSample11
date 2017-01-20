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
	CMyLevel( const SClassCreateContext& context ) : CEntity( context ), m_bPending( true ) { SET_BASEOBJECT_ID( CMyLevel ); }

	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;

	void Start() { if( m_bPending ) { m_bPending = false; m_pClickToStart->bVisible = false; } }

	void KillChunk( SChunk* pChunk, bool bCrush = false );

	void RemoveChunk( SChunk* pChunk );
	void SplitChunks( SChunk* pOldChunk, vector< pair<SChunk*, TVector2<int32> > > newChunks );

	uint32 GetBlockSize() { return m_nBlockSize; }
	float GetShakeStrength() { return m_basements[0].fShakeStrength; }
	void AddShakeStrength( float fShakeStrength );

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

	CRectangle GetBound() { return CRectangle( 0, 0, m_nWidth * m_nBlockSize, m_nSpawnHeight * m_nBlockSize ); }
	CEntity* GetChunkRoot() { return m_pChunkRoot; }
	CEntity* GetChunkEffectRoot() { return m_pChunkEffectRoot; }
	CEntity* GetBulletRoot( uint8 nLevel ) { return m_pBulletRoot[nLevel]; }

	void UpdateBlocksMovement();

	CReference<CSoundFile> pFireSound;
	CReference<CSoundFile> pHitSound;
	CReference<CSoundFile> pExpSound;

	CReference<CPrefab> pChunkUIPrefeb;
private:
	void CreateGrids( bool bNeedInit );
	void CacheNextLevel();
	uint32 m_nCurLevel;

	bool m_bPending;
	uint32 m_nWidth;
	uint32 m_nHeight;
	uint32 m_nSpawnHeight;

	struct SBasement
	{
		SBasement() { memset( this, 0, sizeof( SBasement ) ); }
		SBlock* pSpawnedBlock;
		float fShakeStrength;
		int32 nCurShakeStrength;
		int32 nShakeHeight;
		LINK_LIST_HEAD( pBlock, SBlock, Block )
	};
	vector<SBasement> m_basements;
	uint32 m_nBlockSize;
	float m_fFallDistPerSpeedFrame;

	void CheckSpawn();

	TClassTrigger<CMyLevel> m_onTick;

	CReference<CEntity> m_pChunkRoot;
	CReference<CEntity> m_pChunkEffectRoot;
	CReference<CRenderObject2D> m_pClickToStart;
	CReference<CEntity> m_pBulletRoot[eBulletLevel_Count];

	static CMyLevel* s_pLevel;

	static int8 s_nTypes[eBlockType_Count];
};