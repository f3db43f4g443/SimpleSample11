#pragma once

#include "Vector2.h"
#include "Matrix2D.h"
#include "Transform2D.h"
#include "Rectangle.h"
#include "BoundBox.h"
#include "Frustum.h"

#include "Vector2.inl"
#include "Vector3.inl"
#include "Vector4.inl"
#include "Matrix2D.inl"
#include "Rectangle.inl"
#include "Matrix.inl"
#include "Quaternion.inl"
#include "Euler.inl"
#include "BoundBox.inl"
#include "Frustum.inl"

typedef TVector2<float> CVector2;
typedef TVector3<float> CVector3;
typedef TVector4<float> CVector4;
typedef TMatrix2D<float> CMatrix2D;
typedef TTransform2D<float> CTransform2D;
typedef TRectangle<float> CRectangle;
typedef TMatrix<float> CMatrix;
typedef TQuaternion<float> CQuaternion;
typedef TEuler<float> CEuler;
typedef TBoundBox<float> CBoundBox;
typedef TFrustum<float> CFrustum;

typedef TVector3<double> CVector3d;
typedef TVector4<double> CVector4d;
typedef TMatrix<double> CMatrixd;
typedef TMatrix2D<double> CMatrix2Dd;
typedef TRectangle<double> CRectangled;
typedef TQuaternion<double> CQuaterniond;
typedef TEuler<double> CEulerd;
typedef TBoundBox<double> CBoundBoxd;
typedef TFrustum<double> CFrustumd;
