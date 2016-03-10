#pragma once
#include "Reference.h"

enum class EShaderResourceType
{
	Buffer,
	Texture2D,
	TextureCube,
	Texture3D,
};

class IShaderResource
{
public:
	IShaderResource( EShaderResourceType eType, CReferenceObject* pOrigObject ) : m_eType( eType ), m_pOrigObject( pOrigObject ) {}
	virtual ~IShaderResource() {}
	EShaderResourceType GetType() { return m_eType; }
	CReferenceObject* GetOrigObject() { return m_pOrigObject; }
	
	void AddRef()
	{
		m_pOrigObject->AddRef();
	}
	void Release()
	{
		m_pOrigObject->Release();
	}
protected:
	EShaderResourceType m_eType;
	CReferenceObject* m_pOrigObject;
};

enum class EStreamOutputType
{
	VertexBuffer,
	IndexBuffer,
};

class IStreamOutput
{
public:
	IStreamOutput( EStreamOutputType eType, CReferenceObject* pOrigObject ) : m_eType( eType ), m_pOrigObject( pOrigObject ) {}
	virtual ~IStreamOutput() {}
	EStreamOutputType GetType() { return m_eType; }
	CReferenceObject* GetOrigObject() { return m_pOrigObject; }
	
	void AddRef()
	{
		m_pOrigObject->AddRef();
	}
	void Release()
	{
		m_pOrigObject->Release();
	}
protected:
	EStreamOutputType m_eType;
	CReferenceObject* m_pOrigObject;
};