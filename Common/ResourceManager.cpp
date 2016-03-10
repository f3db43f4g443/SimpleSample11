#include "Common.h"
#include "ResourceManager.h"

CResourceManager::CResourceManager()
{
}

CResource* CResourceFactory::CreateResource( const char* name )
{
	map<string, CResource*>::iterator itr = m_mapRes.find( name );
	if( itr != m_mapRes.end() )
	{
		return itr->second;
	}

	CResource* pRes = Create( name );
	pRes->Create();
	m_mapRes[name] = pRes;
	return pRes;
}

void CResourceFactory::RemoveRes( CResource* pRes )
{
	m_mapRes.erase( pRes->GetName() );
}

void CResourceManager::RemoveRes( CResource* pRes )
{
	m_mapFactories[pRes->GetResourceType()]->RemoveRes( pRes );
}

CResourceManager* CResourceManager::Inst()
{
	static CResourceManager g_inst;
	return &g_inst;
}
