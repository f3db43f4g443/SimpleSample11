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