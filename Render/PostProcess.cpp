#include "stdafx.h"
#include "PostProcess.h"
#include <algorithm>

bool CPostProcessPass::PreProcess( IRenderSystem* pSystem )
{
	m_onPostProcess.Trigger( 0, this );
	if( m_vecPasses.size() )
		return true;
	return false;
}

void CPostProcessPass::Process( IRenderSystem* pSystem, CReference<ITexture>& pTarget, IRenderTarget* pFinalTarget )
{
	m_pSystem = pSystem;
	m_pTarget = pTarget;
	pTarget = NULL;
	struct SLess
	{
		bool operator () ( CPostProcess* pLeft, CPostProcess* pRight )
		{
			if( pLeft->IsForceFirstPass() && !pRight->IsForceFirstPass() )
				return true;
			if( !pLeft->IsForceFirstPass() && pRight->IsForceFirstPass() )
				return false;
			return pLeft->GetPriority() > pRight->GetPriority();
		}
	};
	std::sort( m_vecPasses.begin(), m_vecPasses.end(), SLess() );

	if( m_pTarget && m_pTarget->GetRenderTarget() == pFinalTarget )
		pFinalTarget = NULL;

	for( int i = 0; i < m_vecPasses.size(); i++ )
	{
		auto pPostProcess = m_vecPasses[i];
		pPostProcess->Process( this, i == m_vecPasses.size() - 1 ? pFinalTarget : NULL );
	}

	pTarget = m_pTarget;
	m_vecPasses.clear();
	m_pSystem = NULL;
	m_pTarget = NULL;
}