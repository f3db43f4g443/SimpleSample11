#pragma once
#include "Entity.h"

class CDecorator : public CEntity
{
public:
	CDecorator( const SClassCreateContext& context ) : CEntity( context ) { SET_BASEOBJECT_ID( CDecorator ); }

	virtual void Init( const CVector2& size ) {}
};

class CDecoratorFiber : public CDecorator
{
	friend void RegisterGameClasses();
public:
	CDecoratorFiber( const SClassCreateContext& context ) : CDecorator( context ) { SET_BASEOBJECT_ID( CDecoratorFiber ); }

	virtual void Init( const CVector2& size ) override;
private:
	CVector2 m_texSize;
	float m_fTexelSize;
	float m_fWidth;
	bool m_bVertical;
	uint32 m_nAlignment;
	float m_fMaxHeightPercent;
	float m_fMinHeightPercent;
};