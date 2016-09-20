#pragma once
#include <vector>
#include <map>
#include <sstream>
using namespace std;

class CTabFile
{
public:
	void Load( const char* fileName );

	uint32 GetRowCount() { return m_nHeight; }
	uint32 GetColumnCount() { return m_nWidth; }

	template<typename T>
	T Get( uint32 nColumn, uint32 nRow, T defaultValue = 0 );

	template<typename T>
	T Get( const char* szColumnName, uint32 nRow, T defaultValue = 0 );
private:
	void Build();

	vector<char> m_buffer;
	map<string, int> m_mapColumnNameToIndex;
	vector<const char*> m_vecItems;
	uint32 m_nWidth;
	uint32 m_nHeight;
};

template<typename T>
T CTabFile::Get( const char* szColumnName, uint32 nRow, T defaultValue )
{
	map<string, int>::iterator itr = m_mapColumnNameToIndex.find( szColumnName );
	if( itr == m_mapColumnNameToIndex.end() )
		return defaultValue;
	return Get( itr->second, nRow, defaultValue );
}

template<>
inline const char* CTabFile::Get( uint32 nColumn, uint32 nRow, const char* defaultValue )
{
	if( nColumn >= m_nWidth || nRow >= m_nHeight )
		return defaultValue;
	return m_vecItems[ nColumn + nRow * m_nWidth ];
}

template<typename T>
T CTabFile::Get( uint32 nColumn, uint32 nRow, T defaultValue )
{
	const char* szBuffer = Get<const char*>( nColumn, nRow );
	if( !szBuffer || !szBuffer[0] )
		return defaultValue;
	stringstream ss;
	ss << szBuffer;
	T t;
	ss >> t;
	return t;
}