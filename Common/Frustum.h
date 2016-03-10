#pragma once

#include "Vector3.h"
template <typename T>
class TFrustum
{
public:
	TFrustum(void);
	TFrustum(const TVector3<T>& eye, const TVector3<T>& look, const TVector3<T>& up, T dx, T dy, T zNear, T zFar);
	~TFrustum(void);

	void Set(const TVector3<T>& eye, const TVector3<T>& look, const TVector3<T>& up, T dx, T dy, T zNear, T zFar);

	int TestAABB(T x1, T y1, T z1, T x2, T y2, T z2);
private:
	TVector3<T> m_eye;
	TVector3<T> m_dir;
	TVector3<T> m_up;
	T m_dx, m_dy, m_zNear, m_zFar;

	TVector3<T> m_AABBAxes[26];
	T m_AABBAxesMin[26];
	T m_AABBAxesMax[26];
	int m_nAxes;
};

