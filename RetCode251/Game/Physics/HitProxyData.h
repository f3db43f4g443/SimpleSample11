#pragma once
#include "HitProxy.h"
#include "Common/Reference.h"
#include <vector>
using namespace std;

class TiXmlElement;
struct SHitProxyData
{
public:
	SHitProxyData() : nItems( 0 ), pItems( NULL ) {}
	~SHitProxyData() { if( pItems ) free( pItems ); }
	struct SItem
	{
		uint8 nType;
		union
		{
			struct
			{
				float fRadius;
				CVector2 center;
			};
			struct
			{
				uint32 nVertices;
				CVector2 vertices[MAX_POLYGON_VERTEX_COUNT];
			};
		};
	};
	CVector2 GetCenter();

	void LoadXml( TiXmlElement* pRoot );
	SItem* pItems;
	uint32 nItems;
};

class CHitProxyDataSet : public CReferenceObject
{
public:
	SHitProxyData* GetData( uint32 nIndex ) { return nIndex < m_data.size() ? &m_data[nIndex] : NULL; }
	void LoadXml( TiXmlElement* pRoot );
private:
	vector<SHitProxyData> m_data;
};