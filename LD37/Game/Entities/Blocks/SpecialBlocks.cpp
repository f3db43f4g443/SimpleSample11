#include "stdafx.h"
#include "SpecialBlocks.h"
#include "MyLevel.h"
#include "Common/ResourceManager.h"
#include "Common/Rand.h"

void CTriggerChunk::OnAddedToStage()
{
	m_pPrefab = CResourceManager::Inst()->CreateResource<CPrefab>( m_strPrefab.c_str() );
	m_pPrefab1 = CResourceManager::Inst()->CreateResource<CPrefab>( m_strPrefab1.c_str() );
	CChunkObject::OnAddedToStage();
}