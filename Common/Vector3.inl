#include "Common.h"


template <typename T>
FORCE_INLINE TVector3<T>::TVector3()
{
}
template <typename T>
FORCE_INLINE TVector3<T>::TVector3(T x, T y, T z):x(x), y(y), z(z)
{
}
template <typename T>
FORCE_INLINE TVector3<T>::TVector3(const TVector3<T>& v):x(v.x), y(v.y), z(v.z)
{
}
template <typename T>
FORCE_INLINE TVector3<T>::~TVector3()
{
}

template <typename T>
FORCE_INLINE TVector3<T>& TVector3<T>::operator= (const TVector3<T>& vec)
{
	x = vec.x;
	y = vec.y;
	z = vec.z;
	return *this;
}
template <typename T>
FORCE_INLINE bool TVector3<T>::operator== (const TVector3<T>& vec) const
{
	return x == vec.x && y == vec.y && z == vec.z;
}
template <typename T>
FORCE_INLINE TVector3<T> TVector3<T>::operator+ (const TVector3<T>& vec) const
{
	return TVector3<T>(x + vec.x, y + vec.y, z + vec.z);
}
template <typename T>
FORCE_INLINE TVector3<T> TVector3<T>::operator- (const TVector3<T>& vec) const
{
	return TVector3<T>(x - vec.x, y - vec.y, z - vec.z);
}
template <typename T>
FORCE_INLINE TVector3<T> TVector3<T>::operator* (T scalar) const
{
	return TVector3<T>(x * scalar, y * scalar, z * scalar);
}
template <typename T>
FORCE_INLINE TVector3<T> TVector3<T>::operator* (const TVector3<T>& vec) const
{
	return TVector3<T>(x * vec.x, y * vec.y, z * vec.z);
}

template <typename T>
FORCE_INLINE T TVector3<T>::Length () const
{
	return sqrt(x * x + y * y + z * z);
}
template <typename T>
FORCE_INLINE T TVector3<T>::Dot (const TVector3<T>& vec) const
{
	return x * vec.x + y * vec.y + z * vec.z;
}
template <typename T>
FORCE_INLINE TVector3<T> TVector3<T>::Cross (const TVector3<T>& vec) const
{
	return TVector3<T>(y * vec.z - z * vec.y, z * vec.x - x * vec.z, x * vec.y - y * vec.x);
}
template <typename T>
FORCE_INLINE T TVector3<T>::Normalize ()
{
	T l = sqrt(x * x + y * y + z * z);
	if(l > 0) {
		x /= l;
		y /= l;
		z /= l;
	}
	return l;
}

template <typename T>
FORCE_INLINE TVector3<T>& TVector3<T>::Slerp(T t, const TVector3<T>& p, const TVector3<T>& q)
{
	T cs = p.Dot(q);
	T angle = acos(cs);

    if (angle != 0)
    {
        T sn = sin(angle);
        T invSn = 1.0/sn;
        T tAngle = t*angle;
        T coeff0 = sin(angle - tAngle)*invSn;
        T coeff1 = sin(tAngle)*invSn;

        x = coeff0*p.x + coeff1*q.x;
        y = coeff0*p.y + coeff1*q.y;
        z = coeff0*p.z + coeff1*q.z;
    }
    else
    {
        x = p.x;
        y = p.y;
        z = p.z;
    }

	return *this;
}