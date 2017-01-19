#pragma once

template <typename T>
class TVector2
{
public:
	TVector2();
	TVector2(T x, T y);
	TVector2(const TVector2& v);
	~TVector2();

	TVector2& operator= (const TVector2& vec);
    bool operator== (const TVector2& vec) const;
	bool operator!= ( const TVector2& vec ) const;
    TVector2 operator+ (const TVector2& vec) const;
    TVector2 operator- (const TVector2& vec) const;
	TVector2 operator* (T scalar) const;
	TVector2 operator* (const TVector2& vec) const;
	TVector2 operator/ ( T scalar ) const;
	TVector2 operator/ ( const TVector2& vec ) const;

    T Length () const;
	T Length2 () const;
    T Dot (const TVector2& vec) const;
    T Normalize ();

	T x, y;
};
