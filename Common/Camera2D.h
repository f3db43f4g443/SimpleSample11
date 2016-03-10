#pragma once
#include "Math3D.h"

class CCamera2D
{
public:
	CCamera2D( void ) : m_nPriority( 0 ), bEnabled( true ), nMask( INVALID_32BITID )
	{SetSize(1280, 800); SetPosition(0, 0); SetViewport(0, 0, 1280, 800);}
	CCamera2D(float width, float height) : m_nPriority( 0 ), bEnabled( true )
	{SetSize(width, height); SetPosition(0, 0); SetViewport(0, 0, 1280, 800);}

	void SetPosition(float centerX, float centerY) { m_rectScene.SetCenterX( centerX ); m_rectScene.SetCenterY( centerY ); }
	void SetViewport(float x, float y, float w, float h) { m_rectViewport = CRectangle( x, y, w, h ); }
	void SetViewport( const CRectangle& rect ) { m_rectViewport = rect; }
	void SetViewArea( const CRectangle& rect ) { m_rectScene = rect; }
	void SetSize(float width, float height) { m_rectScene.SetSizeX( width ); m_rectScene.SetSizeY( height ); }

	const CRectangle& GetViewArea() const { return m_rectScene; }
	const CRectangle& GetViewport() const { return m_rectViewport; }
	void GetDrawMatrix(CMatrix& trans, unsigned int nElemCount) const;

	CVector2 ScrToView(CVector2 scr) const;

	int GetPriority() const { return m_nPriority; }
	void SetPriority( int nValue ) { m_nPriority = nValue; }

	bool bEnabled;
	uint32 nMask;
private:
	int m_nPriority;
	CRectangle m_rectScene;
	CRectangle m_rectViewport;
};
