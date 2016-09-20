#include "Common.h"
#include "ResourceManager.h"
#include "FileUtil.h"

CResourceManager::CResourceManager()
{
}

CResource* CResourceFactory::CreateResource( const char* name, bool bNew )
{
	map<string, CResource*>::iterator itr = m_mapRes.find( name );
	if( itr != m_mapRes.end() )
	{
		return itr->second;
	}

	CResource* pRes = Create( name );
	pRes->Create();
	if( !bNew && !pRes->IsCreated() )
	{
		pRes->AddRef();
		pRes->Release();
		return NULL;
	}

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

template <>
CResource* CResourceManager::CreateResource( const char* name, bool bNew )
{
	const char* szExt = GetFileExtension( name );
	auto itr = m_mapExts.find( szExt );
	if( itr != m_mapExts.end() )
		return m_mapFactories[itr->second]->CreateResource( name, bNew );
	return NULL;
}
