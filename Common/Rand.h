#pragma once
#include <vector>
using namespace std;

struct SRand
{
	uint32 nSeed;

	uint32 Rand()
	{
		return( ( (nSeed = nSeed * 214013L + 2531011L ) ) & 0x7fff );
	}

	template<typename T>
	T Rand( T randMin, T randMax )
	{
		return Rand() * ( randMax - randMin ) / ( 0x7fff + 1 ) + randMin;
	}

	template<typename T>
	void Shuffle( T* pElems, uint32 nSize )
	{
		for( uint32 i = nSize; i > 1; i-- )
		{
			uint32 a = i - 1;
			uint32 b = Rand<uint32>( 0, i );
			if( a != b )
			{
				T temp = pElems[a];
				pElems[a] = pElems[b];
				pElems[b] = temp;
			}
		}
	}

	template<typename T>
	void Shuffle( vector<T>& vecElems )
	{
		if( vecElems.size() )
			Shuffle( &vecElems[0], vecElems.size() );
	}

	void C( uint32 a, uint32 b, int8* result )
	{
		memset( result, 1, a );
		memset( result + a, 0, b - a );
		Shuffle( result, b );
	}

	void A( uint32 a, uint32 b, uint32* result )
	{
		int32* arr = (int32*)alloca( b * sizeof( int32 ) );
		for( int i = 0; i < b; i++ )
		{
			arr[i] = i;
		}
		for( uint32 i = 0; i < a; i++ )
		{
			uint32 m = i;
			uint32 n = Rand<uint32>( i, b );
			result[i] = arr[n];
			arr[n] = arr[m];
		}
	}

	static SRand& Inst()
	{
		static SRand g_inst;
		return g_inst;
	}
};