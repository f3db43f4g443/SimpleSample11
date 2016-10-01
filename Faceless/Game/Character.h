#pragma once
#include "Entity.h"
#include "Trigger.h"
#include "Common/PriorityQueue.h"
#include "Common/StringUtil.h"
#include "FaceEditItem.h"

class CTurnBasedContext;
class CCharacter : public CEntity, public TPriorityQueueNode<CCharacter>
{
	friend class CMyLevel;
	friend void RegisterGameClasses();
public:
	CCharacter();
	CCharacter( const SClassCreateContext& context );

	virtual uint32 GetPriority() override { return m_nPriority; }

	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;

	uint8 ShowSubStage( uint8 nSlot );
	void HideSubStage();
	struct SSubStage* GetSubStage();
	uint8 GetSubStageShowSlot() { return m_nSubStageShowSlot; }

	const char* GetSubStageName() { return m_strSubStageName.c_str(); }

	CMyLevel* GetLevel() { return m_pLevel; }
	const TVector2<int32>& GetGrid() { return m_grid; }
	uint8 GetDir() { return m_nDir; }
	bool MoveTo( uint32 gridX, uint32 gridY );
	void Face( uint8 nDir );
	uint16 GetDelay() { return m_nDelay; }
	void SetDelay( uint16 nDelay );

	const vector<CFaceEditItem*>& GetFaceEditItems() { return m_vecFaceEditItems; }
	void AddFaceEditItem( CFaceEditItem* pItem ) { m_vecFaceEditItems.push_back( pItem ); }

	uint32 GetHp() { return m_nHp; }
	uint32 GetMp() { return m_nMp; }
	uint32 GetSp() { return m_nSp; }
	void SetHp( uint32 n ) { SetHp( n, m_nMaxHp ); }
	void SetMp( uint32 n ) { SetMp( n, m_nMaxMp ); }
	void SetSp( uint32 n ) { SetSp( n, m_nMaxSp ); }
	void SetHp( uint32 n, uint32 nMax );
	void SetMp( uint32 n, uint32 nMax );
	void SetSp( uint32 n, uint32 nMax );

	static const TVector2<int32>& GetDirOfs( uint8 nDir )
	{
		static TVector2<int32> ofs[4] = { { -1, 0 }, { 0, -1 }, { 1, 0 }, { 0, 1 } };
		return ofs[nDir];
	}
	static TVector2<int32> RotateDir( const TVector2<int32>& dir, uint8 nCharDir )
	{
		TVector2<int32> dirs[4] = { { -dir.y, dir.x }, { dir.x, dir.y }, { dir.y, -dir.x }, { -dir.x, -dir.y } };
		return dirs[nCharDir & 3];
	}

	virtual void OnTurn( CTurnBasedContext* pContext );

	virtual void MovePhase( CTurnBasedContext* pContext );
	virtual void EmotePhase( CTurnBasedContext* pContext );
	virtual void BattlePhase( CTurnBasedContext* pContext );

	virtual bool SelectTargetLevelGrid( CTurnBasedContext* pContext, struct SOrganActionContext& actionContext );
	virtual TVector2<int32> SelectTargetFaceGrid( CTurnBasedContext* pContext, struct SOrganActionContext& actionContext );

	bool Move( CTurnBasedContext* pContext, uint8 nDir );

	bool UseFaceEditItem( CTurnBasedContext* pContext, CFaceEditItem* pItem, const TVector2<int32>& pos );
protected:
	virtual void OnTick();
private:
	TClassTrigger<CCharacter> m_tickBeforeHitTest;

	CMyLevel* m_pLevel;
	TVector2<int32> m_grid;
	uint8 m_nDir;

	CString m_strSubStageName;

	vector<CFaceEditItem*> m_vecFaceEditItems;

	union
	{
		struct
		{
			uint16 m_nCharacterStageID;
			uint16 m_nDelay;
		};
		uint32 m_nPriority;
	};

	uint32 m_nMaxHp, m_nHp;
	uint32 m_nMaxMp, m_nMp;
	uint32 m_nMaxSp, m_nSp;

	uint32 m_nSubStage;
	uint8 m_nSubStageShowSlot;
};