#include "Common.h"


template <typename T>
FORCE_INLINE TQuaternion<T>::TQuaternion()
{
}
template <typename T>
FORCE_INLINE TQuaternion<T>::TQuaternion(T w, T x, T y, T z):w(w), x(x), y(y), z(z)
{
}
template <typename T>
FORCE_INLINE TQuaternion<T>::TQuaternion(const TQuaternion<T>& q):w(q.w), x(q.x), y(q.y), z(q.z)
{
}

template <typename T>
FORCE_INLINE TQuaternion<T>& TQuaternion<T>::operator= (const TQuaternion<T>& q)
{
	w = q.w;
	x = q.x;
	y = q.y;
	z = q.z;
	return *this;
}
template <typename T>
FORCE_INLINE bool TQuaternion<T>::operator== (const TQuaternion<T>& q) const
{
	return w == q.w && x == q.x && y == q.y && z == q.z;
}
template <typename T>
FORCE_INLINE TQuaternion<T> TQuaternion<T>::operator+ (const TQuaternion<T>& q) const
{
	return TQuaternion<T>(w + q.w, x + q.x, y + q.y, z + q.z);
}
template <typename T>
FORCE_INLINE TQuaternion<T> TQuaternion<T>::operator- (const TQuaternion<T>& q) const
{
	return TQuaternion<T>(w - q.w, x - q.x, y - q.y, z - q.z);
}
template <typename T>
FORCE_INLINE TQuaternion<T> TQuaternion<T>::operator* (const TQuaternion<T>& q) const
{
	TQuaternion<T> result;
    result.w = w*q.w - x*q.x - y*q.y - z*q.z;
    result.x = w*q.x + x*q.w + y*q.z - z*q.y;
    result.y = w*q.y + y*q.w + z*q.x - x*q.z;
    result.z = w*q.z + z*q.w + x*q.y - y*q.x;
    return result;
}
template <typename T>
FORCE_INLINE TQuaternion<T> TQuaternion<T>::operator* (T scalar) const
{
	return TQuaternion<T>(w * scalar, x * scalar, y * scalar, z * scalar);
}

template <typename T>
FORCE_INLINE T TQuaternion<T>::Length () const
{
	return sqrt(x * x + y * y + z * z + w * w);
}
template <typename T>
FORCE_INLINE T TQuaternion<T>::Dot (const TQuaternion<T>& q) const
{
	return x * q.x + y * q.y + z * q.z + w * q.w;
}
template <typename T>
FORCE_INLINE T TQuaternion<T>::Normalize ()
{
	T l = Length();
	if(l > 0) {
		w /= l;
		x /= l;
		y /= l;
		z /= l;
	}
	return l;
}
template <typename T>
FORCE_INLINE TQuaternion<T> TQuaternion<T>::Inverse () const
{
	TQuaternion<T> inverse;

    T norm = x * x + y * y + z * z + w * w;
    if (norm > 0)
    {
        T invNorm = 1.0 / norm;
        inverse.w = w*invNorm;
        inverse.x = -x*invNorm;
        inverse.y = -y*invNorm;
        inverse.z = -z*invNorm;
    }
    else
    {
        inverse.w = 0;
        inverse.x = 0;
        inverse.y = 0;
        inverse.z = 0;
    }

    return inverse;
}
template <typename T>
FORCE_INLINE TQuaternion<T> TQuaternion<T>::Conjugate () const
{
    return TQuaternion<T>(w, -x, -y, -z);
}
template <typename T>
FORCE_INLINE TQuaternion<T> TQuaternion<T>::Exp () const
{
    TQuaternion<T> result;

    T angle = sqrt(x*x + y*y + z*z);

    T sn = sin(angle);
    result.w = cos(angle);

    if (sn != 0)
    {
        T coeff = sn/angle;
		result.x = x * coeff;
		result.y = y * coeff;
		result.z = z * coeff;
    }
    else
    {
		result.x = x;
		result.y = y;
		result.z = z;
    }

    return result;
}
template <typename T>
FORCE_INLINE TQuaternion<T> TQuaternion<T>::Log () const
{
	TQuaternion<T> result;
    result.w = 0;

    if (fabs(w) < 1)
    {
        T angle = acos(w);
        T sn = sin(angle);
        if (sn != 0)
        {
            T coeff = angle/sn;
			result.x = x * coeff;
			result.y = y * coeff;
			result.z = z * coeff;
            return result;
        }
    }

	result.x = x;
	result.y = y;
	result.z = z;
    return result;
}
template <typename T>
FORCE_INLINE TQuaternion<T>& TQuaternion<T>::Slerp (T t, const TQuaternion<T>& p, const TQuaternion<T>& q)
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

        w = coeff0*p.w + coeff1*q.w;
        x = coeff0*p.x + coeff1*q.x;
        y = coeff0*p.y + coeff1*q.y;
        z = coeff0*p.z + coeff1*q.z;
    }
    else
    {
        w = p.w;
        x = p.x;
        y = p.y;
        z = p.z;
    }

	return *this;
}

template <typename T>
FORCE_INLINE TMatrix<T> TQuaternion<T>::ToMatrix() const
{
	T twoX  = 2*x;
    T twoY  = 2*y;
    T twoZ  = 2*z;
    T twoWX = twoX*w;
    T twoWY = twoY*w;
    T twoWZ = twoZ*w;
    T twoXX = twoX*x;
    T twoXY = twoY*x;
    T twoXZ = twoZ*x;
    T twoYY = twoY*y;
    T twoYZ = twoZ*y;
    T twoZZ = twoZ*z;

	return TMatrix<T>(1 - (twoYY + twoZZ), twoXY - twoWZ, twoXZ + twoWY, 0,
		twoXY + twoWZ, 1 - (twoXX + twoZZ), twoYZ - twoWX, 0,
		twoXZ - twoWY, twoYZ + twoWX, 1 - (twoXX + twoYY), 0,
		0, 0, 0, 1);
}
template <typename T>
FORCE_INLINE TEuler<T> TQuaternion<T>::ToEuler() const
{
	TQuaternion<T> alt(w, -y, x, z);
	T cy, sy, cx, sx, cz, sz;
	T a = alt.w*alt.x - alt.y*alt.z;
    T b = 0.5*(alt.w*alt.w - alt.x*alt.x - alt.y*alt.y + alt.z*alt.z);

    T length = sqrt(a*a + b*b);
    if (length > 0)
    {
        T invLength = 1/length;
        T sigma0 = a * invLength;
        T gamma0 = b * invLength;
        if (gamma0 >= 0)
        {
            cx = sqrt(0.5*(1 + gamma0));
            sx = (0.5)*sigma0/cx;
        }
        else
        {
            sx = sqrt(0.5*(1 - gamma0));
            cx = 0.5*sigma0/sx;
        }

        T tmp0 = cx*alt.w + sx*alt.x;
        T tmp1 = cx*alt.z - sx*alt.y;
        invLength = 1 / sqrt(tmp0*tmp0 + tmp1*tmp1);
        cz = tmp0 * invLength;
        sz = tmp1 * invLength;

        if(fabs(cz) >= fabs(sz))
        {
            invLength = 1/cz;
            cy = tmp0 * invLength;
            sy = (cx*alt.y + sx*alt.z) * invLength;
        }
        else
        {
            invLength = 1/sz;
            cy = tmp1 * invLength;
            sy = (cx*alt.x - sx*alt.w) * invLength;
        }
    }
    else
    {
        if(alt.w*alt.y + alt.x*alt.z > 0)
        {
            cx = 1;
            sx = 0;
            cy = sqrt(0.5);
            sy = sqrt(0.5);
            cz = sqrt(2.0) * alt.w;
            sz = sqrt(2.0) * alt.x;
        }
        else
        {
            cx = 1;
            sx = 0;
            cy = sqrt(0.5);
            sy = -sqrt(0.5);
            cz = sqrt(2.0) * alt.w;
            sz = -sqrt(2.0) * alt.x;
        }
    }
    sy = -sy;

	return TEuler<T>(acos(cy), acos(cx), acos(cz));
}
