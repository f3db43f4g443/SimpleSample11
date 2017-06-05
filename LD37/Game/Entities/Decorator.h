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

class CDecoratorDirt : public CDecorator
{
	friend void RegisterGameClasses();
public:
	CDecoratorDirt( const SClassCreateContext& context ) : CDecorator( context ) { SET_BASEOBJECT_ID( CDecoratorDirt ); }

	virtual void Init( const CVector2& size ) override;
private:
	CVector2 m_texSize;
	float m_fTexelSize;
	uint32 m_nMaxTexelSize;
	uint32 m_nMinTexelSize;
	float m_fPercent;
	uint32 m_nMaskCols;
	uint32 m_nMaskRows;
};

class CDecoratorTile : public CDecorator
{
	friend void RegisterGameClasses();
public:
	CDecoratorTile( const SClassCreateContext& context ) : CDecorator( context ) { SET_BASEOBJECT_ID( CDecoratorTile ); }

	virtual void Init( const CVector2& size ) override;
private:
	void AddTile( TVector2<int32> pos, uint8 nType );
	uint32 m_nTexCols;
	uint32 m_nTexRows;
	uint32 m_nTileSize;
	uint32 m_nTileBegin[9];
	uint32 m_nTileCount[9];
};