#pragma once

#include "DX11Common.h"
#include "Shader.h"

class CVertexShader : public IShader
{
public:
	CVertexShader( void* pShaderCode, uint32 nShaderCodeLength, ID3D11VertexShader* pShader ) : m_pShader( pShader ) { m_code.resize( nShaderCodeLength ); memcpy( &m_code[0], pShaderCode, nShaderCodeLength ); }
	vector<uint8>& GetByteCode() { return m_code; }
	ID3D11VertexShader* GetShader() { return m_pShader; }
private:
	vector<uint8> m_code;
	CReference<ID3D11VertexShader> m_pShader;
};

class CGeometryShader : public IShader
{
public:
	CGeometryShader( void* pShaderCode, uint32 nShaderCodeLength, ID3D11GeometryShader* pShader ) : m_pShader( pShader ) { m_code.resize( nShaderCodeLength ); memcpy( &m_code[0], pShaderCode, nShaderCodeLength ); }
	vector<uint8>& GetByteCode() { return m_code; }
	ID3D11GeometryShader* GetShader() { return m_pShader; }
private:
	vector<uint8> m_code;
	CReference<ID3D11GeometryShader> m_pShader;
};

class CPixelShader : public IShader
{
public:
	CPixelShader( void* pShaderCode, uint32 nShaderCodeLength, ID3D11PixelShader* pShader ) : m_pShader( pShader ) { m_code.resize( nShaderCodeLength ); memcpy( &m_code[0], pShaderCode, nShaderCodeLength ); }
	vector<uint8>& GetByteCode() { return m_code; }
	ID3D11PixelShader* GetShader() { return m_pShader; }
private:
	vector<uint8> m_code;
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