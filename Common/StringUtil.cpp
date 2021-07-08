#include "Common.h"
#include "StringUtil.h"
#include "BufFile.h"


#if _MSC_VER >= 1900
#define STRING_POOL &m_stringPool._Get_data()
#else
#define STRING_POOL &m_stringPool
#endif

map<string, int> CString::m_stringPool;

CString::CString( const char* c )
{
	map<string, int>::iterator itr = m_stringPool.find( c );
	if( itr == m_stringPool.end() )
	{
		m_stringPool.insert( pair<string, int>( c, 0 ) );
		itr = m_stringPool.find( c );
	}
	map<string, int>::_Nodeptr ptr = itr._Mynode();
	ptr->_Myval.second++;
	m_ptr = ptr;
}

CString::CString( const CString& str ) :m_ptr( str.m_ptr )
{
	map<string, int>::_Nodeptr ptr = map<string, int>::_Nodeptr( m_ptr );
	ptr->_Myval.second++;
}

CString::~CString()
{
	if( !m_ptr )
		return;
	map<string, int>::_Nodeptr ptr = map<string, int>::_Nodeptr( m_ptr );
	if( !--( ptr->_Myval.second ) )
	{
		m_stringPool.erase( map<string, int>::iterator( ptr, STRING_POOL ) );
	}
}

bool CString::operator < ( const CString& rhs ) const
{
	map<string, int>::_Nodeptr ptr = map<string, int>::_Nodeptr( m_ptr );
	map<string, int>::_Nodeptr ptr1 = map<string, int>::_Nodeptr( rhs.m_ptr );
	return ptr->_Myval.first < ptr1->_Myval.first;
}

bool CString::operator < ( const char* rhs ) const
{
	return strcmp( this->c_str(), rhs ) < 0;
}

bool operator < ( const char* lhs, const CString& rhs )
{
	return strcmp( lhs, rhs.c_str() ) < 0;
}

bool CString::operator == ( const CString& rhs ) const
{
	return m_ptr == rhs.m_ptr;
}

bool CString::operator != ( const CString& rhs ) const
{
	return m_ptr != rhs.m_ptr;
}

bool CString::operator == ( const char* rhs ) const
{
	return strcmp( this->c_str(), rhs ) == 0;
}

bool CString::operator != ( const char* rhs ) const
{
	return strcmp( this->c_str(), rhs ) != 0;
}

bool operator == ( const char* lhs, const CString& rhs )
{
	return strcmp( lhs, rhs.c_str() ) == 0;
}

CString& CString::operator = ( const char* rhs )
{
	map<string, int>::_Nodeptr ptr;
	if( m_ptr )
	{
		ptr = map<string, int>::_Nodeptr( m_ptr );
		if( !--( ptr->_Myval.second ) )
		{
			m_stringPool.erase( map<string, int>::iterator( ptr, STRING_POOL ) );
		}
	}

	map<string, int>::iterator itr = m_stringPool.find( rhs );
	if( itr == m_stringPool.end() )
	{
		m_stringPool.insert( pair<string, int>( rhs, 0 ) );
		itr = m_stringPool.find( rhs );
	}
	ptr = itr._Mynode();
	ptr->_Myval.second++;
	m_ptr = ptr;
	return *this;
}

CString& CString::operator = ( const CString& rhs )
{
	map<string, int>::_Nodeptr ptr = map<string, int>::_Nodeptr( m_ptr );
	if( !--( ptr->_Myval.second ) )
	{
		m_stringPool.erase( map<string, int>::iterator( ptr, STRING_POOL ) );
	}
	ptr = map<string, int>::_Nodeptr( rhs.m_ptr );
	m_ptr = ptr;
	ptr->_Myval.second++;
	return *this;
}

const char* CString::c_str() const
{
	map<string, int>::_Nodeptr ptr = map<string, int>::_Nodeptr( m_ptr );
	return ptr->_Myval.first.c_str();
}

const char* CString::c_str_safe() const
{
	if( !m_ptr )
		return "";
	map<string, int>::_Nodeptr ptr = map<string, int>::_Nodeptr( m_ptr );
	return ptr->_Myval.first.c_str();
}

int CString::length() const
{
	map<string, int>::_Nodeptr ptr = map<string, int>::_Nodeptr( m_ptr );
	return ptr->_Myval.first.length();
}

void CString::PackData( CBufFile& buf, bool bWithMetaData ) const
{
	if( m_ptr )
	{
		map<string, int>::_Nodeptr ptr = map<string, int>::_Nodeptr( m_ptr );
		buf.Write( ptr->_Myval.first );
	}
	else
		buf.Write<int8>( 0 );
}

void CString::UnpackData( IBufReader& buf, bool bWithMetaData, void* pContext )
{
	string str;
	buf.Read( str );
	*this = str.c_str();
}

bool CString::DiffData( const CString& obj0, CBufFile& buf ) const
{
	if( *this == obj0 )
		return false;
	PackData( buf, true );
	return true;
}

void CString::PatchData( IBufReader& buf, void* pContext )
{
	UnpackData( buf, true, pContext );
}
