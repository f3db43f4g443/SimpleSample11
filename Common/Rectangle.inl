#include "Common.h"

template <typename T>
FORCE_INLINE TRectangle<T>::TRectangle()
{
}
template <typename T>
FORCE_INLINE TRectangle<T>::TRectangle( T x, T y, T width, T height ):x(x), y(y), width(width), height(height)
{
}
template <typename T>
FORCE_INLINE TRectangle<T>::TRectangle( TVector2<T> vMin, TVector2<T> vMax ) : x( vMin.x ), y( vMin.y ), width( vMax.x - vMin.x ), height( vMax.y - vMin.y )
{

}
template <typename T>
FORCE_INLINE TRectangle<T>::TRectangle(const TRectangle<T>& rect):x(rect.x), y(rect.y), width(rect.width), height(rect.height)
{
}

template <typename T>
FORCE_INLINE TRectangle<T>& TRectangle<T>::operator= (const TRectangle<T>& rect)
{
	x = rect.x;
	y = rect.y;
	width = rect.width;
	height = rect.height;
	return *this;
}
template <typename T>
FORCE_INLINE bool TRectangle<T>::operator== (const TRectangle<T>& rect) const
{
	return x == rect.x && y == rect.y && width == rect.width && height == rect.height;
}
template <typename T>
FORCE_INLINE TRectangle<T> TRectangle<T>::operator+ (const TRectangle<T>& rect) const
{
	T right = Max( x + width, rect.x + rect.width );
	T bottom = Max( y + height, rect.y + rect.height );
	T x1 = Min( x, rect.x );
	T y1 = Min( y, rect.y );
	T width1 = right - x1;
	T height1 = bottom - y1;
	return TRectangle<T>( x1, y1, width1, height1 );
}
template <typename T>
FORCE_INLINE TRectangle<T> TRectangle<T>::operator* (const TRectangle<T>& rect) const
{
	T right = Min( x + width, rect.x + rect.width );
	T bottom = Min( y + height, rect.y + rect.height );
	T x1 = Max( x, rect.x );
	T y1 = Max( y, rect.y );
	T width1 = right - x1;
	T height1 = bottom - y1;
	if( width1 < 0 ) width1 = 0;
	if( height1 < 0 ) height1 = 0;
	return TRectangle<T>( x1, y1, width1, height1 );
}

template <typename T>
FORCE_INLINE TRectangle<T> TRectangle<T>::operator* ( const T& t ) const
{
	return TRectangle<T>( x * t, y * t, width * t, height * t );
}
template <typename T>
FORCE_INLINE TRectangle<T> TRectangle<T>::operator* ( const TVector2<T>& vec ) const
{
	return TRectangle<T>( x * vec.x, y * vec.y, width * vec.x, height * vec.y );
}

template <typename T>
FORCE_INLINE TRectangle<T> TRectangle<T>::operator* (const TMatrix2D<T>& mat) const
{
	T left = 0, right = 0, top = 0, bottom = 0;
	if( mat.m00 > 0 )
	{
		left += mat.m00 * GetLeft();
		right += mat.m00 * GetRight();
	}
	else
	{
		left += mat.m00 * GetRight();
		right += mat.m00 * GetLeft();
	}
	if( mat.m01 > 0 )
	{
		left += mat.m01 * GetTop();
		right += mat.m01 * GetBottom();
	}
	else
	{
		left += mat.m01 * GetBottom();
		right += mat.m01 * GetTop();
	}
	if( mat.m10 > 0 )
	{
		top += mat.m10 * GetLeft();
		bottom += mat.m10 * GetRight();
	}
	else
	{
		top += mat.m10 * GetRight();
		bottom += mat.m10 * GetLeft();
	}
	if( mat.m11 > 0 )
	{
		top += mat.m11 * GetTop();
		bottom += mat.m11 * GetBottom();
	}
	else
	{
		top += mat.m11 * GetBottom();
		bottom += mat.m11 * GetTop();
	}

	return TRectangle<T>( left + mat.m02, top + mat.m12, right - left, bottom - top );
}

template <typename T>
FORCE_INLINE T TRectangle<T>::GetLeft() const { return x; }
template <typename T>
FORCE_INLINE T TRectangle<T>::GetTop() const { return y; }
template <typename T>
FORCE_INLINE T TRectangle<T>::GetRight() const { return x + width; }
template <typename T>
FORCE_INLINE T TRectangle<T>::GetBottom() const { return y + height; }
template <typename T>
FORCE_INLINE void TRectangle<T>::SetLeft( T value ) { width += x - value; x = value; }
template <typename T>
FORCE_INLINE void TRectangle<T>::SetTop( T value ) { height += y - value; y = value; }
template <typename T>
FORCE_INLINE void TRectangle<T>::SetRight( T value ) { width = value - x; }
template <typename T>
FORCE_INLINE void TRectangle<T>::SetBottom( T value ) { height = value - y; }

template <typename T>
FORCE_INLINE T TRectangle<T>::GetCenterX() const { return x + width / 2; }
template <typename T>
FORCE_INLINE T TRectangle<T>::GetCenterY() const { return y + height / 2; }
template <typename T>
FORCE_INLINE TVector2<T> TRectangle<T>::GetCenter() const { return TVector2<T>( GetCenterX(), GetCenterY() ); }
template <typename T>
FORCE_INLINE T TRectangle<T>::GetSizeX() const { return width; }
template <typename T>
FORCE_INLINE T TRectangle<T>::GetSizeY() const { return height; }
template <typename T>
FORCE_INLINE TVector2<T> TRectangle<T>::GetSize() const { return TVector2<T>( width, height ); }
template <typename T>
FORCE_INLINE void TRectangle<T>::SetCenterX( T value ) { x = value - width / 2; }
template <typename T>
FORCE_INLINE void TRectangle<T>::SetCenterY( T value ) { y = value - height / 2; }
template <typename T>
FORCE_INLINE void TRectangle<T>::SetCenter( TVector2<T> value ) { SetCenterX( value.x ); SetCenterY( value.y ); }
template <typename T>
FORCE_INLINE void TRectangle<T>::SetSizeX( T value ) { x -= ( value - width ) / 2; width = value; }
template <typename T>
FORCE_INLINE void TRectangle<T>::SetSizeY( T value ) { y -= ( value - height ) / 2; height = value; }
template <typename T>
FORCE_INLINE void TRectangle<T>::SetSize( TVector2<T> value ) { SetSizeX( value.x ); SetSizeY( value.y ); }

template <typename T>
FORCE_INLINE bool TRectangle<T>::Contains( const TVector2<T>& ptr ) const { return ptr.x >= x && ptr.x <= x + width && ptr.y >= y && ptr.y <= y + height; }
template <typename T>
FORCE_INLINE bool TRectangle<T>::Contains( const TRectangle<T>& rect ) const { return rect.x >= x && rect.x + rect.width <= x + width && rect.y >= y && rect.y + rect.height <= y + height; }
template <typename T>
FORCE_INLINE TRectangle<T> TRectangle<T>::Offset( const TVector2<T>& ofs ) const { return TRectangle<T>( x + ofs.x, y + ofs.y, width, height ); }
template <typename T>
FORCE_INLINE TRectangle<T> TRectangle<T>::Scale( T s ) { TRectangle<T> rect = *this; rect.SetSizeX( width * s ); rect.SetSizeY( height * s ); return rect; }
template <typename T>
FORCE_INLINE TRectangle<T> TRectangle<T>::Scale( const TVector2<T>& s ) { TRectangle<T> rect = *this; rect.SetSizeX( width * s.x ); rect.SetSizeY( height * s.y ); return rect; }

template <typename T>
FORCE_INLINE TVector2<T> TRectangle<T>::GetTexCoord( const TVector2<T>& s ) const
{
	TVector2<T> vec( ( s.x - x ) / width, 1.0f - ( s.y - y ) / height );
	return vec;
}