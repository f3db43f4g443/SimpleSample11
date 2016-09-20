#pragma once
#include "Entity.h"

class CBullet : public CEntity
{
	friend void RegisterGameClasses();
public:
	CBullet( const SClassCreateContext& context ) : CEntity( context ) {}

};