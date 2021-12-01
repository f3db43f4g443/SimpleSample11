#include "Common.h"
#include "Camera2D.h"

void CCamera2D::GetDrawMatrix(CMatrix& trans, unsigned int nElemCount) const
{
	CMatrix2D mat0;
	mat0.Translate( -( m_rectScene.GetCenterX() + m_extraOfs.x ), -( m_rectScene.GetCenterY() + m_extraOfs.y ) );
	CMatrix2D mat1;
	mat1.Rotate( -m_fRotation );
	mat0 = mat1 * mat0;
	CMatrix m( mat0.m00, mat0.m01, 0, mat0.m02,
		mat0.m10, mat0.m11, 0, mat0.m12,
		0, 0, 1, 0,
		0, 0, 0, 1 );
	trans.Identity();
	trans.m00 = 2.0f / m_rectScene.width;
	trans.m11 = 2.0f / m_rectScene.height;
	trans.m22 = 1.0f / nElemCount;
	trans.m03 = 0;
	trans.m13 = 0;
	trans.m23 = 0.5f / nElemCount;
	trans = trans * m;
}

CVector2 CCamera2D::ScrToView(CVector2 scr) const
{
	CVector2 vec;
	vec.x = ( scr.x - m_rectViewport.x ) / m_rectViewport.width * m_rectScene.width + m_rectScene.x;
	vec.y = ( m_rectViewport.height - scr.y + m_rectViewport.y ) / m_rectViewport.height * m_rectScene.height + m_rectScene.y;
	return vec;
}
