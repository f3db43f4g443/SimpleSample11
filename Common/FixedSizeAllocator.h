#pragma once
#include <vector>
using namespace std;

class CFixedSizeAllocator
{
public:
	struct SBlock
	{
		SBlock* pNext;
	};
	struct SChunk
	{
		SBlock* pBlock;
	};

	CFixedSizeAllocator( uint32 nSize, uint32 nChunkSize = 16384 );
	~CFixedSizeAllocator() { Clear(); }

	void* Alloc();
	void Free( void* pData );
	void Clear();
private:
	vector<SChunk> m_chunks;
	uint32 m_nChunkSize;
	uint32 m_nBlockSize;
	uint32 m_nBlockCountInChunk;
	SBlock* m_pFreedBlocks;
};

template <typename T>
class TObjectAllocator : public CFixedSizeAllocator
{
public:
	TObjectAllocator( uint32 nChunkSize = 16384 ) : CFixedSizeAllocator( sizeof( T ), nChunkSize ) {}

	static TObjectAllocator& Inst()
	{
		static TObjectAllocator<T> g_inst;
		return g_inst;
	}
};