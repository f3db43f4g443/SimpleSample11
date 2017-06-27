#pragma once

#include "Reference.h"
#include "LinkList.h"
#include "Trigger.h"
#include "BufFile.h"
#include <string>
using namespace std;

class CResource : public CReferenceObject, public CEventTrigger<2>
{
	friend class CResourceFactory;
	int32 m_type;
	string m_name;
protected:
	bool m_bCreated;
public:
	CResource( const char* name, int32 type ) : m_name( name ), m_type( type ), m_bCreated( false ) {}

	virtual void Create() = 0;

	int32 GetResourceType() const { return m_type; }
	const char* GetName() const { return m_name.c_str(); }
	bool IsCreated() const { return m_bCreated; }

	bool CanAddDependency( CResource* pResource );
	bool AddDependency( CResource* pResource );
	bool RemoveDependency( CResource* pResource );
	void ClearDependency() { m_mapDependencies.clear(); }

	void RegisterRefreshBegin( CTrigger* pTrigger ) { Register( 0, pTrigger ); }
	void RegisterRefreshEnd( CTrigger* pTrigger ) { Register( 1, pTrigger ); }
	void RefreshBegin() { Trigger( 0, NULL ); }
	void RefreshEnd() { Trigger( 1, NULL ); }
	virtual void Save( CBufFile& buf ) {}
protected:
	bool CheckDependency( CResource* pResource );
	void Dispose();
	map<CResource*, uint32> m_mapDependencies;
};