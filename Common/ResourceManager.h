#pragma once

#include "Resource.h"
#include "StringUtil.h"
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
	CResource* CreateResource( uint8 nType, const char* name, bool bNew = false );

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

template <class T>
class TResourceRef : public CString
{
public:
	TResourceRef() {}
	TResourceRef( const char* c ) : CString( c ) {}
	TResourceRef( const CString& str ) : CString( str ) {}
	TResourceRef( const TResourceRef<T>& ref ) : CString( ref ), m_pRef( ref.m_pRef ) {}
	TResourceRef( const struct SClassCreateContext& context ) : CString( context ) {}

	TResourceRef<T>& operator = ( const TResourceRef<T>& rhs )
	{
		CString::operator=( rhs );
		m_pRef = rhs.m_pRef;
		return *this;
	}
	TResourceRef<T>& operator = ( T* rhs )
	{
		m_pRef = rhs;
		return *this;
	}

	T& operator * () const {
		return *m_pRef.GetPtr();
	}
	T* operator -> () const {
		return m_pRef.GetPtr();
	}
	operator T* () const {
		return (T*)m_pRef;
	}
	operator bool() const {
		return m_pRef.GetPtr() != NULL;
	}
	T* GetPtr() const { return m_pRef.GetPtr(); }

	static int32 GetPtrOfs() { return MEMBER_OFFSET( TResourceRef<T>, m_pRef ); }

	void Create() { m_pRef = CResourceManager::Inst()->CreateResource<T>( c_str() ); }
private:
	CReference<T> m_pRef;
};