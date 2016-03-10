#ifndef _SORTEDLIST_H_
#define _SORTEDLIST_H_

#include <vector>
using namespace std;

template <typename T, typename _Val>
class TSortedList
{
public:
	class iterator
	{
	public:
		iterator( TSortedList<T, _Val>* pList ):m_pList( pList ), n( 0 ), n1( 0 )
		{
			while( !End() && n1 >= m_pList->m_vecs[n].size() )
			{
				n++;
				n1 = 0;
			}
		}
		bool End()
		{
			return n >= m_pList->m_vecs.size();
		}
		void Next()
		{
			n1++;
			while( !End() && n1 >= m_pList->m_vecs[n].size() )
			{
				n++;
				n1 = 0;
			}
		}

		T& Get()
		{
			return m_pList->m_vecs[n][n1];
		}
	private:
		TSortedList<T, _Val>* m_pList;
		int n, n1;
	};
	class rev_iterator
	{
	public:
		rev_iterator( TSortedList<T, _Val>* pList ):m_pList( pList ), n( pList->m_vecs.size() - 1 ), n1( 0 ) 
		{
			while( !End() && n1 >= m_pList->m_vecs[n].size() )
			{
				n--;
				n1 = 0;
			}
		}
		bool End()
		{
			return n < 0;
		}
		void Next()
		{
			n1++;
			while( !End() && n1 >= m_pList->m_vecs[n].size() )
			{
				n--;
				n1 = 0;
			}
		}

		T& Get()
		{
			return m_pList->m_vecs[n][n1];
		}
	private:
		TSortedList<T, _Val>* m_pList;
		int n, n1;
	};

	iterator begin() { return iterator( this ); }
	rev_iterator rev_begin() { return rev_iterator( this ); }
	void insert( T t )
	{
		_Val funVal;
		unsigned int val = funVal( t );
		if( val >= m_vecs.size() )
		{
			m_vecs.resize( val + 1 );
		}
		m_vecs[val].push_back( t );
	}
	void clear()
	{
		for( int i = 0; i < m_vecs.size(); i++ )
		{
			m_vecs[i].clear();
		}
	}
private:
	vector<vector<T> > m_vecs;
};

#endif
