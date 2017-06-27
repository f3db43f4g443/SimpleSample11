#pragma once

#include <map>
#include <string>
using namespace std;

class CString
{
public:
	CString( const char* c );
	CString( const CString& str );
	CString( const struct SClassCreateContext& context ) { if( !m_ptr ) *this = ""; }
	~CString();

	bool operator < ( const CString& rhs ) const;
	bool operator < ( const char* rhs ) const;
	friend bool operator < ( const char* lhs, const CString& rhs );
	bool operator == ( const CString& rhs ) const;
	bool operator != ( const CString& rhs ) const;
	bool operator == ( const char* rhs ) const;
	friend bool operator == ( const char* lhs, const CString& rhs );
	
	CString& operator = ( const char* rhs );
	CString& operator = ( const CString& rhs );

	operator const char* () const
	{
		return c_str();
	}

	const char* c_str() const;
	const char* c_str_safe() const;
	int length() const;

	void PackData( class CBufFile& buf, bool bWithMetaData );
	void UnpackData( class IBufReader& buf, bool bWithMetaData );

	static const CString& empty()
	{
		static CString str( "" );
		return str;
	}
private:
	void* m_ptr;

	static map<string, int> m_stringPool;
};