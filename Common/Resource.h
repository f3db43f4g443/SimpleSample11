#pragma once

#include "Reference.h"
#include "LinkList.h"
#include <string>
using namespace std;

class CResource : public CReferenceObject
{
	friend class CResourceFactory;
	int32 m_type;
	string m_name;
public:
	CResource( const char* name, int32 type ) : m_name( name ), m_type( type ) {}

	virtual void Create() = 0;

	int32 GetResourceType() const { return m_type; }
	const char* GetName() const { return m_name.c_str(); }
protected:
	void Dispose();
};
