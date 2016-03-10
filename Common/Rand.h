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

	static SRand& Inst()
	{
		static SRand g_inst;
		return g_inst;
	}
};