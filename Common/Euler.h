#pragma once
#include "Matrix.h"
#include "Quaternion.h"
#include <math.h>
const double PI = 3.1415926535897932384626433832795;
template <typename T>
class TEuler
{
public:
	TEuler();
	TEuler(const TEuler& e);
	TEuler(T h, T p, T r);

	TEuler& operator= (const TEuler& e);
	bool operator== (const TEuler& e) const;
    TEuler operator+ (const TEuler& e) const;
    TEuler operator- (const TEuler& e) const;
    TEuler operator* (T scalar) const;

	void normalize();

	TMatrix<T> ToMatrix() const;
	TQuaternion<T> ToQuaternion() const;
private:
	T h, p, r;
};

