#include "Common.h"


template <typename T>
FORCE_INLINE TVector2<T>::TVector2()
{
}
template <typename T>
FORCE_INLINE TVector2<T>::TVector2(T x, T y):x(x), y(y)
{
}
template <typename T>
FORCE_INLINE TVector2<T>::TVector2(const TVector2<T>& v):x(v.x), y(v.y)
{
}
template <typename T>
FORCE_INLINE TVector2<T>::~TVector2()
{
}

template <typename T>
FORCE_INLINE TVector2<T>& TVector2<T>::operator= (const TVector2<T>& vec)
{
	x = vec.x;
	y = vec.y;
	return *this;
}
template <typename T>
FORCE_INLINE bool TVector2<T>::operator== (const TVector2<T>& vec) const
{
	return x == vec.x && y == vec.y;
}
template <typename T>
FORCE_INLINE bool TVector2<T>::operator!= ( const TVector2<T>& vec ) const
{
	return x != vec.x || y != vec.y;
}
template <typename T>
FORCE_INLINE TVector2<T> TVector2<T>::operator+ (const TVector2<T>& vec) const
{
	return TVector2<T>(x + vec.x, y + vec.y);
}
template <typename T>
FORCE_INLINE TVector2<T> TVector2<T>::operator- (const TVector2<T>& vec) const
{
	return TVector2<T>(x - vec.x, y - vec.y);
}
template <typename T>
FORCE_INLINE TVector2<T> TVector2<T>::operator* (T scalar) const
{
	return TVector2<T>(x * scalar, y * scalar);
}
template <typename T>
FORCE_INLINE TVector2<T> TVector2<T>::operator* (const TVector2<T>& vec) const
{
	return TVector2<T>(x * vec.x, y * vec.y);
}

template <typename T>
FORCE_INLINE TVector2<T> TVector2<T>::operator/ ( T scalar ) const
{
	return TVector2<T>( x / scalar, y / scalar );
}

template <typename T>
FORCE_INLINE TVector2<T> TVector2<T>::operator/ ( const TVector2<T>& vec ) const
{
	return TVector2<T>( x / vec.x, y / vec.y );
}

template <typename T>
FORCE_INLINE T TVector2<T>::Length () const
{
	return sqrt(x * x + y * y);
}

template <typename T>
FORCE_INLINE T TVector2<T>::Length2() const
{
	return x * x + y * y;
}

template <typename T>
FORCE_INLINE T TVector2<T>::Dot (const TVector2<T>& vec) const
{
	return x * vec.x + y * vec.y;
}
template <typename T>
FORCE_INLINE T TVector2<T>::Normalize ()
{
	T l = sqrt(x * x + y * y);
	if(l > 0) {
		x /= l;
		y /= l;
	}
	return l;
}

template <typename T>
FORCE_INLINE TVector2<T>& TVector2<T>::Slerp( T t, const TVector2<T>& p, const TVector2<T>& q )
{
	T cs = Min<T>( 1, p.Dot( q ) );
	T angle = acos( cs );

	if( angle != 0 )
	{
		T sn = sin( angle );
		T invSn = 1.0 / sn;
		T tAngle = t*angle;
		T coeff0 = sin( angle - tAngle )*invSn;
		T coeff1 = sin( tAngle )*invSn;

		x = coeff0*p.x + coeff1*q.x;
		y = coeff0*p.y + coeff1*q.y;
	}
	else
	{
		x = p.x;
		y = p.y;
	}

	return *this;
}