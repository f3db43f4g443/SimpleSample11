#pragma once

#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "Texture.h"
#include "Shader.h"
#include <vector>
#include <map>
using namespace std;

class IRenderSystem;
class CVertexBufferDesc;
class CGlobalShader
{
	friend bool Compile( IRenderSystem* pRenderSystem, const char* szFileName );
	friend void Compile( CGlobalShader* pShader, const char* szShaderName, const char* szName, const char* szFunctionName, const char* szProfile, const CVertexBufferDesc** ppSOVertexBufferDesc, uint32 nVertexBuffers, uint32 nRasterizedStream );
public:
	class IInitor
	{
		friend bool Compile( IRenderSystem* pRenderSystem, const char* szFileName );
	public:
		virtual void Init() = 0;
		virtual const char* GetFileName() = 0;
	protected:
		CGlobalShader* m_pShader;
		const char* m_szShaderName;
		const char* m_szName;
		const char* m_szFunctionName;
		const char* m_szProfile;
		const CVertexBufferDesc** m_ppSOVertexBufferDesc;
		uint32 m_nVertexBuffers;
		uint32 m_nRasterizedStream;
	};

	template<class T>
	class CInitor : public IInitor
	{
	public:
		CInitor( const char* szShaderName, const char* szName, const char* szFunctionName, const char* szProfile, const CVertexBufferDesc** ppSOVertexBufferDesc = NULL, uint32 nVertexBuffers = 0, uint32 nRasterizedStream = 0 )
		{
			m_szShaderName = szShaderName;
			m_szName = szName;
			m_szFunctionName = szFunctionName;
			m_szProfile = szProfile;
			m_ppSOVertexBufferDesc = ppSOVertexBufferDesc;
			m_nVertexBuffers = nVertexBuffers;
			m_nRasterizedStream = nRasterizedStream;
			m_pShader = new T;
			CGlobalShader::GetShaderInitors().push_back( this );
		}

		T* GetShader() { return static_cast<T*>( m_pShader ); }
		void Init() { m_pShader->Create( m_szShaderName, m_szProfile, m_ppSOVertexBufferDesc, m_nVertexBuffers, m_nRasterizedStream ); }

		const char* GetFileName() { return m_szName; }
	protected:
	};

	virtual ~CGlobalShader() {}
	void Create( const char* szShaderName, const char* szProfile, const CVertexBufferDesc** ppSOVertexBufferDesc = NULL, uint32 nVertexBuffers = 0, uint32 nRasterizedStream = 0 );
	
	IShader* GetShader() { return m_pShader; }

	static void Init( IRenderSystem* pRenderSystem );
	static map<string, IShader*>& GetShaders()
	{
		static map<string, IShader*> g_shaders;
		return g_shaders;
	}
	static IShader* GetShaderByName( const char* szName );
protected:
	virtual void SetMacros( SShaderMacroDef& macros ) {}
	virtual void OnCreated() {}
private:
	CReference<IShader> m_pShader;

	static vector<IInitor*>& GetShaderInitors()
	{
		static vector<IInitor*> g_shaders;
		return g_shaders;
	}
};

#define DECLARE_GLOBAL_SHADER( ClassName ) \
private: \
	static CInitor<ClassName> s_initor; \
public: \
	static ClassName* Inst() { return s_initor.GetShader(); }

#define IMPLEMENT_GLOBAL_SHADER( ClassName, Name, Function, Profile ) \
CGlobalShader::CInitor< ClassName > ClassName::s_initor( "g_"#ClassName, Name, Function, Profile );

#define IMPLEMENT_GLOBAL_SHADER_WITH_SO( ClassName, Name, Function, Profile, ppSOVertexBufferDesc, nVertexBuffers, nRasterizedStream ) \
CGlobalShader::CInitor< ClassName > ClassName::s_initor( "g_"#ClassName, Name, Function, Profile, ppSOVertexBufferDesc, nVertexBuffers, nRasterizedStream );

#define IMPLEMENT_GLOBAL_SHADER_WITH_SHADER_NAME( ShaderName, ClassName, Name, Function, Profile ) \
CGlobalShader::CInitor< ClassName > ClassName::s_initor( ShaderName, Name, Function, Profile );

class CGlobalRenderResources
{
public:
	void Init( IRenderSystem* pRenderSystem );
	static CGlobalRenderResources* Inst()
	{
		static CGlobalRenderResources g_inst;
		return &g_inst;
	}

	IVertexBuffer* GetVBDebug() { return m_pVBDebug; }
	IVertexBuffer* GetVBQuad() { return m_pVBQuad; }
	IIndexBuffer* GetIBQuad() { return m_pIBQuad; }
	static const uint32 nStripInstanceCount = 4096;
	IVertexBuffer* GetVBStrip() { return m_pVBStrip; }
	ITexture* GetRandomNorm() { return m_pTexRandomNorm; }
private:
	CReference<IVertexBuffer> m_pVBDebug;
	CReference<IVertexBuffer> m_pVBQuad;
	CReference<IIndexBuffer> m_pIBQuad;
	CReference<IVertexBuffer> m_pVBStrip;

	CReference<ITexture> m_pTexRandomNorm;
};

void InitEngine();