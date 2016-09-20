#pragma once

#include "Reference.h"

class IConstantBuffer : public CReferenceObject
{
public:
	IConstantBuffer( uint32 nSize ) : m_nSize( nSize ) {}

	uint32 GetSize() { return m_nSize; }
	virtual uint8* GetData() = 0;
	virtual void UpdateBuffer( uint32 nOffset, uint32 nSize, const void* data ) = 0;
	virtual void InvalidBuffer() = 0;
protected:
	uint32 m_nSize;
};