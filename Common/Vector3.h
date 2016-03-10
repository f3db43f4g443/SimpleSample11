#pragma once

template <typename T>
class TVector3
{
public:
	TVector3();
	TVector3(T x, T y, T z);
	TVector3(const TVector3& v);
	~TVector3();
	
    TVector3& operator= (const TVector3& vec);
    bool operator== (const TVector3& vec) const;
    TVector3 operator+ (const TVector3& vec) const;
    TVector3 operator- (const TVector3& vec) const;
	TVector3 operator* (T scalar) const;
	TVector3 operator* (const TVector3& vec) const;
	
    T Length () const;
    T Dot (const TVector3& vec) const;
	TVector3 Cross (const TVector3& vec) const;
    T Normalize ();

	TVector3& Slerp(T t, const TVector3& p, const TVector3& q);

	T x, y, z;
};