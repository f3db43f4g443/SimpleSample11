#pragma once
#include "Item.h"
#include "Common/xml.h"

struct SItemDrop
{
	CReference<CPrefab> pPrefab;
	uint32 nPrice;
};

struct SItemDropPoolItem
{
	SItemDrop drop;
	float p;
};

struct SItemDropContext
{
	uint32 nDrop;
	vector<SItemDropPoolItem> dropPool;
	vector<SItemDrop> dropItems;

	void Clear() { dropPool.clear(); dropItems.clear(); }
	void Drop();
};

class CItemDropNode : public CReferenceObject
{
public:
	virtual void Load( TiXmlElement* pXml, struct SItemDropNodeLoadContext& context ) {}
	virtual void Generate( SItemDropContext& context ) {}

	static CItemDropNode* CreateNode( TiXmlElement* pXml, struct SItemDropNodeLoadContext& context );
};

struct SItemDropNodeLoadContext
{
	map<string, CReference<CItemDropNode> > mapNodes;
	CItemDropNode* FindNode( const char* szName )
	{
		auto itr = mapNodes.find( szName );
		if( itr != mapNodes.end() )
			return itr->second;
		return NULL;
	}
};

class CItemDropNodeFactory
{
public:
	CItemDropNodeFactory();
	CItemDropNode* LoadNode( TiXmlElement* pXml, struct SItemDropNodeLoadContext& context );

	DECLARE_GLOBAL_INST_REFERENCE( CItemDropNodeFactory )
private:
	map<string, function<CItemDropNode*( TiXmlElement* pXml )> > m_mapCreateFuncs;
};

class CItemDropSimple : public CItemDropNode
{
public:
	virtual void Load( TiXmlElement* pXml, struct SItemDropNodeLoadContext& context ) override;
	virtual void Generate( SItemDropContext& context ) override;
private:
	CReference<CPrefab> m_pPrefab;
	uint32 m_nPrice;
};

class CItemDropSimpleGroup : public CItemDropNode
{
public:
	virtual void Load( TiXmlElement* pXml, struct SItemDropNodeLoadContext& context ) override;
	virtual void Generate( SItemDropContext& context ) override;
private:
	void _Generate( SItemDropContext& context );
	bool m_bSector;
	bool m_bAverage;

	struct SSubItem
	{
		CReference<CItemDropNode> pNode;
		float fChance;
		bool bNormalize;
	};
	vector<SSubItem> m_subItems;
};