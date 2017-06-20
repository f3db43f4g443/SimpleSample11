#pragma once
#include "Entity.h"

class CItem : public CEntity
{
	friend void RegisterGameClasses();
public:
	CItem( const SClassCreateContext& context ) : CEntity( context ), m_strKey( context ), m_strUpgrade( context ) { SET_BASEOBJECT_ID( CItem ); }

	virtual void Add( CPlayer* pPlayer ) {}
	virtual void Remove( CPlayer* pPlayer ) {}

	const CString& GetKey() { return m_strKey; }
	const CString& GetUpgrade() { return m_strUpgrade; }
protected:
	CString m_strKey;
	CString m_strUpgrade;
private:
	LINK_LIST_REF( CItem, Item )
};

class CItemCommon : public CItem
{
	friend void RegisterGameClasses();
public:
	CItemCommon( const SClassCreateContext& context ) : CItem( context ) { SET_BASEOBJECT_ID( CItemCommon ); }

	virtual void Add( CPlayer* pPlayer );
	virtual void Remove( CPlayer* pPlayer );
private:
	int32 m_nHp;
	int32 m_nSp;
};