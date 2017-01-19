#pragma once
#include "Common/xml.h"
#include "Block.h"

class CMyLevel;
struct SLevelBuildContext
{
	SLevelBuildContext( CMyLevel* pLevel );
	SChunk* CreateChunk( SChunkBaseInfo& baseInfo, const TRectangle<int32>& region );
	void AttachPrefab( CPrefab* pPrefab, TRectangle<int32> rect );
	void AddSpawnInfo( SChunkSpawnInfo* pInfo, const TVector2<int32> ofs );
	void Build();

	vector<SBlock*> blocks;
	vector<SChunk*> chunks;
	vector<pair<CReference<CPrefab>, TRectangle<int32> > > attachedPrefabs;
	CMyLevel* pLevel;
};

class CLevelGenerateNode : public CReferenceObject
{
public:
	virtual void Load( TiXmlElement* pXml, struct SLevelGenerateNodeLoadContext& context );
	virtual void Generate( SLevelBuildContext& context, const TRectangle<int32>& region );

	static CLevelGenerateNode* CreateNode( TiXmlElement* pXml, struct SLevelGenerateNodeLoadContext& context );
protected:
	CReference<CLevelGenerateNode> m_pNextLevel;
	float m_fNextLevelChance;
};

struct SLevelGenerateNodeLoadContext
{
	const char* szDefaultType;
	map<string, SChunkBaseInfo> mapChunkBaseInfo;
	map<string, CReference<CLevelGenerateNode> > mapNamedNodes;
};

class CLevelGenerateFactory
{
public:
	CLevelGenerateFactory();
	CLevelGenerateNode* LoadNode( TiXmlElement* pXml, struct SLevelGenerateNodeLoadContext& context );

	DECLARE_GLOBAL_INST_REFERENCE( CLevelGenerateFactory )
private:
	map<string, function<CLevelGenerateNode*( TiXmlElement* pXml )> > m_mapCreateFuncs;
};