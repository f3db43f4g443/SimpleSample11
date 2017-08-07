#pragma once
#include "Entity.h"
#include "Render/DrawableGroup.h"

class CPlayer;
class CItem : public CEntity
{
	friend void RegisterGameClasses();
public:
	CItem( const SClassCreateContext& context ) : CEntity( context ) { SET_BASEOBJECT_ID( CItem ); }

	virtual void Add( CPlayer* pPlayer ) {}
	virtual void Remove( CPlayer* pPlayer ) {}

	const CString& GetKey() { return m_strKey; }
	CPrefab* GetUpgrade() { return m_strUpgrade; }
protected:
	CString m_strKey;
	TResourceRef<CPrefab> m_strUpgrade;
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

class CConsumable : public CEntity
{
	friend void RegisterGameClasses();
public:
	CConsumable( const SClassCreateContext& context ) : CEntity( context ) { SET_BASEOBJECT_ID( CConsumable ); }
	CDrawableGroup* GetIcon() { return m_pIcon; }
	virtual bool Use( CPlayer* pPlayer ) { return true; }
private:
	TResourceRef<CDrawableGroup> m_pIcon;
};