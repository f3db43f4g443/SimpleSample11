#pragma once

#include "Reference.h"
#include "ShaderResource.h"
#include <string>
#include <vector>
using namespace std;

struct SVertexBufferElement
{
	string strSemanticName;
	uint32 nSemanticIndex;
	EFormat eFormat;
	uint32 nOffset;
};

class CVertexBufferDesc {
public:
	CVertexBufferDesc() {}
	CVertexBufferDesc( uint32 nElements, const SVertexBufferElement* pElements, uint32 nVertices, bool bIsDynamic = false,
		bool bBindShaderResource = false, bool bBindStreamOutput = false, bool bIsInstance = false )
		: nVertices( nVertices ), bIsDynamic( bIsDynamic ), bBindShaderResource( bBindShaderResource ), bBindStreamOutput( bBindStreamOutput ), bIsInstanceData( bIsInstance )
	{
		vecElements.resize( nElements );
		nStride = 0;
		for( int i = 0; i < nElements; i++ )
		{
			vecElements[i].strSemanticName = pElements[i].strSemanticName;
			vecElements[i].nSemanticIndex = pElements[i].nSemanticIndex;
			vecElements[i].eFormat = pElements[i].eFormat;
			vecElements[i].nOffset = nStride;
			nStride += FormatToLength( pElements[i].eFormat );
		}
	}

	static CVertexBufferDesc* GetDescForIndexBufferStreamOutput()
	{
		static CVertexBufferDesc g_desc;
		static bool bInited = false;
		if( !bInited )
		{
			g_desc.nStride = 4;
			g_desc.vecElements.resize( 1 );
			auto& element = g_desc.vecElements[0];
			element.strSemanticName = "Index";
			element.nSemanticIndex = 0;
			element.eFormat = EFormat::EFormatR32UInt;
			element.nOffset = 0;
			bInited = true;
		}
		return &g_desc;
	}

	vector<SVertexBufferElement> vecElements;
	int nStride;
	int nVertices;

	bool bIsDynamic;
	bool bBindShaderResource;
	bool bBindStreamOutput;
	bool bIsInstanceData;
};

class IVertexBuffer : public CReferenceObject
{
public:
	IVertexBuffer( uint32 nElements, const SVertexBufferElement* pElements, uint32 nVertices, void* pData, bool bIsDynamic = false,
		bool bBindShaderResource = false, bool bBindStreamOutput = false, bool bIsInstance = false )
		: m_desc( nElements, pElements, nVertices, bIsDynamic, bBindShaderResource, bBindStreamOutput ), m_pShaderResource( NULL ), m_pStreamOutput( NULL ) {}
	~IVertexBuffer()
	{
		if( m_pShaderResource )
			delete m_pShaderResource;
		if( m_pStreamOutput )
			delete m_pStreamOutput;
	}

	IShaderResource* GetShaderResource() { return m_pShaderResource; }
	IStreamOutput* GetStreamOutput() { return m_pStreamOutput; }
	const CVertexBufferDesc& GetDesc() { return m_desc; }
protected:
	CVertexBufferDesc m_desc;
	IShaderResource* m_pShaderResource;
	IStreamOutput* m_pStreamOutput;
};
