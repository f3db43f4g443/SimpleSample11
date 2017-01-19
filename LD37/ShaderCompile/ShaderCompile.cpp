// ShaderCompile.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include "Render/stdafx.h"
#include "Render/RenderSystem.h"
#include "Render/GlobalRenderResources.h"
#include "Common/FileUtil.h"
#include <iostream>

bool CompileShader( CBufFile& buf, const char* pData, uint32 nLen, const char* szFunctionName, const char* szProfile, SShaderMacroDef* pMacros, const char* szInclude,
	const CVertexBufferDesc** ppSOVertexBufferDesc, uint32 nVertexBuffers, uint32 nRasterizedStream );

void Compile( CGlobalShader* pShader, const char* szShaderName, const char* szName, const char* szFunctionName, const char* szProfile, const CVertexBufferDesc** ppSOVertexBufferDesc, uint32 nVertexBuffers, uint32 nRasterizedStream )
{
	vector<char> content;
	uint32 nLen = GetFileContent( content, szName, true );

	SShaderMacroDef macroDef;
	pShader->SetMacros( macroDef );

	CBufFile buf;
	string strPath = szName;
	strPath = strPath.substr( 0, strPath.rfind( "/" ) + 1 );
	bool bCreated = CompileShader( buf, &content[0], nLen, szFunctionName, szProfile, &macroDef, strPath.c_str(), ppSOVertexBufferDesc, nVertexBuffers, nRasterizedStream );

	if( bCreated )
	{
		string strFileName = "Root/Shader/";
		strFileName += szShaderName;
		strFileName += ".sb";
		SaveFile( strFileName.c_str(), buf.GetBuffer(), buf.GetBufLen() );
	}
}

bool Compile( IRenderSystem* pRenderSystem, const char* szFileName = NULL )
{
	auto shaders = CGlobalShader::GetShaderInitors();
	bool bCompile = false;
	for( auto item : shaders )
	{
		if( szFileName && szFileName[0] && strcmp( szFileName, item->GetFileName() ) )
			continue;
		bCompile = true;
		Compile( item->m_pShader, item->m_szShaderName, item->m_szName, item->m_szFunctionName, item->m_szProfile, item->m_ppSOVertexBufferDesc, item->m_nVertexBuffers, item->m_nRasterizedStream );
	}
	return bCompile;
}

void Game_ShaderImplement_Dummy();
int _tmain(int argc, _TCHAR* argv[])
{
	Game_ShaderImplement_Dummy();
	string str;
	bool bCompile = false;
	while( !bCompile )
	{
		str = "";
		char c;
		while( ( c = getchar() ) != '\n' )
			str += c;
		if( str.length() )
			str = "Shader/" + str + ".shader";
		bCompile = Compile( IRenderSystem::Inst(), str.c_str() );
		if( !bCompile )
			std::cout << "File not found" << endl;
	}
	return 0;
}

