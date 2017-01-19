#pragma once
#include "Entity.h"

class CPlayerWeapon : public CEntity
{
	friend void RegisterGameClasses();
public:
	CPlayerWeapon( const SClassCreateContext& context ) : CEntity( context ), m_bIsFiring( false ) { SET_BASEOBJECT_ID( CPlayerWeapon ); }

	void Face( bool bLeft );

	bool IsFiring() { return m_bIsFiring; }
	void SetFiring( bool bFiring, CPlayer* pPlayer )
	{
		if( m_bIsFiring != bFiring )
		{
			m_bIsFiring = bFiring;
			if( m_bIsFiring )
				BeginFire( pPlayer );
			else
				EndFire( pPlayer );
		}
	}

	virtual void Add( CPlayer* pPlayer ) {}
	virtual void BeginFire( CPlayer* pPlayer ) {}
	virtual void EndFire( CPlayer* pPlayer ) {}
	virtual void Update( CPlayer* pPlayer ) {}
	virtual void Remove( CPlayer* pPlayer ) {}
protected:
	CRectangle m_texRectFaceRight;
	CRectangle m_texRectFaceLeft;
	bool m_bIsFiring;
};