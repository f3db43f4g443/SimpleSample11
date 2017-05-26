#pragma once
#include "Item.h"

class CPlayerWeapon : public CItem
{
	friend void RegisterGameClasses();
public:
	CPlayerWeapon( const SClassCreateContext& context ) : CItem( context ), m_bIsFiring( false ) { SET_BASEOBJECT_ID( CPlayerWeapon ); }

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

	virtual void Add( CPlayer* pPlayer ) override;
	virtual void Remove( CPlayer* pPlayer ) override;

	virtual void Equip( CPlayer* pPlayer ) {}
	virtual void BeginFire( CPlayer* pPlayer ) {}
	virtual void EndFire( CPlayer* pPlayer ) {}
	virtual void Update( CPlayer* pPlayer ) {}
	virtual void UnEquip( CPlayer* pPlayer ) {}
protected:
	CRectangle m_texRectFaceRight;
	CRectangle m_texRectFaceLeft;
	bool m_bIsFiring;
};