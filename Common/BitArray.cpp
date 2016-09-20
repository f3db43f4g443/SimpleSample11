#include "Common.h"
#include "BitArray.h"

CBitArray::CBitArray( uint32 nBits )
	: m_nBitCount( nBits )
{
	m_bits.resize( ( ( nBits - 1 ) >> 5 ) + 1 );
}

void CBitArray::SetBitCount( uint32 nBits )
{
	m_nBitCount = nBits;
	m_bits.resize( ( ( nBits - 1 ) >> 5 ) + 1 );
}

bool CBitArray::GetBit( uint32 nBit )
{
	if( nBit >= m_nBitCount )
		return false;
	uint32 value = m_bits[nBit >> 5];
	return !!( value & ( 1 << ( nBit & 31 ) ) );
}

void CBitArray::SetBit( uint32 nBit, bool bBit )
{
	if( nBit >= m_nBitCount )
		return;
	uint32& value = m_bits[nBit >> 5];
	if( bBit )
		value |= ( 1 << ( nBit & 31 ) );
	else
		value &= ~( 1 << ( nBit & 31 ) );
}

uint32 CBitArray::GetBits( uint32 nBegin, uint32 nCount )
{
	nBegin = Min( nBegin, m_nBitCount );
	nCount = Min( 32u, Min( nCount, m_nBitCount - nBegin ) );
	if( ( nBegin & 31 ) + nCount > 32 )
	{
		uint64 nValue = *(uint64*)&m_bits[nBegin >> 5];
		return ( nValue >> ( nBegin & 31 ) ) & ( ( 1 << nCount ) - 1 );
	}
	else
	{
		uint32 nValue = m_bits[nBegin >> 5];
		return ( nValue >> ( nBegin & 31 ) ) & ( ( 1 << nCount ) - 1 );
	}
}

void CBitArray::SetBits( uint32 nBegin, uint32 nCount, uint32 nValue )
{
	nBegin = Min( nBegin, m_nBitCount );
	nCount = Min( 32u, Min( nCount, m_nBitCount - nBegin ) );
	if( ( nBegin & 31 ) + nCount > 32 )
	{
		uint64& nValue = *(uint64*)&m_bits[nBegin >> 5];
		uint64 nMask = ( ( (uint64)1 << nCount ) - 1 ) << ( nBegin & 31 );
		nValue = ( nValue & ~nMask ) | ( ( nValue << ( nBegin & 31 ) ) & nMask );
	}
	else
	{
		uint32& nValue = m_bits[nBegin >> 5];
		uint32 nMask = ( ( 1 << nCount ) - 1 ) << ( nBegin & 31 );
		nValue = ( nValue & ~nMask ) | ( ( nValue << ( nBegin & 31 ) ) & nMask );
	}
}