// ShaderCompile.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include "Render/stdafx.h"
#include "Render/RenderSystem.h"
#include "Render/GlobalRenderResources.h"
#include <iostream>

void Game_ShaderImplement_Dummy();
int _tmain(int argc, _TCHAR* argv[])
{
	Game_ShaderImplement_Dummy();
	string str;
	bool bCompile = false;
	while( !bCompile )
	{
		std::cin >> str;
		if( str.length() )
			str = "Shader/" + str + ".shader";
		bCompile = CGlobalShader::Compile( IRenderSystem::Inst(), str.c_str() );
		if( !bCompile )
			std::cout << "File not found" << endl;
	}
	return 0;
}

