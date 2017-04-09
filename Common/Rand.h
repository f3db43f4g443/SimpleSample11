#pragma once

struct SRand
{
	uint32 nSeed;

	uint32 Rand()
	{
		return( ( (nSeed = nSeed * 214013L + 2531011L ) >> 16 ) & 0x7fff );
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

	void C( uint32 a, uint32 b, int8* result )
	{
		memset( result, 1, a );
		memset( result + a, 0, b - a );
		Shuffle( result, b );
	}

	void C1( uint32 a, uint32 b, uint32* result )
	{
		int8* arr = (int8*)alloca( b );
		memset( arr, 1, a );
		memset( arr + a, 0, b - a );
		Shuffle( arr, b );
		int iRes = 0;
		for( int i = 0; i < b; i++ )
		{
			if( arr[i] )
				result[iRes++] = i;
		}
	}

	static SRand& Inst()
	{
		static SRand g_inst;
		return g_inst;
	}
};