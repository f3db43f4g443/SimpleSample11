#include "Common.h"
#include "FixedSizeAllocator.h"

CFixedSizeAllocator::CFixedSizeAllocator( uint32 nSize, uint32 nChunkSize ) : m_pFreedBlocks( NULL )
{
	uint32 nBlockSize = nSize + sizeof( SBlock );
	nBlockSize = ( nBlockSize + 3 ) & ~3;
	m_nBlockSize = nBlockSize;
	m_nChunkSize = nChunkSize;
	m_nBlockCountInChunk = m_nChunkSize / m_nBlockSize;
}

void* CFixedSizeAllocator::Alloc()
{
	if( m_pFreedBlocks )
	{
		SBlock* pBlock = m_pFreedBlocks;
		m_pFreedBlocks = m_pFreedBlocks->pNext;
		return pBlock + 1;
	}
	else
	{
		m_chunks.resize( m_chunks.size() + 1 );
		SChunk& chunk = m_chunks.back();
		chunk.pBlock = (SBlock*)malloc( m_nChunkSize );
		uint8* pBlockData = (uint8*)chunk.pBlock;
		SBlock* pBlock = NULL;
		for( int i = 0; i < m_nBlockCountInChunk; i++ )
		{
			pBlock = (SBlock*)pBlockData;
			pBlockData += m_nBlockSize;
			pBlock->pNext = (SBlock*)pBlockData;
		}
		pBlock->pNext = m_pFreedBlocks;
		m_pFreedBlocks = chunk.pBlock->pNext;
		return chunk.pBlock + 1;
	}
}

void CFixedSizeAllocator::Free( void* pData )
{
	SBlock* pBlock = (SBlock*)pData - 1;
	pBlock->pNext = m_pFreedBlocks;
	m_pFreedBlocks = pBlock;
}

void CFixedSizeAllocator::Clear()
{
	for( int i = 0; i < m_chunks.size(); i++ )
	{
		free( m_chunks[i].pBlock );
	}
	m_chunks.clear();
	m_pFreedBlocks = NULL;
}