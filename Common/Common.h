#pragma once

#include <math.h>
#include <stdlib.h>
#include <string.h>
#ifdef _DEBUG
#include <stdio.h>
#endif
#include <float.h>

typedef char int8;
typedef unsigned char uint8;
typedef short int16;
typedef unsigned short uint16;
typedef int int32;
typedef unsigned int uint32;
typedef long long int64;
typedef unsigned long long uint64;

#define INVALID_8BITID 0xff
#define INVALID_16BITID 0xffff
#define INVALID_32BITID 0xffffffff
#define INVALID_64BITID 0xffffffffffffffffll

#define MAX( a, b ) ((a) > (b)? (a): (b))
#define MIN( a, b ) ((a) < (b)? (a): (b))

#define ELEM_COUNT( a ) ( sizeof( a ) / sizeof( (a)[0] ) )

template<typename T>
T Min( T a, T b )
{
	return a < b ? a : b;
}
template<typename T>
T Max( T a, T b )
{
	return a > b ? a : b;
}

#define DECLARE_GLOBAL_INST_REFERENCE( className ) \
static className& Inst() \
{ \
	static className g_inst; \
	return g_inst; \
}

#define DECLARE_GLOBAL_INST_POINTER( className ) \
static className* Inst() \
{ \
	static className g_inst; \
	return &g_inst; \
}