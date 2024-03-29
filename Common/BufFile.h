#pragma once
#include <vector>
#include <string>
#include "StringUtil.h"
using namespace std;

class IBufReader
{
public:
	IBufReader() : m_nBufferLen( 0 ), m_nCurPos( 0 ) {}
	IBufReader( const IBufReader& a ) : m_nBufferLen( a.m_nBufferLen ), m_nCurPos( a.m_nCurPos ) {}
	IBufReader& operator = ( const IBufReader& a ) { m_nBufferLen = a.m_nBufferLen; m_nCurPos = a.m_nCurPos; return *this; }
	virtual const void* GetBuffer() const = 0;

	uint32 Read( void* pBuf, uint32 nMaxLen );
	uint32 Read( string& t, uint32 nMaxLen );
	uint32 Read( CString& t, uint32 nMaxLen );
	template <typename T>
	uint32 Read( T& t )
	{
		return Read( &t, sizeof( T ) );
	}
	template <typename T>
	uint32 Read( vector<T>& t )
	{
		uint32 nLen;
		uint32 nCount;
		nLen = Read( nCount );
		if( nLen < sizeof( uint32 ) )
			return nLen;
		if( !nCount )
			return nLen;
		t.resize( nCount );
		return Read( &t[0], sizeof( T ) * nCount ) + nLen;
	}

	template <typename T>
	bool CheckedRead( T& t )
	{
		return Read( t ) == sizeof( T );
	}
	template <typename T>
	bool CheckedRead( vector<T>& t )
	{
		return Read( t ) >= sizeof( uint32 );
	}

	template <typename T>
	T Read()
	{
		T t;
		Read( t );
		return t;
	}
	const void* GetCurBuffer() const { return ( (const char*)GetBuffer() ) + m_nCurPos; }
	uint32 GetBufLen() const { return m_nBufferLen; }
	uint32 GetBytesLeft() const { return m_nBufferLen - m_nCurPos; }
	void ResetCurPos() { m_nCurPos = 0; }
protected:
	uint32 m_nBufferLen;
	uint32 m_nCurPos;
};

template <>
inline uint32 IBufReader::Read( string& t )
{
	t = "";
	uint32 nPos = m_nCurPos;
	const char* pBuffer = ( const char* )GetBuffer();
	char cBuf;
	while( m_nCurPos < m_nBufferLen && !!( cBuf = pBuffer[m_nCurPos++] ) )
		t.push_back( cBuf );
	return m_nCurPos - nPos;
}

template <>
inline uint32 IBufReader::Read( CString& t )
{
	string str;
	uint32 n = Read( str );
	uint32 nPos = m_nCurPos;
	t = str.c_str();
	return n;
}

template <>
inline bool IBufReader::CheckedRead( string& t )
{
	return Read( t ) >= sizeof( uint32 );
}

template <>
inline bool IBufReader::CheckedRead( CString& t )
{
	return Read( t ) >= sizeof( uint32 );
}

class CBufFile : public IBufReader
{
public:
	CBufFile() {}
	CBufFile( const CBufFile& a ) : IBufReader( a ), m_tempBuffer( a.m_tempBuffer ) {}
	CBufFile& operator = ( const CBufFile& a ) { *(IBufReader*)this = *(IBufReader*)&a; m_tempBuffer = a.m_tempBuffer; return *this; }
	void Clear() { m_nBufferLen = m_nCurPos = 0; }
	void Write( const void* pBuf, uint32 nBufLen );

	template <typename T>
	void Write( const T& t )
	{
		Write( &t, sizeof( T ) );
	}
	template <typename T>
	void Write( const vector<T>& t )
	{
		uint32 nCount = t.size();
		Write( nCount );
		if( nCount )
			Write( &t[0], sizeof(T)* nCount );
	}

	bool operator == ( const CBufFile& a )
	{
		if( m_nBufferLen != a.m_nBufferLen )
			return false;
		if( !m_nBufferLen )
			return true;
		return !memcmp( &m_tempBuffer[0], &a.m_tempBuffer[0], m_nBufferLen );
	}
	bool operator != ( const CBufFile& a )
	{
		return !( *this == a );
	}
	
	const void* GetBuffer() const { return m_tempBuffer.size()? &m_tempBuffer[0]: NULL; }
	void SetBufLen( uint32 n ) { m_nBufferLen = n; }
private:
	vector<uint8> m_tempBuffer;
};

template <>
inline uint32 IBufReader::Read( CBufFile& t )
{
	vector<uint8> vec;
	uint32 nLen = Read( vec );
	if( vec.size() )
		t.Write( &vec[0], vec.size() );
	return nLen;
}

template <>
inline bool IBufReader::CheckedRead( CBufFile& t )
{
	return Read( t ) >= sizeof( uint32 );
}

template <>
inline void CBufFile::Write( const string& t )
{
	Write( t.c_str(), t.length() + 1 );
}

template <>
inline void CBufFile::Write( const CString& t )
{
	Write( t.c_str(), t.length() + 1 );
}

template <>
inline void CBufFile::Write( const IBufReader& t )
{
	Write( t.GetBufLen() );
	Write( t.GetBuffer(), t.GetBufLen() );
}

template <>
inline void CBufFile::Write( const CBufFile& t )
{
	Write( t.GetBufLen() );
	Write( t.GetBuffer(), t.GetBufLen() );
}

class CBufReader : public IBufReader
{
public:
	CBufReader( const void* pBuf, uint32 nBufLen ) : m_pBuf( ( const char* )pBuf ) { m_nBufferLen = nBufLen; }
	CBufReader( const CBufReader& a ) : IBufReader( a ), m_pBuf( a.m_pBuf ) {}
	CBufReader( IBufReader& buf )
	{
		buf.Read( m_nBufferLen );
		m_pBuf = ( const char* )buf.GetCurBuffer();
		buf.Read( NULL, m_nBufferLen );
	}
	CBufReader& operator = ( const CBufReader& a ) { *( IBufReader* )this = *(IBufReader*)&a; m_pBuf = a.m_pBuf; return *this; }

	const void* GetBuffer() const { return m_pBuf; }
private:
	const char* m_pBuf;
};

template <>
inline void CBufFile::Write( const CBufReader& t )
{
	Write( t.GetBufLen() );
	Write( t.GetBuffer(), t.GetBufLen() );
}

#define DEFINE_PACKAGE_EXTRA_BUFFER( t, buf ) CBufReader buf( (t) + 1, (t)->l - sizeof( *(t) ) + 2 );
