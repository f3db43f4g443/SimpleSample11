#include "Common.h"


template <typename T>
FORCE_INLINE TMatrix2D<T>::TMatrix2D ()
{
}
template <typename T>
FORCE_INLINE TMatrix2D<T>::TMatrix2D (const TMatrix2D& mat):m00(mat.m00), m01(mat.m01), m02(mat.m02),
	m10(mat.m10), m11(mat.m11), m12(mat.m12),
	m20(mat.m20), m21(mat.m21), m22(mat.m22)
{
}
template <typename T>
FORCE_INLINE TMatrix2D<T>::TMatrix2D (
    T m00, T m01, T m02,
    T m10, T m11, T m12,
    T m20, T m21, T m22):m00(m00), m01(m01), m02(m02),
	m10(m10), m11(m11), m12(m12),
	m20(m20), m21(m21), m22(m22)
{
}

template <typename T>
FORCE_INLINE TMatrix2D<T>& TMatrix2D<T>::operator= (const TMatrix2D<T>& mat)
{
	memcpy(this, &mat, sizeof(TMatrix2D<T>));
	return *this;
}
template <typename T>
FORCE_INLINE TMatrix2D<T> TMatrix2D<T>::operator+ (const TMatrix2D<T>& mat) const
{
	return TMatrix<T>(m00 + mat.m00, m01 + mat.m01, m02 + mat.m02,
		m10 + mat.m10, m11 + mat.m11, m12 + mat.m12,
		m20 + mat.m20, m21 + mat.m21, m22 + mat.m22);
}
template <typename T>
FORCE_INLINE TMatrix2D<T> TMatrix2D<T>::operator- (const TMatrix2D<T>& mat) const
{
	return TMatrix2D<T>(m00 - mat.m00, m01 - mat.m01, m02 - mat.m02,
		m10 - mat.m10, m11 - mat.m11, m12 - mat.m12,
		m20 - mat.m20, m21 - mat.m21, m22 - mat.m22);
}
template <typename T>
FORCE_INLINE TMatrix2D<T> TMatrix2D<T>::operator* (T scalar) const
{
	return TMatrix2D<T>(m00 * scalar, m01 * scalar, m02 * scalar,
		m10 * scalar, m11 * scalar, m12 * scalar,
		m20 * scalar, m21 * scalar, m22 * scalar);
}

template <typename T>
FORCE_INLINE bool TMatrix2D<T>::operator== ( const TMatrix2D& mat )
{
	return m00 == mat.m00 && m01 == mat.m01 && m02 == mat.m02
		&& m10 == mat.m10 && m11 == mat.m11 && m12 == mat.m12
		&& m20 == mat.m20 && m21 == mat.m21 && m22 == mat.m22;
}

template <typename T>
FORCE_INLINE bool TMatrix2D<T>::operator!= ( const TMatrix2D& mat )
{
	return !( *this == mat );
}

template <typename T>
FORCE_INLINE TVector3<T> TMatrix2D<T>::operator* (const TVector3<T>& vec) const
{
	return TVector3<T>(vec.Dot(TVector3<T>(m00, m01, m02)),
		vec.Dot(TVector3<T>(m10, m11, m12)),
		vec.Dot(TVector3<T>(m20, m21, m22)));
}
template <typename T>
FORCE_INLINE TMatrix2D<T> TMatrix2D<T>::operator* (const TMatrix2D<T>& mat) const
{
	return TMatrix2D<T>(m00 * mat.m00 + m01 * mat.m10 + m02 * mat.m20,
		m00 * mat.m01 + m01 * mat.m11 + m02 * mat.m21,
		m00 * mat.m02 + m01 * mat.m12 + m02 * mat.m22,
		m10 * mat.m00 + m11 * mat.m10 + m12 * mat.m20,
		m10 * mat.m01 + m11 * mat.m11 + m12 * mat.m21,
		m10 * mat.m02 + m11 * mat.m12 + m12 * mat.m22,
		m20 * mat.m00 + m21 * mat.m10 + m22 * mat.m20,
		m20 * mat.m01 + m21 * mat.m11 + m22 * mat.m21,
		m20 * mat.m02 + m21 * mat.m12 + m22 * mat.m22);
}

template <typename T>
FORCE_INLINE TVector2<T> TMatrix2D<T>::MulVector2Pos(const TVector2<T>& vec) const
{
	TVector3<T> vec3(vec.x, vec.y, 1);
	TVector3<T> vec3a = *this * vec3;
	return TVector2<T>(vec3a.x, vec3a.y);
}
template <typename T>
FORCE_INLINE TVector2<T> TMatrix2D<T>::MulVector2Dir(const TVector2<T>& vec) const
{
	TVector3<T> vec3(vec.x, vec.y, 0);
	TVector3<T> vec3a = *this * vec3;
	return TVector2<T>(vec3a.x, vec3a.y);
}

template <typename T>
FORCE_INLINE TVector2<T> TMatrix2D<T>::MulTVector2PosNoScale(const TVector2<T>& vec) const
{
	TVector2<T> vec2( vec.x - m02, vec.y - m12 );
	return TVector2<T>( vec2.Dot( TVector2<T>( m00, m10 ) ), vec2.Dot( TVector2<T>( m01, m11 ) ) );
}

template <typename T>
FORCE_INLINE TVector2<T> TMatrix2D<T>::MulTVector2DirNoScale(const TVector2<T>& vec) const
{
	TVector2<T> vec2( vec.x, vec.y );
	return TVector2<T>( vec2.Dot( TVector2<T>( m00, m10 ) ), vec2.Dot( TVector2<T>( m01, m11 ) ) );
}

template <typename T>
FORCE_INLINE TVector2<T> TMatrix2D<T>::GetPosition() const
{
	return TVector2<T>(m02, m12);
}

template <typename T>
FORCE_INLINE void TMatrix2D<T>::SetPosition( TVector2<T> pos )
{
	m02 = pos.x;
	m12 = pos.y;
}

template <typename T>
FORCE_INLINE void TMatrix2D<T>::Identity()
{
	*this = TMatrix2D<T>( 1, 0, 0, 0, 1, 0, 0, 0, 1 );
}

template <typename T>
FORCE_INLINE void TMatrix2D<T>::Transform(T x, T y, T angle, T s)
{
	T cs = cosf(angle);
    T sn = sinf(angle);

	m00 = s * cs;
	m01 = s * -sn;
	m02 = x;
	m10 = s * sn;
	m11 = s * cs;
	m12 = y;
	m20 = 0;
	m21 = 0;
	m22 = 1.0f;
}

template <typename T>
FORCE_INLINE void TMatrix2D<T>::Decompose( T& x, T& y, T& angle, T& s ) const
{
	x = m02;
	y = m12;
	s = sqrt( m00 * m00 + m01 * m01 );
	angle = atan2( m10, m00 );
}

template <typename T>
FORCE_INLINE void TMatrix2D<T>::Decompose( T& x, T& y, T& angle, T& sx, T& sy ) const
{
	x = m02;
	y = m12;
	sx = sqrt( m00 * m00 + m01 * m01 );
	sy = sqrt( m10 * m10 + m11 * m11 );
	angle = atan2( -m01, m00 );
}

template <typename T>
FORCE_INLINE void TMatrix2D<T>::Transform(T x, T y, T angle, T sx, T sy)
{
	T cs = cosf(angle);
    T sn = sinf(angle);

	m00 = sx * cs;
	m01 = sy * -sn;
	m02 = x;
	m10 = sx * sn;
	m11 = sy * cs;
	m12 = y;
	m20 = 0;
	m21 = 0;
	m22 = 1.0f;
}

template <typename T>
FORCE_INLINE void TMatrix2D<T>::Translate(T x, T y)
{
	m00 = 1;
	m01 = 0;
	m02 = x;
	m10 = 0;
	m11 = 1;
	m12 = y;
	m20 = 0;
	m21 = 0;
	m22 = 1;
}
template <typename T>
FORCE_INLINE void TMatrix2D<T>::Rotate(T angle)
{
	T cs = cosf(angle);
	T sn = sinf(angle);

	m00 = cs;
	m01 = -sn;
	m02 = 0;
	m10 = sn;
	m11 = cs;
	m12 = 0;
	m20 = 0;
	m21 = 0;
	m22 = 1.0f;
}
template <typename T>
FORCE_INLINE void TMatrix2D<T>::Scale(T s)
{
	m00 = s;
	m01 = 0;
	m02 = 0;
	m10 = 0;
	m11 = s;
	m12 = 0;
	m20 = 0;
	m21 = 0;
	m22 = 1;
}

template <typename T>
FORCE_INLINE TMatrix2D<T> TMatrix2D<T>::Inverse() const
{
	TMatrix2D<T> invTrans;
	invTrans.Translate( -m02, -m12 );
	float s2 = m00 * m00 + m01 * m01;
	TMatrix2D<T> invRot( m00 / s2, m10 / s2, 0, m01 / s2, m11 / s2, 0, 0, 0, 1 );
	return invRot * invTrans;
}

template <typename T>
FORCE_INLINE TMatrix2D<T> TMatrix2D<T>::InverseNoScale() const
{
	TMatrix2D<T> invTrans;
	invTrans.Translate( -m02, -m12 );
	TMatrix2D<T> invRot( m00, m10, 0, m01, m11, 0, 0, 0, 1 );
	return invRot * invTrans;
}