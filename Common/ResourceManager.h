#pragma once

#include "Resource.h"
#include <map>
#include <string>
using namespace std;

class CResourceFactory
{
public:
	virtual ~CResourceFactory() {}

	CResource* CreateResource( const char* name );
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
	template <typename T>
	T* CreateResource( const char* name );

	void RemoveRes( CResource* pRes );
	void Register( CResourceFactory* pFactory ) { m_mapFactories[pFactory->GetType()] = pFactory; }

	static CResourceManager* Inst();
private:
	CResourceManager();
	map<int, CResourceFactory*> m_mapFactories;
};

template <typename T>
T* CResourceManager::CreateResource( const char* name )
{
	return static_cast<T*>( m_mapFactories[T::eResType]->CreateResource( name ) );
}
