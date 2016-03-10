#include "Common.h"


template <typename T>
TMatrix<T>::TMatrix ()
{
}
template <typename T>
TMatrix<T>::TMatrix (const TMatrix& mat):m00(mat.m00), m01(mat.m01), m02(mat.m02), m03(mat.m03),
	m10(mat.m10), m11(mat.m11), m12(mat.m12), m13(mat.m13),
	m20(mat.m20), m21(mat.m21), m22(mat.m22), m23(mat.m23),
	m30(mat.m30), m31(mat.m31), m32(mat.m32), m33(mat.m33)
{
}
template <typename T>
TMatrix<T>::TMatrix (
    T m00, T m01, T m02, T m03,
    T m10, T m11, T m12, T m13,
    T m20, T m21, T m22, T m23,
    T m30, T m31, T m32, T m33):m00(m00), m01(m01), m02(m02), m03(m03),
	m10(m10), m11(m11), m12(m12), m13(m13),
	m20(m20), m21(m21), m22(m22), m23(m23),
	m30(m30), m31(m31), m32(m32), m33(m33)
{
}

template <typename T>
TMatrix<T>& TMatrix<T>::operator= (const TMatrix<T>& mat)
{
	memcpy(this, &mat, sizeof(TMatrix<T>));
	return *this;
}
template <typename T>
TMatrix<T> TMatrix<T>::operator+ (const TMatrix<T>& mat) const
{
	return TMatrix<T>(m00 + mat.m00, m01 + mat.m01, m02 + mat.m02, m03 + mat.m03,
		m10 + mat.m10, m11 + mat.m11, m12 + mat.m12, m13 + mat.m13,
		m20 + mat.m20, m21 + mat.m21, m22 + mat.m22, m23 + mat.m23,
		m30 + mat.m30, m31 + mat.m31, m32 + mat.m32, m33 + mat.m33);
}
template <typename T>
TMatrix<T> TMatrix<T>::operator- (const TMatrix<T>& mat) const
{
	return TMatrix<T>(m00 - mat.m00, m01 - mat.m01, m02 - mat.m02, m03 - mat.m03,
		m10 - mat.m10, m11 - mat.m11, m12 - mat.m12, m13 - mat.m13,
		m20 - mat.m20, m21 - mat.m21, m22 - mat.m22, m23 - mat.m23,
		m30 - mat.m30, m31 - mat.m31, m32 - mat.m32, m33 - mat.m33);
}
template <typename T>
TMatrix<T> TMatrix<T>::operator* (T scalar) const
{
	return TMatrix<T>(m00 * scalar, m01 * scalar, m02 * scalar, m03 * scalar,
		m10 * scalar, m11 * scalar, m12 * scalar, m13 * scalar,
		m20 * scalar, m21 * scalar, m22 * scalar, m23 * scalar,
		m30 * scalar, m31 * scalar, m32 * scalar, m33 * scalar);
}

template <typename T>
TVector4<T> TMatrix<T>::operator* (const TVector4<T>& vec) const
{
	return TVector4<T>(vec.Dot(TVector4<T>(m00, m01, m02, m03)),
		vec.Dot(TVector4<T>(m10, m11, m12, m13)),
		vec.Dot(TVector4<T>(m20, m21, m22, m23)),
		vec.Dot(TVector4<T>(m30, m31, m32, m33)));
}
template <typename T>
TMatrix<T> TMatrix<T>::operator* (const TMatrix<T>& mat) const
{
	return TMatrix<T>(m00 * mat.m00 + m01 * mat.m10 + m02 * mat.m20 + m03 * mat.m30,
		m00 * mat.m01 + m01 * mat.m11 + m02 * mat.m21 + m03 * mat.m31,
		m00 * mat.m02 + m01 * mat.m12 + m02 * mat.m22 + m03 * mat.m32,
		m00 * mat.m03 + m01 * mat.m13 + m02 * mat.m23 + m03 * mat.m33,
		m10 * mat.m00 + m11 * mat.m10 + m12 * mat.m20 + m13 * mat.m30,
		m10 * mat.m01 + m11 * mat.m11 + m12 * mat.m21 + m13 * mat.m31,
		m10 * mat.m02 + m11 * mat.m12 + m12 * mat.m22 + m13 * mat.m32,
		m10 * mat.m03 + m11 * mat.m13 + m12 * mat.m23 + m13 * mat.m33,
		m20 * mat.m00 + m21 * mat.m10 + m22 * mat.m20 + m23 * mat.m30,
		m20 * mat.m01 + m21 * mat.m11 + m22 * mat.m21 + m23 * mat.m31,
		m20 * mat.m02 + m21 * mat.m12 + m22 * mat.m22 + m23 * mat.m32,
		m20 * mat.m03 + m21 * mat.m13 + m22 * mat.m23 + m23 * mat.m33,
		m30 * mat.m00 + m31 * mat.m10 + m32 * mat.m20 + m33 * mat.m30,
		m30 * mat.m01 + m31 * mat.m11 + m32 * mat.m21 + m33 * mat.m31,
		m30 * mat.m02 + m31 * mat.m12 + m32 * mat.m22 + m33 * mat.m32,
		m30 * mat.m03 + m31 * mat.m13 + m32 * mat.m23 + m33 * mat.m33);
}

template <typename T>
TMatrix<T> TMatrix<T>::Transpose ()
{
	return TMatrix<T>(m00, m10, m20, m30,
		m01, m11, m21, m31,
		m02, m12, m22, m32,
		m03, m13, m23, m33);
}
template <typename T>
TMatrix<T> TMatrix<T>::Inverse ()
{
	T a0 = m00*m11 - m01*m10;
    T a1 = m00*m12 - m02*m10;
    T a2 = m00*m13 - m03*m10;
    T a3 = m01*m12 - m02*m11;
    T a4 = m01*m13 - m03*m11;
    T a5 = m02*m13 - m03*m12;
    T b0 = m20*m31 - m21*m30;
    T b1 = m20*m32 - m22*m30;
    T b2 = m20*m33 - m23*m30;
    T b3 = m21*m32 - m22*m31;
    T b4 = m21*m33 - m23*m31;
    T b5 = m22*m33 - m23*m32;

    T det = a0*b5 - a1*b4 + a2*b3 + a3*b2 - a4*b1 + a5*b0;
    if (fabs(det) <= 0)
    {
        return TMatrix(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    }

    TMatrix<T> inverse;
    inverse.m00 = + m11*b5 - m12*b4 + m13*b3;
    inverse.m10 = - m10*b5 + m12*b2 - m13*b1;
    inverse.m20 = + m10*b4 - m11*b2 + m13*b0;
    inverse.m30 = - m10*b3 + m11*b1 - m12*b0;
    inverse.m01 = - m01*b5 + m02*b4 - m03*b3;
    inverse.m11 = + m00*b5 - m02*b2 + m03*b1;
    inverse.m21 = - m00*b4 + m01*b2 - m03*b0;
    inverse.m31 = + m00*b3 - m01*b1 + m02*b0;
    inverse.m02 = + m31*a5 - m32*a4 + m33*a3;
    inverse.m12 = - m30*a5 + m32*a2 - m33*a1;
    inverse.m22 = + m30*a4 - m31*a2 + m33*a0;
    inverse.m32 = - m30*a3 + m31*a1 - m32*a0;
    inverse.m03 = - m21*a5 + m22*a4 - m23*a3;
    inverse.m13 = + m20*a5 - m22*a2 + m23*a1;
    inverse.m23 = - m20*a4 + m21*a2 - m23*a0;
    inverse.m33 = + m20*a3 - m21*a1 + m22*a0;

    T invDet = 1.0f/det;
    inverse.m00 *= invDet;
    inverse.m01 *= invDet;
    inverse.m02 *= invDet;
    inverse.m03 *= invDet;
    inverse.m10 *= invDet;
    inverse.m11 *= invDet;
    inverse.m12 *= invDet;
    inverse.m13 *= invDet;
    inverse.m20 *= invDet;
    inverse.m21 *= invDet;
    inverse.m22 *= invDet;
    inverse.m23 *= invDet;
    inverse.m30 *= invDet;
    inverse.m31 *= invDet;
    inverse.m32 *= invDet;
    inverse.m33 *= invDet;

    return inverse;
}

template <typename T>
TVector3<T> TMatrix<T>::MulVector3Pos(const TVector3<T> vec)
{
	TVector4<T> vec4(vec.x, vec.y, vec.z, 1);
	TVector4<T> vec4a = *this * vec4;
	return TVector3<T>(vec4a.x, vec4a.y, vec4a.z);
}
template <typename T>
TVector3<T> TMatrix<T>::MulVector3Dir(const TVector3<T> vec)
{
	TVector4<T> vec4(vec.x, vec.y, vec.z, 0);
	TVector4<T> vec4a = *this * vec4;
	return TVector3<T>(vec4a.x, vec4a.y, vec4a.z);
}

template <typename T>
TVector3<T> TMatrix<T>::GetPosition()
{
	return TVector3<T>(m03, m13, m23);
}

template <typename T>
void TMatrix<T>::Identity()
{
	*this = TMatrix<T>( 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 );
}

template <typename T>
void TMatrix<T>::Transform(T x, T y, T z, T ax, T ay, T az, T angle, T sx, T sy, T sz)
{
	T cs = cosf(angle);
    T sn = sinf(angle);
    T oneMinusCos = 1.0f - cs;
    T x2 = ax*ax;
    T y2 = ay*ay;
    T z2 = az*az;
    T xym = ax*ay*oneMinusCos;
    T xzm = ax*az*oneMinusCos;
    T yzm = ay*az*oneMinusCos;
    T xSin = ax*sn;
    T ySin = ay*sn;
    T zSin = az*sn;

	m00 = sx * (x2*oneMinusCos + cs);
    m01 = sy * (xym - zSin);
    m02 = sz * (xzm + ySin);
    m03 = x;
    m10 = sx * (xym + zSin);
    m11 = sy * (y2*oneMinusCos + cs);
    m12 = sz * (yzm - xSin);
    m13 = y;
    m20 = sx * (xzm - ySin);
    m21 = sy * (yzm + xSin);
    m22 = sz * (z2*oneMinusCos + cs);
    m23 = z;
    m30 = 0.0f;
    m31 = 0.0f;
    m32 = 0.0f;
    m33 = 1.0f;
}

template <typename T>
void TMatrix<T>::Translate(T x, T y, T z)
{
	m00 = 1;
	m01 = 0;
	m02 = 0;
	m03 = x;
	m10 = 0;
	m11 = 1;
	m12 = 0;
	m13 = y;
	m20 = 0;
	m21 = 0;
	m22 = 1;
	m23 = z;
	m30 = 0;
	m31 = 0;
	m32 = 0;
	m33 = 1;
}
template <typename T>
void TMatrix<T>::Rotate(T ax, T ay, T az, T angle)
{
	T cs = cosf(angle);
    T sn = sinf(angle);
    Rotate(ax, ay, az, cs, sn);
}
template <typename T>
void TMatrix<T>::Rotate(T ax, T ay, T az, T cs, T sn)
{
	T oneMinusCos = 1.0f - cs;
	T x2 = ax*ax;
	T y2 = ay*ay;
	T z2 = az*az;
	T xym = ax*ay*oneMinusCos;
	T xzm = ax*az*oneMinusCos;
	T yzm = ay*az*oneMinusCos;
	T xSin = ax*sn;
	T ySin = ay*sn;
	T zSin = az*sn;

	m00 = x2*oneMinusCos + cs;
	m01 = xym - zSin;
	m02 = xzm + ySin;
	m03 = 0.0f;
	m10 = xym + zSin;
	m11 = y2*oneMinusCos + cs;
	m12 = yzm - xSin;
	m13 = 0.0f;
	m20 = xzm - ySin;
	m21 = yzm + xSin;
	m22 = z2*oneMinusCos + cs;
	m23 = 0.0f;
	m30 = 0.0f;
	m31 = 0.0f;
	m32 = 0.0f;
	m33 = 1.0f;
}
template <typename T>
void TMatrix<T>::Scale(T sx, T sy, T sz)
{
	m00 = sx;
	m01 = 0;
	m02 = 0;
	m03 = 0;
	m10 = 0;
	m11 = sy;
	m12 = 0;
	m13 = 0;
	m20 = 0;
	m21 = 0;
	m22 = sz;
	m23 = 0;
	m30 = 0;
	m31 = 0;
	m32 = 0;
	m33 = 1;
}
template <typename T>
void TMatrix<T>::PerspectiveProjection (T originx, T originy, T originz,
    T normalx, T normaly, T normalz, T eyex, T eyey, T eyez)
{
	T dotND = normalx * (eyex - originx) + normaly * (eyey - originy) + normalz * (eyez - originz);

    m00 = dotND - eyex*normalx;
    m01 = -eyex*normaly;
    m02 = -eyex*normalz;
    m03 = -(m00*eyex + m01*eyey + m02*eyez);
    m10 = -eyey*normalx;
    m11 = dotND - eyey*normaly;
    m12 = -eyey*normalz;
    m13 = -(m10*eyex + m11*eyey + m12*eyez);
    m20 = -eyez*normalx;
    m21 = -eyez*normaly;
    m22 = dotND- eyez*normalz;
    m23 = -(m20*eyex + m21*eyey + m22*eyez);
    m30 = -normalx;
    m31 = -normaly;
    m32 = -normalz;
    m33 = eyex * normalx + eyey * normaly + eyez * normalz;
}

template <typename T>
void TMatrix<T>::Decompose(T& x, T& y, T& z, T& ax, T& ay, T& az, T& angle,
		T& sx, T& sy, T& sz)
{
	x = m03;
	y = m13;
	z = m23;

	T det = m00*m11*m22*m33 - m00*m11*m23*m32 + m00*m12*m23*m31 - m00*m12*m21*m33 
		+ m00*m13*m21*m32 - m00*m13*m22*m31 - m01*m12*m23*m30 + m01*m12*m20*m33 
		- m01*m13*m20*m32 + m01*m13*m22*m30 - m01*m10*m22*m33 + m01*m10*m23*m32 
		+ m02*m13*m20*m31 - m02*m13*m21*m30 + m02*m10*m21*m33 - m02*m10*m23*m31 
		+ m02*m11*m23*m30 - m02*m11*m20*m33 - m03*m10*m21*m32 + m03*m10*m22*m31
		- m03*m11*m22*m30 + m03*m11*m20*m32 - m03*m12*m20*m31 + m03*m12*m21*m30;

	TVector3<T> row1(m00, m10, m20), row2(m01, m11, m21), row3(m02, m12, m22);
	sx = row1.Length();
	sy = row2.Length();
	sz = row3.Length();
	if(det < 0) {
		sx = -sx;
		sy = -sy;
		sz = -sz;
	}
	if(sx > 0) row1 = row1 * (1.0f / sx);
	if(sy > 0) row2 = row2 * (1.0f / sy);
	if(sz > 0) row3 = row3 * (1.0f / sz);

	TMatrix<T> mat1(row1.x, row2.x, row3.x, 0,
		row1.y, row2.y, row3.y, 0,
		row1.z, row2.z, row3.z, 0,
		0, 0, 0, 1);
	TQuaternion<T> quat = mat1.ToQuaternion();
	angle = acos(quat.w) * 2;
	T sq = sqrt(1 - quat.w * quat.w);
	if(sq > FLT_EPSILON) {
		ax = quat.x / sq;
		ay = quat.y / sq;
		az = quat.z / sq;
	}
	else {
		ax = 1;
		ay = 0;
		az = 0;
	}
}

template <typename T>
void TMatrix<T>::BillboardX()
{
	TVector3<T> myXAxis = TVector3<T>(m00, m10, m20);
	myXAxis.Normalize();
	TVector3<T> axis(myXAxis.y, -myXAxis.x, 0);
	T sn = axis.Normalize();
	T cs = myXAxis.z;
	if( sn < 0.0001f )
	{
		axis = TVector3<T>(0, 1, 0);
		axis.Normalize();
		sn = 0;
		cs = 1;
	}
	TMatrix<T> rotMatrix;
	if( m22 >= 0 )
		rotMatrix.Rotate(axis.x, axis.y, axis.z, sn, -cs);
	else
		rotMatrix.Rotate(axis.x, axis.y, axis.z, -sn, cs);
	T x = m03, y = m13, z = m23;
	*this = rotMatrix * this;
	m03 = x;
	m13 = y;
	m23 = z;
}

template <typename T>
void TMatrix<T>::BillboardX(const TVector3<T>& up)
{
	TVector3<T> myXAxis = TVector3<T>(m00, m10, m20);
	myXAxis.Normalize();
	TVector3<T> axis = myXAxis.Cross( up );
	T sn = axis.Normalize();
	T cs = up.Dot( myXAxis );
	if( sn < 0.0001f )
	{
		if( up.z > 0 )
			axis = TVector3<T>(0, up.z, -up.y);
		else
			axis = TVector3<T>(-up.y, up.x, 0);
		axis.Normalize();
		sn = 0;
		cs = 1;
	}
	TMatrix<T> rotMatrix;
	if( up.Dot(TVector3<T>(m02, m12, m22)) >= 0 )
		rotMatrix.Rotate(axis.x, axis.y, axis.z, sn, -cs);
	else
		rotMatrix.Rotate(axis.x, axis.y, axis.z, -sn, cs);
	T x = m03, y = m13, z = m23;
	*this = rotMatrix * this;
	m03 = x;
	m13 = y;
	m23 = z;
}

template <typename T>
void TMatrix<T>::BillboardY()
{
	TVector3<T> myYAxis = TVector3<T>(m01, m11, m21);
	myYAxis.Normalize();
	TVector3<T> axis(myYAxis.y, -myYAxis.x, 0);
	T sn = axis.Normalize();
	T cs = myYAxis.z;
	if( sn < 0.0001f )
	{
		axis = TVector3<T>(0, 1, 0);
		axis.Normalize();
		sn = 0;
		cs = 1;
	}
	TMatrix<T> rotMatrix;
	if( m22 >= 0 )
		rotMatrix.Rotate(axis.x, axis.y, axis.z, sn, -cs);
	else
		rotMatrix.Rotate(axis.x, axis.y, axis.z, -sn, cs);
	T x = m03, y = m13, z = m23;
	*this = rotMatrix * this;
	m03 = x;
	m13 = y;
	m23 = z;
}

template <typename T>
void TMatrix<T>::BillboardY(const TVector3<T>& up)
{
	TVector3<T> myYAxis = TVector3<T>(m01, m11, m21);
	myYAxis.Normalize();
	TVector3<T> axis = myYAxis.Cross( up );
	T sn = axis.Normalize();
	T cs = up.Dot( myYAxis );
	if( sn < 0.0001f )
	{
		if( up.z > 0 )
			axis = TVector3<T>(0, up.z, -up.y);
		else
			axis = TVector3<T>(-up.y, up.x, 0);
		axis.Normalize();
		sn = 0;
		cs = 1;
	}
	TMatrix<T> rotMatrix;
	if( up.Dot(TVector3<T>(m02, m12, m22)) >= 0 )
		rotMatrix.Rotate(axis.x, axis.y, axis.z, sn, -cs);
	else
		rotMatrix.Rotate(axis.x, axis.y, axis.z, -sn, cs);
	T x = m03, y = m13, z = m23;
	*this = rotMatrix * this;
	m03 = x;
	m13 = y;
	m23 = z;
}

template <typename T>
void TMatrix<T>::BillboardXY()
{
	TVector3<T> myZAxis = TVector3<T>(m02, m12, m22);
	myZAxis.Normalize();
	TVector3<T> axis(myZAxis.y, -myZAxis.x, 0);
	T sn = axis.Normalize();
	T cs = myZAxis.z;
	if( sn < 0.0001f )
	{
		return;
	}
	TMatrix<T> rotMatrix;
	rotMatrix.Rotate(axis.x, axis.y, axis.z, cs, sn);
	T x = m03, y = m13, z = m23;
	*this = rotMatrix * this;
	m03 = x;
	m13 = y;
	m23 = z;
}

template <typename T>
void TMatrix<T>::BillboardXY(const TVector3<T>& up)
{
	TVector3<T> myZAxis = TVector3<T>(m02, m12, m22);
	myZAxis.Normalize();
	TVector3<T> axis = myZAxis.Cross( up );
	T sn = axis.Normalize();
	T cs = up.Dot( myZAxis );
	if( sn < 0.0001f )
	{
		return;
	}
	TMatrix<T> rotMatrix;
	rotMatrix.Rotate(axis.x, axis.y, axis.z, cs, sn);
	T x = m03, y = m13, z = m23;
	*this = rotMatrix * this;
	m03 = x;
	m13 = y;
	m23 = z;
}

template <typename T>
TQuaternion<T> TMatrix<T>::ToQuaternion () const
{
	T trace = m00 + m11 + m22;
    T root;

    if (trace > 0)
    {
        root = sqrt(trace + 1);
        T root1 = 0.5/root;
		return TQuaternion<T>(0.5*root, (m21 - m12)*root1, (m02 - m20)*root1, (m10 - m01)*root1);
    }
    else
    {
        int i = 0;
        if (m11 > m00)
        {
            i = 1;
			if (m22 > m11)
			{
				i = 2;
			}
        }
		else {
			if (m22 > m00)
			{
				i = 2;
			}
		}
		T ii, jj, kk, ij, ji, jk, kj, ki, ik;
		switch(i) {
		case 0:
			ii = m00;
			jj = m11;
			kk = m22;
			ij = m01;
			ji = m10;
			jk = m12;
			kj = m21;
			ki = m20;
			ik = m02;
			break;
		case 1:
			ii = m11;
			jj = m22;
			kk = m00;
			ij = m12;
			ji = m21;
			jk = m20;
			kj = m02;
			ki = m01;
			ik = m10;
			break;
		case 2:
			ii = m22;
			jj = m00;
			kk = m11;
			ij = m20;
			ji = m02;
			jk = m01;
			kj = m10;
			ki = m12;
			ik = m21;
			break;
		}

        root = sqrt(ii - jj - kk + 1);
        T root1 = 0.5/root;
		switch(i) {
		case 0:
			return TQuaternion<T>((kj - jk)*root1, 0.5*root, (ji + ij)*root1, (ki + ik)*root1);
		case 1:
			return TQuaternion<T>((kj - jk)*root1, (ki + ik)*root1, 0.5*root, (ji + ij)*root1);
		default:
			return TQuaternion<T>((kj - jk)*root1, (ji + ij)*root1, (ki + ik)*root1, 0.5*root);
		}
    }
}

template <typename T>
TEuler<T> TMatrix<T>::ToEuler() const
{
	return ToQuaternion().ToEuler();
}
