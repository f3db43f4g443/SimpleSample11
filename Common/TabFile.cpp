#include "Common.h"
#include "TabFile.h"
#include "FileUtil.h"

void CTabFile::Load( const char* fileName )
{
	m_buffer.resize( 0 );
	GetFileContent( m_buffer, fileName, true );
	Build();
}

void CTabFile::Build()
{
	m_nWidth = m_nHeight = 0;
	m_mapColumnNameToIndex.clear();
	m_vecItems.clear();
	if( !m_buffer.size() )
		return;
	char* szBuffer = &m_buffer[0];
	for( char* p = szBuffer; *p; p++ )
	{
		bool bBreak = false;
		switch ( *p )
		{
		case '\t':
		case '\r':
			{
				*p = 0;
				m_mapColumnNameToIndex[szBuffer] = m_nWidth++;
				szBuffer = p + 1;
				break;
			}
		case '\n':
			{
				if( *( p - 1 ) != '\r' )
				{
					*p = 0;
					m_mapColumnNameToIndex[szBuffer] = m_nWidth++;
				}
				szBuffer = p + 1;
				bBreak = true;
				break;
			}
		default:
			break;
		}
		if( bBreak )
			break;
	}
	
	for( char* p = szBuffer; *p; p++ )
	{
		switch ( *p )
		{
		case '\t':
		case '\r':
			{
				*p = 0;
				m_vecItems.push_back( szBuffer );
				szBuffer = p + 1;
				break;
			}
		case '\n':
			{
				if( *( p - 1 ) != '\r' )
				{
					*p = 0;
					m_vecItems.push_back( szBuffer );
				}
				szBuffer = p + 1;
				break;
			}
		default:
			break;
		}
	}
	m_nHeight = m_vecItems.size() / m_nWidth;
}
