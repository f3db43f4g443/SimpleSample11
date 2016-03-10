#include "Common.h"

template <typename T>
TEuler<T>::TEuler()
{
}
template <typename T>
TEuler<T>::TEuler(const TEuler<T>& e):h(e.h), p(e.p), r(e.r)
{
}
template <typename T>
TEuler<T>::TEuler(T h, T p, T r):h(h), p(p), r(r)
{
}

template <typename T>
TEuler<T>& TEuler<T>::operator= (const TEuler<T>& e)
{
	h = e.h;
	p = e.p;
	r = e.r;
	return *this;
}
template <typename T>
bool TEuler<T>::operator== (const TEuler<T>& e) const
{
	return h == e.h && p == e.p && r == e.r;
}
template <typename T>
TEuler<T> TEuler<T>::operator+ (const TEuler<T>& e) const
{
	return TEuler<T>(h + e.h, p + e.p, r + e.r);
}
template <typename T>
TEuler<T> TEuler<T>::operator- (const TEuler<T>& e) const
{
	return TEuler<T>(h - e.h, p - e.p, r - e.r);
}
template <typename T>
TEuler<T> TEuler<T>::operator* (T scalar) const
{
	return TEuler<T>(h * scalar, p * scalar, r * scalar);
}

template <typename T>
void TEuler<T>::normalize()
{
	p -= floor(p / PI / 2 + 0.5) * PI * 2;
	if(p == PI * 0.5 || p == -PI * 0.5) {
		h += r;
		r = 0;
	}
	if(p > PI * 0.5){p = PI - p; h += PI; r += PI;}
	else if(p < -PI * 0.5){p = -PI - p; h += PI; r += PI;}
	h -= floor(h / (PI * 2) + 0.5) * PI * 2;
	r -= floor(r / (PI * 2) + 0.5) * PI * 2;
}

template <typename T>
TMatrix<T> TEuler<T>::ToMatrix() const
{
	TMatrix<T> matH, matP, matR;
	matH.Rotate(0, 1, 0, h);
	matP.Rotate(1, 0, 0, p);
	matR.Rotate(0, 0, 1, r);
	return matH * matP * matR;
}
template <typename T>
TQuaternion<T> TEuler<T>::ToQuaternion() const
{
	return ToMatrix().ToQuaternion();
}
