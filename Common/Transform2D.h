#pragma once
#include "Matrix2D.h"
#include "Euler.h"

template <typename T>
class TTransform2D
{
public:
	TTransform2D() {}
	TTransform2D( const TTransform2D& transform )
		: x( transform.x ), y( transform.y ), r( transform.r ), sx( transform.sx ), sy( transform.sy ) {}
	TTransform2D( T x, T y, T r, T sx, T sy ) : x( x ), y( y ), r( r ), sx( sx ), sy( sy ) {}

	TTransform2D& operator= ( const TTransform2D& transform )
	{
		x = transform.x;
		y = transform.y;
		r = transform.r;
		sx = transform.sx;
		sy = transform.sy;
		return *this;
	}

	TMatrix2D<T> ToMatrix() const
	{
		TMatrix2D<T> mat;
		mat.Transform( x, y, r, sx, sy );
		return mat;
	}

	TTransform2D& Lerp( const TTransform2D& a, const TTransform2D& b, T t, int32 nRotate = 0 )
	{
		T invt = 1 - t;
		x = a.x * invt + b.x * t;
		y = a.y * invt + b.y * t;
		sx = a.sx * invt + b.sx * t;
		sy = a.sy * invt + b.sy * t;

		T dr = ( b.r - a.r + PI ) / ( PI * 2 );
		dr = ( dr - floor( dr ) + nRotate ) * ( PI * 2 ) - PI;
		r = a.r + dr * t;
		return *this;
	}

	T x, y, r, sx, sy;
};