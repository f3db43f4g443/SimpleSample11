#include "stdafx.h"
#include "LevelEditToolset.h"

CLevelEditPrefabToolset* CLevelEditToolsetMgr::GetPrefabToolset( SClassMetaData* pMetaData )
{
	for( ;; )
	{
		auto itr = m_mapPrefabToolset.find( pMetaData->strClassName );
		if( itr != m_mapPrefabToolset.end() )
			return itr->second;
		if( !pMetaData->vecBaseClassData.size() )
			break;
		pMetaData = pMetaData->vecBaseClassData[0].pBaseClass;
	}
	return NULL;
}

CLevelEditObjectToolset* CLevelEditToolsetMgr::GetObjectToolset( SClassMetaData* pMetaData )
{
	for( ;; )
	{
		auto itr = m_mapObjectToolset.find( pMetaData->strClassName );
		if( itr != m_mapObjectToolset.end() )
			return itr->second;
		if( !pMetaData->vecBaseClassData.size() )
			break;
		pMetaData = pMetaData->vecBaseClassData[0].pBaseClass;
	}
	return NULL;
}