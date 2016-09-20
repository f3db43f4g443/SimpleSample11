#pragma once
#include <vector>
using namespace std;

class CBitArray
{
public:
	CBitArray( uint32 nBits = 0 );
	void SetBitCount( uint32 nBits );

	void Clear() { memset( &m_bits[0], 0, sizeof( uint32 ) * m_bits.size() ); }
	bool GetBit( uint32 nBit );
	void SetBit( uint32 nBit, bool bBit );
	uint32 GetBits( uint32 nBegin, uint32 nCount );
	void SetBits( uint32 nBegin, uint32 nCount, uint32 nValue );
private:
	vector<uint32> m_bits;
	uint32 m_nBitCount;
};