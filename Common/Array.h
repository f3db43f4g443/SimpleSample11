#pragma once
using namespace std;

class CRawArray
{
public:
	CRawArray() {}
	~CRawArray() { Clear(); }

	uint8* GetData() { return m_p; }
	uint32 Size() { return m_nSize; }
	void Resize( uint32 n, uint32 nDataSize )
	{
		if( n == m_nSize )
			return;
		if( n > m_nSize )
		{
			auto p1 = (uint8*)malloc( nDataSize * n );
			memcpy( p1, m_p, nDataSize * m_nSize );
			memset( p1 + nDataSize * m_nSize, 0, nDataSize * ( n - m_nSize ) );
			Clear();
			m_p = p1;
		}
		m_nSize = n;
	}

	static int32 GetPtrOfs() { return MEMBER_OFFSET( CRawArray, m_p ); }
private:
	void Clear() { free( m_p ); m_p = NULL; }
	uint32 m_nSize;
	uint8* m_p;
};

template<typename T>
class TArray
{
public:
	TArray() { for( int i = 0; i < m_nSize; i++ ) Constructor( m_p + i ); }
	TArray( const struct SClassCreateContext& context ) { for( int i = 0; i < m_nSize; i++ ) Constructor( m_p + i ); }
	TArray( const TArray<T>& arr )
	{
		m_nSize = arr.m_nSize;
		m_p = (T*)malloc( sizeof( T ) * m_nSize );
		for( int i = 0; i < m_nSize; i++ )
			new( m_p + i ) T( arr.m_p[i] );
	}
	~TArray() { Clear(); }
	T& operator[]( uint32 n );
	const T& operator[]( uint32 n ) const;

	uint32 Size() const { return m_nSize; }
	void Resize( uint32 n );

	static int32 GetPtrOfs() { return MEMBER_OFFSET( TArray<T>, m_p ); }
private:
	void Clear();

	void Constructor( T* p ) { new( p ) T( SClassCreateContext() ); }
	uint32 m_nSize;
	T* m_p;
};

template<>
void TArray<bool>::Constructor( bool* p ) {}
template<>
void TArray<int8>::Constructor( int8* p ) {}
template<>
void TArray<int16>::Constructor( int16* p ) {}
template<>
void TArray<int32>::Constructor( int32* p ) {}
template<>
void TArray<int64>::Constructor( int64* p ) {}
template<>
void TArray<uint8>::Constructor( uint8* p ) {}
template<>
void TArray<uint16>::Constructor( uint16* p ) {}
template<>
void TArray<uint32>::Constructor( uint32* p ) {}
template<>
void TArray<uint64>::Constructor( uint64* p ) {}
template<>
void TArray<float>::Constructor( float* p ) {}
template<>
void TArray<CVector2>::Constructor( CVector2* p ) {}
template<>
void TArray<CVector3>::Constructor( CVector3* p ) {}
template<>
void TArray<CVector4>::Constructor( CVector4* p ) {}
template<>
void TArray<CRectangle>::Constructor( CRectangle* p ) {}

template<typename T>
T& TArray<T>::operator[]( uint32 n )
{
	ASSERT( n < m_nSize );
	return m_p[n];
}

template<typename T>
const T& TArray<T>::operator[]( uint32 n ) const
{
	ASSERT( n < m_nSize );
	return m_p[n];
}

template<typename T>
void TArray<T>::Resize( uint32 n )
{
	if( n == m_nSize )
		return;
	if( n > m_nSize )
	{
		auto p1 = (T*)malloc( sizeof( T ) * n );
		memset( p1, 0, sizeof( T ) * n );
		for( int i = 0; i < m_nSize; i++ )
			new( p1 + i ) T( m_p[i] );
		for( int i = m_nSize; i < n; i++ )
			Constructor( p1 + i );
		Clear();
		m_p = p1;
	}
	else
	{
		for( int i = n; i < m_nSize; i++ )
			m_p[i].~T();
	}
	m_nSize = n;
}

template<typename T>
void TArray<T>::Clear()
{
	for( int i = 0; i < m_nSize; i++ )
		m_p[i].~T();
	free( m_p );
	m_p = NULL;
}