#pragma once
#include "Matrix2D.h"

template <typename T>
class TRectangle
{
public:
	TRectangle();
	TRectangle( T x, T y, T width, T height );
	TRectangle( const TRectangle& rect );

	TRectangle& operator= (const TRectangle& rect);
    bool operator== (const TRectangle& rect) const;
	TRectangle operator+ (const TRectangle& rect) const;
	TRectangle operator* (const TRectangle& rect) const;
	TRectangle operator* (const T& t) const;
	TRectangle operator* (const TVector2<T>& vec) const;
	TRectangle operator* (const TMatrix2D<T>& mat) const;

	T GetLeft() const { return x; }
	T GetTop() const { return y; }
	T GetRight() const { return x + width; }
	T GetBottom() const { return y + height; }
	void SetLeft( T value ) { width += x - value; x = value; }
	void SetTop( T value ) { height += y - value; y = value; }
	void SetRight( T value ) { width = value - x; }
	void SetBottom( T value ) { height = value - y; }

	T GetCenterX() const { return x + width / 2; }
	T GetCenterY() const { return y + height / 2; }
	T GetSizeX() const { return width; }
	T GetSizeY() const { return height; }
	TVector2<T> GetSize() const { return TVector2<T>( width, height ); }
	void SetCenterX( T value ) { x = value - width / 2; }
	void SetCenterY( T value ) { y = value - height / 2; }
	void SetSizeX( T value ) { x -= ( value - width ) / 2; width = value; }
	void SetSizeY( T value ) { y -= ( value - height ) / 2; height = value; }

	bool Contains( const TVector2<T>& ptr ) const { return ptr.x >= x && ptr.x <= x + width && ptr.y >= y && ptr.y <= y + height; }
	TRectangle Offset( const TVector2<T>& ofs ) { return TRectangle<T>( x + ofs.x, y + ofs.y, width, height ); }
	TRectangle Scale( T s ) { TRectangle<T> rect = *this; rect.SetSizeX( width * s ); rect.SetSizeY( height * s ); return rect; }
	TRectangle Scale( const TVector2<T>& s ) { TRectangle<T> rect = *this; rect.SetSizeX( width * s.x ); rect.SetSizeY( height * s.y ); return rect; }

	TVector2<T> GetTexCoord( const TVector2<T>& s ) const;

	T x, y, width, height;
};
