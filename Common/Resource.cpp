#include "Common.h"
#include "Resource.h"
#include "ResourceManager.h"

void CResource::Dispose()
{
	CResourceManager::Inst()->RemoveRes( this );

	CReferenceObject::Dispose();
}
