#include "Common.h"

template <typename T>
TVector4<T>::TVector4()
{
}
template <typename T>
TVector4<T>::TVector4(T x, T y, T z, T w):x(x), y(y), z(z), w(w)
{
}
template <typename T>
TVector4<T>::TVector4(const TVector4<T>& v):x(v.x), y(v.y), z(v.z), w(v.w)
{
}
template <typename T>
TVector4<T>::~TVector4()
{
}

template <typename T>
TVector4<T>& TVector4<T>::operator= (const TVector4<T>& vec)
{
	x = vec.x;
	y = vec.y;
	z = vec.z;
	w = vec.w;
	return *this;
}
template <typename T>
bool TVector4<T>::operator== (const TVector4<T>& vec) const
{
	return x == vec.x && y == vec.y && z == vec.z && w == vec.w;
}
template <typename T>
TVector4<T> TVector4<T>::operator+ (const TVector4<T>& vec) const
{
	return TVector4(x + vec.x, y + vec.y, z + vec.z, w + vec.w);
}
template <typename T>
TVector4<T> TVector4<T>::operator- (const TVector4<T>& vec) const
{
	return TVector4<T>(x - vec.x, y - vec.y, z - vec.z, w - vec.w);
}
template <typename T>
TVector4<T> TVector4<T>::operator* (T scalar) const
{
	return TVector4<T>(x * scalar, y * scalar, z * scalar, w * scalar);
}
template <typename T>
TVector4<T> TVector4<T>::operator* (const TVector4& vec) const
{
	return TVector4<T>(x * vec.x, y * vec.y, z * vec.z, w * vec.w);
}

template <typename T>
T TVector4<T>::Length () const
{
	return sqrt(x * x + y * y + z * z + w * w);
}
template <typename T>
T TVector4<T>::Dot (const TVector4<T>& vec) const
{
	return x * vec.x + y * vec.y + z * vec.z + w * vec.w;
}
template <typename T>
TVector4<T> TVector4<T>::Cross (const TVector4<T>& vec) const
{
	return TVector4(y * vec.z - z * vec.y, z * vec.x - x * vec.z, x * vec.y - y * vec.x, 0);
}
template <typename T>
T TVector4<T>::Normalize ()
{
	T l = sqrt(x * x + y * y + z * z + w * w);
	if(l > 0) {
		x /= l;
		y /= l;
		z /= l;
		w /= l;
	}
	return l;
}
