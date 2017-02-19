#pragma once
#include "Texture.h"
#include "Trigger.h"
#include <vector>
#include "Math3D.h"
using namespace std;

class CPostProcessPass;
class CPostProcess
{
public:
	CPostProcess() : m_bEnabled( true ), m_nPriority( 0 ) {}
	virtual ~CPostProcess() {}
	virtual void Process( CPostProcessPass* pPass, IRenderTarget* pFinalTarget ) = 0;
	virtual bool IsForceFirstPass() { return false; }

	bool IsEnabled() { return m_bEnabled; }
	void SetEnabled( bool bEnabled ) { m_bEnabled = bEnabled; }
	uint32 GetPriority() { return m_nPriority; }
	void SetPriority( uint32 nPriority ) { m_nPriority = nPriority; }
private:
	bool m_bEnabled;
	uint32 m_nPriority;
};

enum EPostProcessPass
{
	ePostProcessPass_PreGUI,

	ePostProcessPass_Count,
};

class IRenderSystem;
class CPostProcessPass
{
public:
	CPostProcessPass() : m_pSystem( NULL ), m_pRenderTargetPool( NULL ), m_bFinalViewport( false ) {}
	void Register( CPostProcess* pPostProcess ) { m_vecPasses.push_back( pPostProcess ); }
	void RegisterOnPostProcess( CTrigger* pTrigger ) { m_onPostProcess.Register( 0, pTrigger ); }

	void Process( IRenderSystem* pSystem, CReference<ITexture>& pTarget, IRenderTarget* pFinalTarget );
	IRenderSystem* GetRenderSystem() { return m_pSystem; }
	CReference<ITexture>& GetTarget() { return m_pTarget; }
	CRenderTargetPool& GetRenderTargetPool() { return m_pRenderTargetPool ? *m_pRenderTargetPool : CRenderTargetPool::GetSizeDependentPool(); }
	void SetRenderTargetPool( CRenderTargetPool* pPool ) { m_pRenderTargetPool = pPool; }
	bool IsFinalViewport() { return m_bFinalViewport; }
	TRectangle<int32>& GetFinalViewport() { return m_finalViewport; }
	void SetFinalViewport( const TRectangle<int32>& rect ) { m_finalViewport = rect; m_bFinalViewport = true; }

	static CPostProcessPass* GetPostProcessPass( EPostProcessPass ePass )
	{
		static CPostProcessPass g_insts[ePostProcessPass_Count];
		if( ePass >= ePostProcessPass_Count )
			return NULL;
		return &g_insts[ePass];
	}
private:
	IRenderSystem* m_pSystem;
	vector<CPostProcess*> m_vecPasses;
	CReference<ITexture> m_pTarget;
	CRenderTargetPool* m_pRenderTargetPool;
	bool m_bFinalViewport;
	TRectangle<int32> m_finalViewport;

	CEventTrigger<1> m_onPostProcess;
};