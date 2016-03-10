#pragma once
template <typename T>
class TMatrix;
template <typename T>
class TEuler;
template <typename T>
class TQuaternion
{
public:
	TQuaternion();
	TQuaternion(T w, T x, T y, T z);
	TQuaternion(const TQuaternion& q);

	TQuaternion& operator= (const TQuaternion& q);
	bool operator== (const TQuaternion& q) const;
	TQuaternion operator+ (const TQuaternion& q) const;
	TQuaternion operator- (const TQuaternion& q) const;
	TQuaternion operator* (const TQuaternion& q) const;
	TQuaternion operator* (T scalar) const;
	
	T Length () const;  // length of 4-tuple
	T Dot (const TQuaternion& q) const;  // dot product of 4-tuples
	T Normalize ();
	TQuaternion Inverse () const;  // apply to non-zero quaternion
	TQuaternion Conjugate () const;  // negate x, y, and z terms
	TQuaternion Exp () const;  // apply to quaternion with w = 0
	TQuaternion Log () const;  // apply to unit-length quaternion
	TQuaternion& Slerp (T t, const TQuaternion& p, const TQuaternion& q);

	TMatrix<T> ToMatrix() const;
	TEuler<T> ToEuler() const;

	T w, x, y, z;
};

