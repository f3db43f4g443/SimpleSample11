#pragma once
#include "Matrix.h"

template <typename T>
class TBoundBox
{
public:
	TBoundBox();
	TBoundBox(T x1, T y1, T z1, T x2, T y2, T z2):m_x1(x1), m_y1(y1), m_z1(z1),
		m_x2(x2), m_y2(y2), m_z2(z2){}
	~TBoundBox(void);

	void Set(T x1, T y1, T z1, T x2, T y2, T z2);

	void GetAABB(const TMatrix<T>& transform, T& x1, T& y1, T& z1, T& x2, T& y2, T& z2);
private:
	T m_x1, m_y1, m_z1, m_x2, m_y2, m_z2;
};

