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
protected:
	void ( *m_pAllocFunc )( void* );
	void( *m_pFreeFunc )( void* );
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

template <typename T>
class TObjectPool : public CFixedSizeAllocator
{
public:
	TObjectPool( uint32 nChunkSize = 16384 ) : CFixedSizeAllocator( sizeof( T ), nChunkSize ) { m_pAllocFunc = &AllocData; m_pFreeFunc = &FreeData; }

	static void AllocData( void* data ) { new ( data )T; }
	static void FreeData( void* data ) { ( (T*)data )->~T(); }

	static TObjectPool& Inst()
	{
		static TObjectPool<T> g_inst;
		return g_inst;
	}
};