#pragma once
#include "Entity.h"
#include "Common/StringUtil.h"

class CLightning : public CEntity
{
	friend void RegisterGameClasses();
public:
	CLightning( const SClassCreateContext& context ) : CEntity( context ), m_nFrame( 0 ), m_onTick( this, &CLightning::OnTick ),
		m_onBeginRemoved( this, &CLightning::OnBeginRemoved ), m_onEndRemoved( this, &CLightning::OnEndRemoved ), m_nBeginTransIndex( -1 ), m_nEndTransIndex( -1 ),
		m_bSet( false ), m_bAutoRemove( false ), m_bIsBeamInited( false ) { SET_BASEOBJECT_ID( CLightning ); }

	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;

	virtual void OnHit( CEntity* pEntity ) { if( m_onHit ) m_onHit( this, pEntity ); }
	void SetOnHit( function<void( CLightning*, CEntity* )> onHit ) { m_onHit = onHit; }

	void Set( CEntity* pBegin, CEntity* pEnd, const CVector2& begin, const CVector2& end, int16 nBeginTransIndex, int16 nEndTransIndex );
	void SetCreator( CEntity* pCreator ) { m_pCreator = pCreator; }
	void SetAutoRemove( bool bAutoRemove ) { m_bAutoRemove = bAutoRemove; }
protected:
	void OnTick();
	void UpdateRenderObject();

	void OnBeginRemoved();
	void OnEndRemoved();

	CReference<CEntity> m_pCreator;

	uint8 m_nType;
	float m_fWidth;
	float m_fHitWidth;
	float m_fTexYTileLen;
	int32 m_nHitFrameBegin;
	int32 m_nHitFrameCount;
	float m_fHitWidthPerFrame;
	CReference<CRenderObject2D> m_pBeginEft;
	CReference<CRenderObject2D> m_pEndEft;

	CReference<CEntity> m_pBegin;
	CReference<CEntity> m_pEnd;
	CVector2 m_begin;
	CVector2 m_end;
	CVector2 m_beamEnd;
	int32 m_nFrame;
	int16 m_nBeginTransIndex;
	int16 m_nEndTransIndex;
	bool m_bSet;
	bool m_bAutoRemove;
	bool m_bIsBeam;
	bool m_bIsBeamInited;

	uint32 m_nDamage;
	uint32 m_nDamage1;
	uint32 m_nDamage2;
	float m_fKnockback;

	function<void( CLightning*, CEntity* )> m_onHit;

	TClassTrigger<CLightning> m_onTick;

	TClassTrigger<CLightning> m_onBeginRemoved;
	TClassTrigger<CLightning> m_onEndRemoved;
};