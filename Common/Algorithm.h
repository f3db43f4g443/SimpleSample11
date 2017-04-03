#pragma once
#include <vector>
using namespace std;

class CUnionFind
{
public:
	void Init( uint32 nCount )
	{
		vecGroups.resize( nCount );
		for( int i = 0; i < nCount; i++ )
			vecGroups[i] = i;
	}

	bool Union( uint32 a, uint32 b )
	{
		a = FindParent( a );
		b = FindParent( b );
		if( a != b )
		{
			vecGroups[b] = a;
			return true;
		}
		return false;
	}

	uint32 FindParent( uint32 a )
	{
		uint32 a0 = a;
		uint32 a1 = vecGroups[a];
		while( a1 != a )
		{
			a = a1;
			a1 = vecGroups[a];
		}

		a = a0;
		a0 = vecGroups[a];
		while( a0 != a )
		{
			a = a0;
			a0 = vecGroups[a];
			vecGroups[a] = a1;
		}
		return a1;
	}
private:
	vector<uint32> vecGroups;
};