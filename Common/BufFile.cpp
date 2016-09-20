#include "Common.h"
#include "BufFile.h"

uint32 IBufReader::Read( void* pBuf, uint32 nMaxLen )
{
	if( !nMaxLen )
		return 0;
	uint32 nMaxSize = m_nBufferLen - m_nCurPos;
	if( !nMaxSize )
		return 0;
	nMaxLen = MIN( nMaxLen, nMaxSize );
	if( pBuf )
		memcpy( pBuf, &( ( const char* )GetBuffer() )[m_nCurPos], nMaxLen );
	m_nCurPos += nMaxLen;
	return nMaxLen;
}
uint32 IBufReader::Read(string& t, uint32 nMaxLen)
{
	uint32 nPos = m_nCurPos;
	uint32 nMaxSize = m_nBufferLen - m_nCurPos;
	if( !nMaxSize )
		return 0;
	nMaxLen = MIN( nMaxLen, nMaxSize );
	const char* pBuffer = (const char*)GetBuffer();
	uint32 strEndPos = nPos + nMaxLen;
	while( m_nCurPos < strEndPos )
		t.push_back( pBuffer[m_nCurPos++] );
	return m_nCurPos - nPos;
}
void CBufFile::Write( const void* pBuf, uint32 nBufLen )
{
	if( !nBufLen )
		return;
	uint32 nNewSize = m_nBufferLen + nBufLen;
	if( m_tempBuffer.size() < nNewSize )
		m_tempBuffer.resize( nNewSize );
	memcpy( &m_tempBuffer[m_nBufferLen], pBuf, nBufLen );
	m_nBufferLen += nBufLen;
}