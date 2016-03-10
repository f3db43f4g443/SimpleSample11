#pragma once

#include "DX11Common.h"
#include "Shader.h"

class CVertexShader : public IShader
{
public:
	CVertexShader( ID3D10Blob* pCode, ID3D11VertexShader* pShader ) : m_pByteCode( pCode ), m_pShader( pShader ) {}
	ID3D10Blob* GetByteCode() { return m_pByteCode; }
	ID3D11VertexShader* GetShader() { return m_pShader; }
private:
	CReference<ID3D10Blob> m_pByteCode;
	CReference<ID3D11VertexShader> m_pShader;
};

class CGeometryShader : public IShader
{
public:
	CGeometryShader( ID3D10Blob* pCode, ID3D11GeometryShader* pShader ) : m_pByteCode( pCode ), m_pShader( pShader ) {}
	ID3D10Blob* GetByteCode() { return m_pByteCode; }
	ID3D11GeometryShader* GetShader() { return m_pShader; }
private:
	CReference<ID3D10Blob> m_pByteCode;
	CReference<ID3D11GeometryShader> m_pShader;
};

class CPixelShader : public IShader
{
public:
	CPixelShader( ID3D10Blob* pCode, ID3D11PixelShader* pShader ) : m_pByteCode( pCode ), m_pShader( pShader ) {}
	ID3D10Blob* GetByteCode() { return m_pByteCode; }
	ID3D11PixelShader* GetShader() { return m_pShader; }
private:
	CReference<ID3D10Blob> m_pByteCode;
	CReference<ID3D11PixelShader> m_pShader;
};

class CVertexBufferDesc;
class CShaderBoundState : public IShaderBoundState
{
public:
	CShaderBoundState( ID3D11Device* pDevice, IShader* pVertexShader, IShader* pPixelShader, const CVertexBufferDesc** ppVertexBufferDesc, uint32 nVertexBuffers, IShader* pGeometryShader );

	virtual IShader* GetShader( EShaderType eType ) override;
	CVertexShader* GetVertexShader() { return m_pVertexShader; }
	CPixelShader* GetPixelShader() { return m_pPixelShader; }
	CGeometryShader* GetGeometryShader() { return m_pGeometryShader; }
	ID3D11InputLayout* GetInputLayout() { return m_pInputLayout; }

	int32 GetInstanceBufferIndex() { return m_nInstanceBufferIndex; }
private:
	CReference<CVertexShader> m_pVertexShader;
	CReference<CPixelShader> m_pPixelShader;
	CReference<CGeometryShader> m_pGeometryShader;

	CReference<ID3D11InputLayout> m_pInputLayout;

	int32 m_nInstanceBufferIndex;
};