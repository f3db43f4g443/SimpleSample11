#pragma once
#include "Vector3.h"
#include "Vector4.h"
#include "Quaternion.h"
#include "Euler.h"

template <typename T>
class TMatrix
{
public:
	TMatrix ();
    TMatrix (const TMatrix& mat);
    TMatrix (
        T m00, T m01, T m02, T m03,
        T m10, T m11, T m12, T m13,
        T m20, T m21, T m22, T m23,
        T m30, T m31, T m32, T m33);

	TMatrix& operator= (const TMatrix& mat);
    TMatrix operator+ (const TMatrix& mat) const;
    TMatrix operator- (const TMatrix& mat) const;
    TMatrix operator* (T scalar) const;
	
    TVector4<T> operator* (const TVector4<T>& vec) const;
    TMatrix operator* (const TMatrix& mat) const;
	
    TMatrix Transpose ();
    TMatrix Inverse ();

	TVector3<T> MulVector3Pos(const TVector3<T> vec);
	TVector3<T> MulVector3Dir(const TVector3<T> vec);

	TVector3<T> GetPosition();

	void Identity();
	void Transform(T x, T y, T z, T ax, T ay, T az, T angle, T sx, T sy, T sz);
	void Translate(T x, T y, T z);
	void Rotate(T ax, T ay, T az, T angle);
	void Rotate(T ax, T ay, T az, T cs, T sn);
	void Scale(T sx, T sy, T sz);
	void PerspectiveProjection (T originx, T originy, T originz,
        T normalx, T normaly, T normalz, T eyex, T eyey, T eyez);

	void Decompose(T& x, T& y, T& z, T& ax, T& ay, T& az, T& angle,
		T& sx, T& sy, T& sz);

	void BillboardX();
	void BillboardX(const TVector3<T>& up);
	void BillboardY();
	void BillboardY(const TVector3<T>& up);
	void BillboardXY();
	void BillboardXY(const TVector3<T>& up);

	T* GetMatrix(){return &m00;}

	TQuaternion<T> ToQuaternion () const;
	TEuler<T> ToEuler() const;

	T m00,m10,m20,m30,m01,m11,m21,m31,m02,m12,m22,m32,m03,m13,m23,m33;
};
