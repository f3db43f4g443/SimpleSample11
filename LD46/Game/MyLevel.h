#pragma once
#include "Common/StringUtil.h"
#include "Common/PriorityQueue.h"
#include "Render/DrawableGroup.h"
#include "BasicElems.h"

#define LEVEL_GRID_SIZE_X 24.0f
#define LEVEL_GRID_SIZE_Y 32.0f
#define LEVEL_GRID_SIZE CVector2( LEVEL_GRID_SIZE_X, LEVEL_GRID_SIZE_Y )

struct SLevelGridData
{
	SLevelGridData() { bBlocked = false; nNextStage = nSpawn = 0; }
	SLevelGridData( const SClassCreateContext& context ) {}
	bool bBlocked;
	int32 nNextStage;
	int32 nSpawn;
};

struct SLevelNextStageData
{
	SLevelNextStageData( const SClassCreateContext& context ) {}
	TResourceRef<CPrefab> pNxtStage;
	int32 nOfsX, nOfsY;
};

class CMyLevel : public CEntity
{
	friend void RegisterGameClasses();
	friend class CLevelEdit;
public:
	CMyLevel( const SClassCreateContext& context ) : CEntity( context ) { SET_BASEOBJECT_ID( CMyLevel ); }

	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;
	struct SGrid
	{
		SGrid() : bCanEnter( true ), nNextStage( 0 ) {}
		bool bCanEnter;
		int32 nNextStage;
		CReference<CPawn> pPawn;
	};
	TVector2<int32> GetSize() { return TVector2<int32>( m_nWidth, m_nHeight ); }
	SGrid* GetGrid( const TVector2<int32>& p ) { return p.x >= 0 && p.y >= 0 && p.x < m_nWidth && p.y < m_nHeight ?
		&m_vecGrid[p.x + p.y * m_nWidth] : NULL; }
	const CVector2& GetCamPos() { return m_camPos; }
	CPlayer* GetPlayer() { return m_pPlayer; }

	bool AddPawn( CPawn* pPawn, const TVector2<int32>& pos, int8 nDir, CPawn* pCreator = NULL, int32 nForm = 0 );
	void RemovePawn( CPawn* pPawn );
	bool PawnMoveTo( CPawn* pPawn, const TVector2<int32>& ofs );
	void PawnMoveEnd( CPawn* pPawn );
	void PawnMoveBreak( CPawn* pPawn );
	void PawnDeath( CPawn* pPawn );
	bool PawnTransform( CPawn* pPawn, int32 nForm, const TVector2<int32>& ofs );

	void Init();
	void Update();
private:
	int32 m_nWidth, m_nHeight;
	CReference<CEntity> m_pPawnRoot;
	CVector2 m_camPos;
	TArray<SLevelGridData> m_arrGridData;
	TArray<SLevelNextStageData> m_arrNextStage;
	TArray<TResourceRef<CPrefab> > m_arrSpawnPrefab;
	TResourceRef<CDrawableGroup> m_pTileDrawable;

	bool m_bComplete;
	bool m_bFailed;
	vector<SGrid> m_vecGrid;
	TClassTrigger<CMyLevel> m_onTick;
	CReference<CPlayer> m_pPlayer;
	vector<CReference<CPawn> > m_vecBarriers;
	CReference<CRenderObject2D> m_pTip;

	struct
	{
		LINK_LIST_REF_HEAD( m_pPawns, CPawn, Pawn );
	} m_spawningPawns;
	vector<CReference<CPawnHit> > m_vecPawnHits;
	LINK_LIST_REF_HEAD( m_pPawns, CPawn, Pawn );
};