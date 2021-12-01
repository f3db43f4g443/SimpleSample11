#pragma once
#include "Math3D.h"

class CCamera2D
{
public:
	CCamera2D( void ) : m_fRotation( 0 ), m_nPriority( 0 ), bEnabled( true ), m_extraOfs( 0, 0 ), nMask( INVALID_32BITID )
	{SetSize(1280, 800); SetPosition(0, 0); SetViewport(0, 0, 0, 0);}
	CCamera2D(float width, float height) : m_fRotation( 0 ), m_nPriority( 0 ), bEnabled( true ), m_extraOfs( 0, 0 )
	{SetSize(width, height); SetPosition(0, 0); SetViewport(0, 0, 0, 0);}

	void SetPosition(float centerX, float centerY) { m_rectScene.SetCenterX( centerX ); m_rectScene.SetCenterY( centerY ); }
	void SetViewport(float x, float y, float w, float h) { m_rectViewport = CRectangle( x, y, w, h ); }
	void SetViewport( const CRectangle& rect ) { m_rectViewport = rect; }
	void SetViewArea( const CRectangle& rect ) { m_rectScene = rect; }
	void SetSize(float width, float height) { m_rectScene.SetSizeX( width ); m_rectScene.SetSizeY( height ); }
	void SetExtraOfs( const CVector2& ofs ) { m_extraOfs = ofs; }

	CRectangle GetViewArea() const { return m_rectScene.Offset( m_extraOfs ); }
	CRectangle GetOrigViewArea() const { return m_rectScene; }
	CRectangle GetViewport() const { return m_rectViewport; }
	void GetDrawMatrix(CMatrix& trans, unsigned int nElemCount) const;

	CVector2 ScrToView(CVector2 scr) const;

	int GetPriority() const { return m_nPriority; }
	void SetPriority( int nValue ) { m_nPriority = nValue; }
	float GetRotation() const { return m_fRotation; }
	void SetRotation( float r ) { m_fRotation = r; }

	bool bEnabled;
	uint32 nMask;
private:
	int m_nPriority;
	CRectangle m_rectScene;
	CRectangle m_rectViewport;
	float m_fRotation;
	CVector2 m_extraOfs;
};
