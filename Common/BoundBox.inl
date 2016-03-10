#include "Common.h"

template <typename T>
TBoundBox<T>::TBoundBox()
{
}

template <typename T>
TBoundBox<T>::~TBoundBox(void)
{
}

template <typename T>
void TBoundBox<T>::Set(T x1, T y1, T z1, T x2, T y2, T z2)
{
	m_x1 = x1;
	m_y1 = y1; 
	m_z1 = z1;
	m_x2 = x2;
	m_y2 = y2;
	m_z2 = z2;
}

template <typename T>
void TBoundBox<T>::GetAABB(const TMatrix<T>& transform, T& x1, T& y1, T& z1, T& x2, T& y2, T& z2)
{
	x1 = x2 = transform.m03;
	if(transform.m00 > 0) {x1 += m_x1 * transform.m00; x2 += m_x2 * transform.m00;}
	else {x1 += m_x2 * transform.m00; x2 += m_x1 * transform.m00;}
	if(transform.m01 > 0) {x1 += m_y1 * transform.m01; x2 += m_y2 * transform.m01;}
	else {x1 += m_y2 * transform.m01; x2 += m_y1 * transform.m01;}
	if(transform.m02 > 0) {x1 += m_z1 * transform.m02; x2 += m_z2 * transform.m02;}
	else {x1 += m_z2 * transform.m02; x2 += m_z1 * transform.m02;}
	
	y1 = y2 = transform.m13;
	if(transform.m10 > 0) {y1 += m_x1 * transform.m10; y2 += m_x2 * transform.m10;}
	else {y1 += m_x2 * transform.m10; y2 += m_x1 * transform.m10;}
	if(transform.m11 > 0) {y1 += m_y1 * transform.m11; y2 += m_y2 * transform.m11;}
	else {y1 += m_y2 * transform.m11; y2 += m_y1 * transform.m11;}
	if(transform.m12 > 0) {y1 += m_z1 * transform.m12; y2 += m_z2 * transform.m12;}
	else {y1 += m_z2 * transform.m12; y2 += m_z1 * transform.m12;}
	
	z1 = z2 = transform.m23;
	if(transform.m20 > 0) {z1 += m_z1 * transform.m20; z2 += m_z2 * transform.m20;}
	else {z1 += m_z2 * transform.m20; z2 += m_z1 * transform.m20;}
	if(transform.m21 > 0) {z1 += m_y1 * transform.m21; z2 += m_y2 * transform.m21;}
	else {z1 += m_y2 * transform.m21; z2 += m_y1 * transform.m21;}
	if(transform.m22 > 0) {z1 += m_z1 * transform.m22; z2 += m_z2 * transform.m22;}
	else {z1 += m_z2 * transform.m22; z2 += m_z1 * transform.m22;}
}