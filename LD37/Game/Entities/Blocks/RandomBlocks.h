#pragma once
#include "Block.h"

class CRandomChunk0 : public CChunkObject
{
	friend void RegisterGameClasses();
public:
	CRandomChunk0( const SClassCreateContext& context ) : CChunkObject( context ) { SET_BASEOBJECT_ID( CRandomChunk0 ); }
	virtual void OnSetChunk( SChunk* pChunk, class CMyLevel* pLevel ) override;
private:
	uint32 m_nWidth, m_nHeight;
};

class CRandomChunkTiledSimple : public CChunkObject
{
	friend void RegisterGameClasses();
public:
	CRandomChunkTiledSimple( const SClassCreateContext& context ) : CChunkObject( context ) { SET_BASEOBJECT_ID( CRandomChunkTiledSimple ); }
	virtual void OnSetChunk( SChunk* pChunk, class CMyLevel* pLevel ) override;
protected:
	virtual void OnKilled() override;
private:
	uint32 m_nHpPerSize;

	CRectangle m_texRect1;
	CRectangle m_texRect2;
	CRectangle m_texRect3;
	CRectangle m_texRect4;
};

class CRandomChunkTiled : public CChunkObject
{
	friend void RegisterGameClasses();
public:
	CRandomChunkTiled( const SClassCreateContext& context ) : CChunkObject( context ) { SET_BASEOBJECT_ID( CRandomChunkTiled ); }
	virtual void OnSetChunk( SChunk* pChunk, class CMyLevel* pLevel ) override;
protected:
	virtual void OnKilled() override;
private:
	uint32 m_nHpPerSize;
	uint32 m_nTypeX;
	uint32 m_nTypeY;
	uint8 m_nGroupType;
	bool m_bBlockTypeMask[eBlockType_Count];
};

class CRandomChunkTiled1 : public CChunkObject
{
	friend void RegisterGameClasses();
public:
	CRandomChunkTiled1( const SClassCreateContext& context ) : CChunkObject( context ) { SET_BASEOBJECT_ID( CRandomChunkTiled1 ); }
	virtual void OnSetChunk( SChunk* pChunk, class CMyLevel* pLevel ) override;
protected:
	virtual void OnKilled() override;
private:
	uint32 m_nHpPerSize;
	uint32 m_sizeX[eBlockType_Count];
	uint32 m_sizeY[eBlockType_Count];
	CRectangle m_texRects[eBlockType_Count];
};

class CRandomChunk1 : public CChunkObject
{
	friend void RegisterGameClasses();
public:
	CRandomChunk1( const SClassCreateContext& context ) : CChunkObject( context ) { SET_BASEOBJECT_ID( CRandomChunk1 ); }

	virtual void OnSetChunk( SChunk* pChunk, class CMyLevel* pLevel ) override;
protected:
	virtual void OnKilled() override;
private:
	uint32 m_nHpPerSize;
	uint32 m_nAltX, m_nAltY;
};

class CRandomChunk2 : public CChunkObject
{
	friend void RegisterGameClasses();
public:
	CRandomChunk2( const SClassCreateContext& context ) : CChunkObject( context ) { SET_BASEOBJECT_ID( CRandomChunk2 ); }

	virtual void OnSetChunk( SChunk* pChunk, class CMyLevel* pLevel ) override;
protected:
	virtual void OnKilled() override;
private:
	uint32 m_nHpPerLength;
	
	bool m_bVertical;
	uint32 m_nWidth;
	CRectangle m_texRect1;
	uint32 m_nTexRect1X, m_nTexRect1Y;
	CRectangle m_texRect1End;
	uint32 m_nTexRect1EndX, m_nTexRect1EndY;
	CRectangle m_texRect2;
	uint32 m_nTexRect2X, m_nTexRect2Y;
	CRectangle m_texRect2End;
	uint32 m_nTexRect2EndX, m_nTexRect2EndY;
};

class CRandomChunk3 : public CChunkObject
{
	friend void RegisterGameClasses();
public:
	CRandomChunk3( const SClassCreateContext& context ) : CChunkObject( context ) { SET_BASEOBJECT_ID( CRandomChunk3 ); }

	virtual void OnSetChunk( SChunk* pChunk, class CMyLevel* pLevel ) override;
protected:
	virtual void OnKilled() override;
private:
	uint32 m_nHpPerSize;
	float m_fXCoef, m_fYCoef;
	bool m_bBlockTypeMask[eBlockType_Count];
};

class CRandomChunk4 : public CChunkObject
{
	friend void RegisterGameClasses();
public:
	CRandomChunk4( const SClassCreateContext& context ) : CChunkObject( context ) { SET_BASEOBJECT_ID( CRandomChunk4 ); }

	virtual void OnSetChunk( SChunk* pChunk, class CMyLevel* pLevel ) override;
protected:
	virtual void OnKilled() override;
private:
	uint32 m_nHpPerSize;
	uint32 m_nTexWidth, m_nTexHeight, m_nLeft, m_nRight, m_nTop, m_nBottom;
};

class CDefaultRandomRoom : public CChunkObject
{
	friend void RegisterGameClasses();
public:
	CDefaultRandomRoom( const SClassCreateContext& context ) : CChunkObject( context ) { SET_BASEOBJECT_ID( CDefaultRandomRoom ); }

	virtual void OnSetChunk( SChunk* pChunk, class CMyLevel* pLevel ) override;
protected:
	virtual void OnKilled() override;
private:
	uint32 m_nHpPerSize;
};

class CRandomRoom1 : public CChunkObject
{
	friend void RegisterGameClasses();
public:
	CRandomRoom1( const SClassCreateContext& context ) : CChunkObject( context ) { SET_BASEOBJECT_ID( CRandomRoom1 ); }

	virtual void OnSetChunk( SChunk* pChunk, class CMyLevel* pLevel ) override;
protected:
	virtual void OnKilled() override;
private:
	uint32 m_nHpPerSize;
};