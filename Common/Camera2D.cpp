#include "Common.h"
#include "Camera2D.h"

void CCamera2D::GetDrawMatrix(CMatrix& trans, unsigned int nElemCount) const
{
	trans.Identity();
	trans.m00 = 2.0f / m_rectScene.width;
	trans.m11 = 2.0f / m_rectScene.height;
	trans.m22 = 1.0f / nElemCount;
	trans.m03 = -m_rectScene.GetCenterX() * trans.m00;
	trans.m13 = -m_rectScene.GetCenterY() * trans.m11;
	trans.m23 = 0.5f / nElemCount;
}

CVector2 CCamera2D::ScrToView(CVector2 scr) const
{
	CVector2 vec;
	vec.x = ( scr.x - m_rectViewport.x ) / m_rectViewport.width * m_rectScene.width + m_rectScene.x;
	vec.y = ( m_rectViewport.height - scr.y + m_rectViewport.y ) / m_rectViewport.height * m_rectScene.height + m_rectScene.y;
	return vec;
}
