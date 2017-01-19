#pragma once

template <typename T>
class TVector4
{
public:
	TVector4();
	TVector4(T x, T y, T z, T w);
	TVector4(const TVector4& v);
	~TVector4();
	
    TVector4& operator= (const TVector4& vec);
    bool operator== (const TVector4& vec) const;
    TVector4 operator+ (const TVector4& vec) const;
    TVector4 operator- (const TVector4& vec) const;
	TVector4 operator* (T scalar) const;
	TVector4 operator* (const TVector4& vec) const;
	TVector4 operator/ ( T scalar ) const;
	
    T Length () const;
    T Dot (const TVector4& vec) const;
	TVector4 Cross (const TVector4& vec) const;
    T Normalize ();

	T x, y, z, w;
};