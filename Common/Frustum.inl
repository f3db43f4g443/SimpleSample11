#include "Common.h"

template <typename T>
FORCE_INLINE TFrustum<T>::TFrustum(void)
{
}

template <typename T>
FORCE_INLINE TFrustum<T>::TFrustum(const TVector3<T>& eye, const TVector3<T>& look, const TVector3<T>& up, T dx, T dy, T zNear, T zFar)
{
	Set(eye, look, up, dx, dy, zNear, zFar);
}

template <typename T>
FORCE_INLINE TFrustum<T>::~TFrustum(void)
{
}

template <typename T>
void TFrustum<T>::Set(const TVector3<T>& eye, const TVector3<T>& look, const TVector3<T>& up, T dx, T dy, T zNear, T zFar)
{
	m_eye = eye;
	m_dir = look - eye;
	m_dir.Normalize();
	m_up = up;
	m_up.Normalize();
	m_dx = dx;
	m_dy = dy;
	m_zNear = zNear;
	m_zFar = zFar;

	TVector3<T> right = m_dir.Cross(m_up);

	m_nAxes = 0;
	m_AABBAxes[m_nAxes++] = TVector3<T>(1, 0, 0);
	m_AABBAxes[m_nAxes++] = TVector3<T>(0, 1, 0);
	m_AABBAxes[m_nAxes++] = TVector3<T>(0, 0, 1);

	m_AABBAxes[m_nAxes++] = m_dir;
	m_AABBAxes[m_nAxes] = right + m_dir * m_dx;
	m_AABBAxes[m_nAxes++].Normalize();
	m_AABBAxes[m_nAxes] = right - m_dir * m_dx;
	m_AABBAxes[m_nAxes++].Normalize();
	m_AABBAxes[m_nAxes] = m_up + m_dir * m_dy;
	m_AABBAxes[m_nAxes++].Normalize();
	m_AABBAxes[m_nAxes] = m_up - m_dir * m_dy;
	m_AABBAxes[m_nAxes++].Normalize();

	m_AABBAxes[m_nAxes] = right.Cross(TVector3<T>(1, 0, 0));
	if(m_AABBAxes[m_nAxes].Normalize() > 0) m_nAxes++;
	m_AABBAxes[m_nAxes] = right.Cross(TVector3<T>(0, 1, 0));
	if(m_AABBAxes[m_nAxes].Normalize() > 0) m_nAxes++;
	m_AABBAxes[m_nAxes] = right.Cross(TVector3<T>(0, 0, 1));
	if(m_AABBAxes[m_nAxes].Normalize() > 0) m_nAxes++;
	m_AABBAxes[m_nAxes] = m_up.Cross(TVector3<T>(1, 0, 0));
	if(m_AABBAxes[m_nAxes].Normalize() > 0) m_nAxes++;
	m_AABBAxes[m_nAxes] = m_up.Cross(TVector3<T>(0, 1, 0));
	if(m_AABBAxes[m_nAxes].Normalize() > 0) m_nAxes++;
	m_AABBAxes[m_nAxes] = m_up.Cross(TVector3<T>(0, 0, 1));
	if(m_AABBAxes[m_nAxes].Normalize() > 0) m_nAxes++;
	TVector3<T> e1 = m_dir + right * m_dx + m_up * m_dy;
	m_AABBAxes[m_nAxes] = e1.Cross(TVector3<T>(1, 0, 0));
	if(m_AABBAxes[m_nAxes].Normalize() > 0) m_nAxes++;
	m_AABBAxes[m_nAxes] = e1.Cross(TVector3<T>(0, 1, 0));
	if(m_AABBAxes[m_nAxes].Normalize() > 0) m_nAxes++;
	m_AABBAxes[m_nAxes] = e1.Cross(TVector3<T>(0, 0, 1));
	if(m_AABBAxes[m_nAxes].Normalize() > 0) m_nAxes++;
	TVector3<T> e2 = m_dir - right * m_dx + m_up * m_dy;
	m_AABBAxes[m_nAxes] = e2.Cross(TVector3<T>(1, 0, 0));
	if(m_AABBAxes[m_nAxes].Normalize() > 0) m_nAxes++;
	m_AABBAxes[m_nAxes] = e2.Cross(TVector3<T>(0, 1, 0));
	if(m_AABBAxes[m_nAxes].Normalize() > 0) m_nAxes++;
	m_AABBAxes[m_nAxes] = e2.Cross(TVector3<T>(0, 0, 1));
	if(m_AABBAxes[m_nAxes].Normalize() > 0) m_nAxes++;
	TVector3<T> e3 = m_dir - right * m_dx - m_up * m_dy;
	m_AABBAxes[m_nAxes] = e3.Cross(TVector3<T>(1, 0, 0));
	if(m_AABBAxes[m_nAxes].Normalize() > 0) m_nAxes++;
	m_AABBAxes[m_nAxes] = e3.Cross(TVector3<T>(0, 1, 0));
	if(m_AABBAxes[m_nAxes].Normalize() > 0) m_nAxes++;
	m_AABBAxes[m_nAxes] = e3.Cross(TVector3<T>(0, 0, 1));
	if(m_AABBAxes[m_nAxes].Normalize() > 0) m_nAxes++;
	TVector3<T> e4 = m_dir + right * m_dx - m_up * m_dy;
	m_AABBAxes[m_nAxes] = e4.Cross(TVector3<T>(1, 0, 0));
	if(m_AABBAxes[m_nAxes].Normalize() > 0) m_nAxes++;
	m_AABBAxes[m_nAxes] = e4.Cross(TVector3<T>(0, 1, 0));
	if(m_AABBAxes[m_nAxes].Normalize() > 0) m_nAxes++;
	m_AABBAxes[m_nAxes] = e4.Cross(TVector3<T>(0, 0, 1));
	if(m_AABBAxes[m_nAxes].Normalize() > 0) m_nAxes++;

	TVector3<T> vec[8] = {
		m_eye + e1 * zNear,
		m_eye + e1 * zFar,
		m_eye + e2 * zNear,
		m_eye + e2 * zFar,
		m_eye + e3 * zNear,
		m_eye + e3 * zFar,
		m_eye + e4 * zNear,
		m_eye + e4 * zFar,
	};
	for(int i = 0; i < m_nAxes; i++) {
		m_AABBAxesMin[i] = FLT_MAX;
		m_AABBAxesMax[i] = -FLT_MAX;
		for(int j = 0; j < 8; j++) {
			T dot = vec[j].Dot(m_AABBAxes[i]);
			if(dot < m_AABBAxesMin[i]) m_AABBAxesMin[i] = dot;
			if(dot > m_AABBAxesMax[i]) m_AABBAxesMax[i] = dot;
		}
	}
}

template <typename T>
int TFrustum<T>::TestAABB(T x1, T y1, T z1, T x2, T y2, T z2)
{
	int result = 1;
	for(int i = 0; i < m_nAxes; i++) {
		T aabbMin = 0, aabbMax = 0;
		if(m_AABBAxes[i].x > 0) {aabbMin += x1 * m_AABBAxes[i].x; aabbMax += x2 * m_AABBAxes[i].x;}
		else {aabbMin += x2 * m_AABBAxes[i].x; aabbMax += x1 * m_AABBAxes[i].x;}
		if(m_AABBAxes[i].y > 0) {aabbMin += y1 * m_AABBAxes[i].y; aabbMax += y2 * m_AABBAxes[i].y;}
		else {aabbMin += y2 * m_AABBAxes[i].y; aabbMax += y1 * m_AABBAxes[i].y;}
		if(m_AABBAxes[i].z > 0) {aabbMin += z1 * m_AABBAxes[i].z; aabbMax += z2 * m_AABBAxes[i].z;}
		else {aabbMin += z2 * m_AABBAxes[i].z; aabbMax += z1 * m_AABBAxes[i].z;}

		if(m_AABBAxesMax[i] <= aabbMin || m_AABBAxesMin[i] >= aabbMax) return -1;
		if(!(m_AABBAxesMin[i] <= aabbMin && m_AABBAxesMax[i] >= aabbMax)) result = 0;
	}
	return result;
}
