#pragma once
#include "Common.h"

template<typename T, typename TValue = uint32>
class TPriorityQueue;

template<typename T, typename TValue = uint32>
class TPriorityQueueNode
{
public:
	friend class TPriorityQueue<T, TValue>;

	virtual TValue GetPriority() = 0;
private:
	uint32 m_nPriorityQueueIndex;
};

#define ASSIGN_NODE_INDEX( Node, Index ) \
	pHead[nIndex] = Node; \
	Node->m_nPriorityQueueIndex = nIndex;

template<typename T, typename TValue = uint32>
class TPriorityQueue
{
public:
	typedef TPriorityQueueNode<T, TValue> NodeType;

	void Insert( NodeType* pNode )
	{
		if( !m_vecNodes.size() )
		{
			m_vecNodes.push_back( pNode );
			pNode->m_nPriorityQueueIndex = 1;
			return;
		}

		m_vecNodes.push_back( pNode );
		auto pHead = Head();
		uint32 nIndex = m_vecNodes.size();
		TValue priority = pNode->GetPriority();
		Float( pHead, nIndex, priority );
		ASSIGN_NODE_INDEX( pNode, nIndex );
	}
	void Remove( NodeType* pNode )
	{
		auto pHead = Head();
		uint32 nIndex = pNode->m_nPriorityQueueIndex;
		while( nIndex > 1 )
		{
			uint32 nIndex1 = nIndex >> 1;
			auto pNode1 = pHead[nIndex1];
			ASSIGN_NODE_INDEX( pNode1, nIndex );
			nIndex = nIndex1;
		}
		Pop();
	}
	void Modify( NodeType* pNode )
	{
		auto pHead = Head();
		uint32 nIndex = pNode->m_nPriorityQueueIndex;
		TValue priority = pNode->GetPriority();
		Float( pHead, nIndex, priority );
		Sink( pHead, nIndex, priority );
		ASSIGN_NODE_INDEX( pNode, nIndex );
	}

	NodeType* Pop()
	{
		auto pHead = Head();
		if( !pHead )
			return NULL;

		auto pPop = pHead[1];
		auto pNode = pHead[Size()];
		m_vecNodes.pop_back();
		pHead = Head();
		uint32 nSize = Size();
		if( nSize )
		{
			uint32 nIndex = 1;
			TValue priority = pNode->GetPriority();
			Sink( pHead, nIndex, priority );
			ASSIGN_NODE_INDEX( pNode, nIndex );
		}

		return pPop;
	}
	uint32 Size() { return m_vecNodes.size(); }

	NodeType* Front() { return m_vecNodes.size() ? m_vecNodes[0] : NULL; }
	NodeType** Head() { return m_vecNodes.size() ? &m_vecNodes[0] - 1 : NULL; }

	void Clear() { m_vecNodes.resize( 0 ); }
	const vector<NodeType*>& GetAllNodes() { return m_vecNodes; }
	void Reserve( uint32 nSize ) { m_vecNodes.reserve( nSize ); }
private:
	vector<NodeType*> m_vecNodes;

	void Float( NodeType** pHead, uint32& nIndex, TValue priority )
	{
		while( nIndex > 1 )
		{
			uint32 nIndex1 = nIndex >> 1;
			auto pNode1 = pHead[nIndex1];
			if( pNode1->GetPriority() <= priority )
				break;
			ASSIGN_NODE_INDEX( pNode1, nIndex );
			nIndex = nIndex1;
		}
	}
	void Sink( NodeType** pHead, uint32& nIndex, TValue priority )
	{
		uint32 nSize = Size();
		while( true )
		{
			uint32 nIndex1 = nIndex << 1;
			if( nIndex1 > nSize )
				break;
			auto pNode1 = pHead[nIndex1];
			uint32 nIndex2 = nIndex1 + 1;
			if( nIndex2 <= nSize )
			{
				auto pNode2 = pHead[nIndex2];
				if( pNode2->GetPriority() < pNode1->GetPriority() )
				{
					pNode1 = pNode2;
					nIndex1 = nIndex2;
				}
			}

			if( priority <= pNode1->GetPriority() )
				break;
			
			ASSIGN_NODE_INDEX( pNode1, nIndex );
			nIndex = nIndex1;
		}
	}
};

#undef ASSIGN_NODE_INDEX