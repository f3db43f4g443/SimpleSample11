#pragma once

#include "Reference.h"
#include "ShaderResource.h"

class CIndexBufferDesc
{
public:
	CIndexBufferDesc() {}
	CIndexBufferDesc( uint32 nIndices, EFormat eFormat, bool bIsDynamic = false, bool bBindStreamOutput = false )
		: eFormat( eFormat ), nIndices( nIndices ), bIsDynamic( bIsDynamic ), bBindStreamOutput( bBindStreamOutput )
	{
		nOffset = FormatToLength( eFormat );
	}
	EFormat eFormat;
	uint32 nOffset;
	uint32 nIndices;

	bool bIsDynamic;
	bool bBindStreamOutput;
};

class IIndexBuffer : public CReferenceObject
{
public:
	IIndexBuffer( uint32 nIndices, EFormat eFormat, void* pData, bool bIsDynamic = false, bool bBindStreamOutput = false )
		: m_desc( nIndices, eFormat, bIsDynamic, bBindStreamOutput ), m_pStreamOutput( NULL ) {}
	~IIndexBuffer() { if( m_pStreamOutput ) delete m_pStreamOutput; }

	IStreamOutput* GetStreamOutput() { return m_pStreamOutput; }
	const CIndexBufferDesc& GetDesc() { return m_desc; }
protected:
	CIndexBufferDesc m_desc;
	IStreamOutput* m_pStreamOutput;
};