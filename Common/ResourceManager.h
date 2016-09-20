#pragma once

#include "Resource.h"
#include <map>
#include <string>
using namespace std;

class CResourceFactory
{
public:
	virtual ~CResourceFactory() {}

	CResource* CreateResource( const char* name, bool bNew );
	void RemoveRes( CResource* pRes );

	virtual int GetType() = 0;
protected:
	virtual CResource* Create( const char* name ) = 0;
private:
	map<string, CResource*> m_mapRes;
};

template <typename T>
class TResourceFactory : public CResourceFactory
{
public:
	int GetType() { return T::eResType; }
protected:
	CResource* Create( const char* name )
	{
		return new T( name, T::eResType );
	}
};

class CResourceManager
{
public:
	template <typename T = CResource>
	T* CreateResource( const char* name, bool bNew = false );

	void RemoveRes( CResource* pRes );
	void Register( CResourceFactory* pFactory ) { m_mapFactories[pFactory->GetType()] = pFactory; }

	void RegisterExtension( int32 nResType, const char* szExt ) { m_mapExts[szExt] = nResType; }
	template <class T>
	void RegisterExtension( const char* szExt ) { RegisterExtension( T::eResType, szExt ); }

	DECLARE_GLOBAL_INST_POINTER( CResourceManager )
private:
	CResourceManager();
	map<int32, CResourceFactory*> m_mapFactories;
	map<string, int32> m_mapExts;
};

template <>
CResource* CResourceManager::CreateResource( const char* name, bool bNew );

template <typename T>
T* CResourceManager::CreateResource( const char* name, bool bNew )
{
	return static_cast<T*>( m_mapFactories[T::eResType]->CreateResource( name, bNew ) );
}
