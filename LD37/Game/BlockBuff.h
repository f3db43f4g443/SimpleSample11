#pragma once
#include "Block.h"

class CBlockBuff : public CEntity
{
	friend void RegisterGameClasses();
public:
	struct SContext
	{
		uint32 nLife;
		float fParams[4];
	};

	enum
	{
		eAddedReason_New,
		eAddedReason_Update,
	};
	enum
	{
		eRemovedReason_Default,
		eRemovedReason_Timeout,
		eRemovedReason_BlockDestroyed,
	};

	CBlockBuff( const SClassCreateContext& context ) : CEntity( context ), m_nLife( 0 ), m_bIsRemoving( false )
		, m_onParentRemoved( this, &CBlockBuff::OnParentRemoved )
		, m_onTick( this, &CBlockBuff::OnTick ) { SET_BASEOBJECT_ID( CBlockBuff ); }

	static CBlockBuff* AddBuff( CPrefab* pPrefab, CBlockObject* pBlock, SContext* pContext );

	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;
protected:
	virtual CBlockBuff* Add( CPrefab* pPrefab, CBlockObject* pBlock, SContext* pContext ) const;
	virtual void OnTick();
	
	virtual void OnAdded( uint8 nReason, SContext* pContext ) { m_nLife = pContext->nLife; }
	virtual void OnRemoved( uint8 nReason ) { m_bIsRemoving = true; }

	bool m_bMulti;
	
	uint32 m_nLife;
private:
	void OnParentRemoved() { OnRemoved( eRemovedReason_BlockDestroyed ); }
	bool m_bIsRemoving;

	TClassTrigger<CBlockBuff> m_onParentRemoved;
	TClassTrigger<CBlockBuff> m_onTick;
};