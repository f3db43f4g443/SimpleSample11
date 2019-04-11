#pragma once

class IOperateable
{
public:
	virtual int8 IsOperateable( const CVector2& pos ) = 0;
	virtual void Operate( const CVector2& pos ) = 0;
};

class IHook
{
public:
	virtual void OnDetach() = 0;
};

class IAttachable
{
public:
	virtual void OnSlotDetach( class CEntity* pTarget ) = 0;
};

class IAttachableSlot
{
public:
	virtual bool CanAttach( class CEntity* pOwner, class CEntity* pTarget ) = 0;
	virtual void Attach( class CEntity* pOwner, class CEntity* pTarget ) = 0;
	virtual void OnEntityDetach() = 0;
};