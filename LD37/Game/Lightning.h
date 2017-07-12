#pragma once
#include "Entity.h"
#include "Common/StringUtil.h"

class CLightning : public CEntity
{
	friend void RegisterGameClasses();
public:
	CLightning( const SClassCreateContext& context ) : CEntity( context ), m_nFrame( 0 ), m_onTick( this, &CLightning::OnTick ),
		m_onBeginRemoved( this, &CLightning::OnBeginRemoved ), m_onEndRemoved( this, &CLightning::OnEndRemoved ), m_nBeginTransIndex( -1 ), m_nEndTransIndex( -1 ),
		m_bSet( false ), m_bAutoRemove( false ), m_bIsBeamInited( false ), m_nHitCDLeft( 0 ) { SET_BASEOBJECT_ID( CLightning ); }

	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;

	virtual void OnHit( CEntity* pEntity ) { if( m_onHit ) m_onHit( this, pEntity ); }
	void SetOnHit( function<void( CLightning*, CEntity* )> onHit ) { m_onHit = onHit; }

	void Set( CEntity* pBegin, CEntity* pEnd, const CVector2& begin, const CVector2& end, int16 nBeginTransIndex, int16 nEndTransIndex );
	void SetCreator( CEntity* pCreator ) { m_pCreator = pCreator; }
	void SetAutoRemove( bool bAutoRemove ) { m_bAutoRemove = bAutoRemove; }
	void SetWidth( float fWidth, float fHitWidth ) { m_fWidth = fWidth; m_fHitWidth = fHitWidth; }
	void SetDamage( uint32 nDamage ) { m_nDamage = nDamage; }
	void SetDamage1( uint32 nDamage ) { m_nDamage1 = nDamage; }
	void SetDamage2( uint32 nDamage ) { m_nDamage2 = nDamage; }
	void SetLife( uint32 nLife ) { m_nLife = nLife; }
	void SetHitCD( int32 nCD ) { m_nHitCD = nCD; }
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
	int32 m_nHitCD;
	bool m_bBurst;
	int32 m_nLife;
	float m_fHitWidthPerFrame;
	float m_fBeginLen;
	float m_fEndLen;
	float m_fBeginTexLen;
	float m_fEndTexLen;
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
	int32 m_nHitCDLeft;
	bool m_bSet;
	bool m_bAutoRemove;
	bool m_bIsBeam;
	bool m_bIsBeamInited;

	uint32 m_nDamage;
	uint32 m_nDamage1;
	uint32 m_nDamage2;
	float m_fKnockback;
	TResourceRef<CPrefab> m_pDmgEft;

	function<void( CLightning*, CEntity* )> m_onHit;

	TClassTrigger<CLightning> m_onTick;

	TClassTrigger<CLightning> m_onBeginRemoved;
	TClassTrigger<CLightning> m_onEndRemoved;
};