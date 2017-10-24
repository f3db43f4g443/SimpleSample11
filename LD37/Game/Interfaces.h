#pragma once

class IOperateable
{
public:
	virtual int8 IsOperateable( const CVector2& pos ) = 0;
	virtual void Operate( const CVector2& pos ) = 0;
};