#pragma once
#include "Rand.h"

float randFloat();
float randFloat(float min, float max);
float randFloat1();
float randFloat1(float min, float max);

uint32 Pow2Ceil( uint32 n );
float NormalizeAngle( float f );

uint32 ZCurveOrder( uint16 x, uint16 y );
void ZCurveOrderInv( uint32 nZCurveOrder, uint16& x, uint16& y );