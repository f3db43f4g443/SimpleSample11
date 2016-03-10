#pragma once

struct SCurveKey
{
	uint32 nTimes;
	float* fTimes;

	uint32 GetKeyIndex( float fTime, float& t );
};

template <typename T>
struct TCurve
{
	struct SVertex
	{
		T value;
		T tangentIn;
		T tangentOut;
		bool bSmooth;
	};

	struct SSegment
	{
		uint8 nItems;
		T a[4];
	};

	uint32 nVertices;
	SVertex* vertices;
	SSegment* segments;
	void CalcSegments( const SCurveKey& key );
	T GetValue( uint32 nIndex, float t );
};

template <typename T>
void TCurve<T>::CalcSegments( const SCurveKey& key )
{
	if( nVertices <= 1 )
		return;

	for( int i = 1; i < nVertices; i++ )
	{
		float fTime = key.fTimes[i] - key.fTimes[i - 1];
		float fInvTime = 1.0f / fTime;
		SSegment& segment = segments[i - 1];
		T& x0 = vertices[i - 1].value;
		T& x1 = vertices[i].value;
		T& t0 = vertices[i - 1].tangentOut;
		T& t1 = vertices[i].tangentIn;
		if( vertices[i - 1].bSmooth && vertices[i].bSmooth )
		{
			segment.nItems = 4;
			segment.a[3] = x0;
			segment.a[2] = t0;
			segment.a[1] = ( ( x1 - x0 ) * 3 * fInvTime - t1 - t0 * 2 ) * fInvTime;
			segment.a[0] = ( t1 + t0 + ( x0 - x1 ) * 2 * fInvTime ) * fInvTime * fInvTime;
		}
		else if( vertices[i - 1].bSmooth )
		{
			segment.nItems = 3;
			segment.a[2] = x0;
			segment.a[1] = t0;
			segment.a[0] = ( ( x1 - x0 ) * fInvTime - t0 ) * fInvTime;
		}
		else if( vertices[i].bSmooth )
		{
			segment.nItems = 3;
			segment.a[2] = x0;
			segment.a[1] = ( x1 - x0 ) * 2 * fInvTime - t1;
			segment.a[0] = ( t1 + ( x0 - x1 ) * fInvTime ) * fInvTime;
		}
		else
		{
			segment.nItems = 2;
			segment.a[1] = x0;
			segment.a[0] = ( x1 - x0 ) * fInvTime;
		}
	}
}

template <typename T>
T TCurve<T>::GetValue( uint32 nIndex, float t )
{
	if( nIndex == 0 )
		return vertices[0].value;
	if( nIndex == nVertices )
		return vertices[nVertices - 1].value;
	SSegment& segment = segments[nIndex - 1];
	T value = segment.a[0];
	for( int i = 1; i < segment.nItems; i++ )
	{
		value = value * t + segment.a[i];
	}
	return value;
}