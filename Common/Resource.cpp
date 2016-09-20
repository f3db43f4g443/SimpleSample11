#include "Common.h"
#include "Resource.h"
#include "ResourceManager.h"

void CResource::Dispose()
{
	CResourceManager::Inst()->RemoveRes( this );

	CReferenceObject::Dispose();
}

bool CResource::CanAddDependency( CResource* pResource )
{
	return !pResource->CheckDependency( this );
}

bool CResource::AddDependency( CResource* pResource )
{
	if( !CanAddDependency( pResource ) )
		return false;
	auto itr = m_mapDependencies.find( pResource );
	if( itr != m_mapDependencies.end() )
		itr->second++;
	else
		m_mapDependencies[pResource] = 1;
	return true;
}

bool CResource::RemoveDependency( CResource* pResource )
{
	auto itr = m_mapDependencies.find( pResource );
	if( itr == m_mapDependencies.end() )
		return false;
	itr->second--;
	if( !itr->second )
		m_mapDependencies.erase( itr );
	return true;
}

bool CResource::CheckDependency( CResource* pResource )
{
	if( this == pResource )
		return true;
	for( auto item : m_mapDependencies )
	{
		if( item.first->CheckDependency( pResource ) )
			return true;
	}
	return false;
}