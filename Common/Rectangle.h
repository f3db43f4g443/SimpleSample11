#pragma once
#include "Matrix2D.h"

template <typename T>
class TRectangle
{
public:
	TRectangle();
	TRectangle( T x, T y, T width, T height );
	TRectangle( TVector2<T> vMin, TVector2<T> vMax );
	TRectangle( const TRectangle& rect );

	TRectangle& operator= (const TRectangle& rect);
    bool operator== (const TRectangle& rect) const;
	bool operator!= ( const TRectangle& rect ) const { return !( *this == rect ); }
	TRectangle operator+ (const TRectangle& rect) const;
	TRectangle operator* (const TRectangle& rect) const;
	TRectangle operator* (const T& t) const;
	TRectangle operator* (const TVector2<T>& vec) const;
	TRectangle operator* (const TMatrix2D<T>& mat) const;
	TRectangle operator/ ( const T& t ) const;

	T GetLeft() const;
	T GetTop() const;
	T GetRight() const;
	T GetBottom() const;
	void SetLeft( T value );
	void SetTop( T value );
	void SetRight( T value );
	void SetBottom( T value );

	T GetCenterX() const;
	T GetCenterY() const;
	TVector2<T> GetCenter() const;
	T GetSizeX() const;
	T GetSizeY() const;
	TVector2<T> GetSize() const;
	void SetCenterX( T value );
	void SetCenterY( T value );
	void SetCenter( TVector2<T> value );
	void SetSizeX( T value );
	void SetSizeY( T value );
	void SetSize( TVector2<T> value );

	bool Contains( const TVector2<T>& ptr ) const;
	bool Contains( const TRectangle<T>& rect ) const;
	TRectangle Offset( const TVector2<T>& ofs ) const;
	TRectangle Rotate( T r );
	TRectangle RotateByCenter( T r );
	TRectangle Scale( T s );
	TRectangle Scale( const TVector2<T>& s );

	TVector2<T> GetTexCoord( const TVector2<T>& s ) const;

	T x, y, width, height;
};
