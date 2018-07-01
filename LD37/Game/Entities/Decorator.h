#pragma once
#include "Entity.h"

class CDecorator : public CEntity
{
public:
	CDecorator( const SClassCreateContext& context ) : CEntity( context ) { SET_BASEOBJECT_ID( CDecorator ); }

	virtual void Init( const CVector2& size ) {}
};

class CDecoratorRandomTex : public CDecorator
{
	friend void RegisterGameClasses();
public:
	CDecoratorRandomTex( const SClassCreateContext& context ) : CDecorator( context ) { SET_BASEOBJECT_ID( CDecoratorRandomTex ); }

	virtual void Init( const CVector2& size ) override;
private:
	CVector2 m_texSize;
	float m_fTexelSize;
};

class CDecorator9Patch : public CDecorator
{
	friend void RegisterGameClasses();
public:
	CDecorator9Patch( const SClassCreateContext& context ) : CDecorator( context ) { SET_BASEOBJECT_ID( CDecorator9Patch ); }

	virtual void Init( const CVector2& size ) override;
private:
	float m_fX1, m_fX2;
	float m_fY1, m_fY2;
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
	void AddTile( TVector2<int32> pos, uint8 nType, class CChunkObject* pChunkObject );
	uint32 m_nTexCols;
	uint32 m_nTexRows;
	uint32 m_nTileSize;
	uint32 m_nTileBegin[9];
	uint32 m_nTileCount[9];
	uint8 m_nBlockTag;
};

class CDecoratorTile1 : public CDecorator
{
	friend void RegisterGameClasses();
public:
	CDecoratorTile1( const SClassCreateContext& context ) : CDecorator( context ) { SET_BASEOBJECT_ID( CDecoratorTile1 ); }

	virtual void Init( const CVector2& size ) override;
private:
	uint32 m_nTexCols;
	uint32 m_nTexRows;
	uint32 m_nTileBegin[16];
	uint32 m_nTileCount[16];
	uint8 m_nBlockTag;
};