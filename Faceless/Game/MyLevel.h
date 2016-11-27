#pragma once
#include "Character.h"
#include "Common/StringUtil.h"
#include "Common/PriorityQueue.h"
#include "Entities/AIObject.h"
#include "Render/DrawableGroup.h"

class CTurnBasedContext : public CAIObject
{
public:
	struct SOrganActionContext* pActionContext;
protected:
	virtual void AIFunc() override;
};

class CMyLevel : public CEntity
{
	friend class CTurnBasedContext;
	friend void RegisterGameClasses();
public:
	CMyLevel( const SClassCreateContext& context ) : CEntity( context ) { SET_BASEOBJECT_ID( CMyLevel ); }

	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;

	bool AddCharacter( CCharacter* pCharacter, uint32 x, uint32 y );
	bool RemoveCharacter( CCharacter* pCharacter );

	bool MoveCharacter( CCharacter* pCharacter, uint32 x, uint32 y );
	void OnCharacterDelayChanged( CCharacter* pCharacter ) { m_characters.Modify( pCharacter ); }

	enum
	{
		eGridFlag_BlockMove = 1,
		eGridFlag_BlockSight = 2,
	};

	const CVector2& GetBaseOffset() { return m_baseOffset; }
	const CVector2& GetGridScale() { return m_gridScale; }
	struct SGrid
	{
		SGrid() : nGridFlag( 0 ), pCharacter( NULL ) {}
		uint8 nGridFlag;
		CCharacter* pCharacter;
	};
	SGrid* GetGrid( uint32 x, uint32 y ) { return x < m_nWidth && y < m_nHeight ? &m_grids[x + y * m_nWidth] : NULL; }

	void RayCast( TVector2<int32> src, TVector2<int32> target, function<bool( const TVector2<int32>& )> func );

	CTurnBasedContext* GetTurnBasedContext() { return m_pTurnBasedContext; }

	enum
	{
		eSelectTile_TargetInvalid,
		eSelectTile_TargetValid,

		eSelectTile_InRange,

		eSelectTile_InEffectRange,
	};

	CTileMap2D* GetSelectTile() { return m_pWorldSelectTile; }

	static CMyLevel* GetInst() { return s_pLevel; }
private:
	void CreateGrids();
	void Turn();

	uint32 m_nWidth;
	uint32 m_nHeight;
	vector<SGrid> m_grids;
	CVector2 m_baseOffset;
	CVector2 m_gridScale;

	CReference<CTileMap2D> m_pWorldSelectTile;

	uint16 m_nCurCharacterID;
	TPriorityQueue<CCharacter> m_characters;
	CReference<CTurnBasedContext> m_pTurnBasedContext;
	TClassTrigger<CMyLevel> m_onTick;

	static CMyLevel* s_pLevel;
};