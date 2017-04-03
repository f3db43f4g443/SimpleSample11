#pragma once
#include "Common/xml.h"
#include "Block.h"

class CMyLevel;
struct SLevelBuildContext
{
	SLevelBuildContext( CMyLevel* pLevel, SChunk* pParentChunk = NULL );
	SChunk* CreateChunk( SChunkBaseInfo& baseInfo, const TRectangle<int32>& region );
	void AttachPrefab( CPrefab* pPrefab, TRectangle<int32> rect, uint8 nLayer, uint8 nType );
	void AddSpawnInfo( SChunkSpawnInfo* pInfo, const TVector2<int32> ofs );
	void Build();

	SBlockLayer*& GetBlock( uint32 x, uint32 y, uint32 z );

	uint32 nWidth;
	uint32 nHeight;
	vector<SBlockLayer*> blocks;
	vector<SChunk*> chunks;
	vector<pair<CReference<CPrefab>, TRectangle<int32> > > attachedPrefabs[SBlock::eAttachedPrefab_Count];

	vector<int8> blueprint;
	map<string, int8> mapTags;

	CMyLevel* pLevel;
	SChunk* pParentChunk;
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

struct SLevelGenerateFileContext
{
	SLevelGenerateFileContext() : bValid( false ) {}
	bool bValid;
	string strFileName;
	string strFullPath;
	map<string, SLevelGenerateFileContext*> mapIncludeFiles;
	vector<pair<SLevelGenerateFileContext*, bool> > vecIncludeFiles;
	map<string, SChunkBaseInfo> mapChunkBaseInfo;
	map<string, CReference<CLevelGenerateNode> > mapNamedNodes;

	void AddInclude( SLevelGenerateFileContext* pContext, bool bPublic );
	CLevelGenerateNode* FindNode( const char* szNode );
};

struct SLevelGenerateNodeLoadContext
{
	const char* szDefaultType;
	vector<string> vecPaths;
	map<string, SLevelGenerateFileContext> mapFiles;

	SLevelGenerateFileContext* pCurFileContext;

	SLevelGenerateFileContext* FindFile( const char* szFileName );
	SLevelGenerateFileContext* LoadFile( const char* szFileName, const char* szPath );
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


class CLevelGenerateSimpleNode : public CLevelGenerateNode
{
public:
	virtual void Load( TiXmlElement* pXml, SLevelGenerateNodeLoadContext& context ) override;
	virtual void Generate( SLevelBuildContext& context, const TRectangle<int32>& region ) override;
protected:
	SChunkBaseInfo* m_pChunkBaseInfo;
	bool m_bIsLevelBarrier;

	CReference<CLevelGenerateNode> m_pSubChunk;
};