#pragma once
#include "Vector2.h"
#include "Vector3.h"

template <typename T>
class TMatrix2D
{
public:
	TMatrix2D ();
	TMatrix2D (const TMatrix2D& mat);
	TMatrix2D (
        T m00, T m01, T m02,
        T m10, T m11, T m12,
        T m20, T m21, T m22);

	TMatrix2D& operator= (const TMatrix2D& mat);
	TMatrix2D operator+ (const TMatrix2D& mat) const;
	TMatrix2D operator- (const TMatrix2D& mat) const;
	TMatrix2D operator* (T scalar) const;

	bool operator== ( const TMatrix2D& mat );
	bool operator!= ( const TMatrix2D& mat );

    TVector3<T> operator* (const TVector3<T>& vec) const;
    TMatrix2D operator* (const TMatrix2D& mat) const;

	TVector2<T> MulVector2Pos(const TVector2<T>& vec) const;
	TVector2<T> MulVector2Dir(const TVector2<T>& vec) const;
	TVector2<T> MulTVector2PosNoScale(const TVector2<T>& vec) const;
	TVector2<T> MulTVector2DirNoScale( const TVector2<T>& vec ) const;
	TVector2<T> MulTVector2Pos( const TVector2<T>& vec ) const;
	TVector2<T> MulTVector2Dir( const TVector2<T>& vec ) const;

	TVector2<T> GetPosition() const;
	void SetPosition( TVector2<T> pos );

	T* GetMatrix() { return &m00; }

	void Identity();
	void Transform(T x, T y, T angle, T s);
	void Transform(T x, T y, T angle, T sx, T sy);
	void Translate(T x, T y);
	void Rotate(T angle);
	void Scale(T s);
	void Decompose(T& x, T& y, T& angle, T& s) const;
	void Decompose(T& x, T& y, T& angle, T& sx, T& sy) const;
	TMatrix2D Inverse() const;
	TMatrix2D InverseNoScale() const;

	FORCE_INLINE static const TMatrix2D<T>& GetIdentity()
	{
		static TMatrix2D<T> g_mat( 1, 0, 0, 0, 1, 0, 0, 0, 1 );
		return g_mat;
	}

	T m00,m10,m20,m01,m11,m21,m02,m12,m22;
};
