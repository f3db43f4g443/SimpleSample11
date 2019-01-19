#pragma once
#include "Common/xml.h"
#include "Block.h"

class CMyLevel;
struct SLevelBuildContext
{
	SLevelBuildContext();
	SLevelBuildContext( CMyLevel* pLevel, SChunk* pParentChunk = NULL );
	SLevelBuildContext( const SLevelBuildContext& par, SChunk* pParentChunk = NULL );
	SLevelBuildContext( uint32 nWidth, uint32 nHeight, bool bTest = false );
	void Set( const SLevelBuildContext& par, SChunk* pParentChunk );
	SChunk* CreateChunk( SChunkBaseInfo& baseInfo, const TRectangle<int32>& region, SLevelBuildContext* pSubContext );
	void CreateChain( SChainBaseInfo& baseInfo, int32 x, int32 y1, int32 y2 );
	void AttachPrefab( CPrefab* pPrefab, TRectangle<int32> rect, uint8 nLayer, uint8 nType, bool bType1 );
	void AddSpawnInfo( SChunkSpawnInfo* pInfo, const TVector2<int32>& ofs );
	void PushScrollObj( CPrefab* pPrefab, uint32 nType );
	void Build();

	SBlockLayer*& GetBlock( uint32 x, uint32 y, uint32 z );

	uint32 nWidth;
	uint32 nHeight;
	vector<SBlockLayer*> blocks;
	vector<SChunk*> chunks;
	vector<SChain*> chains;
	struct SAttach
	{
		CReference<CPrefab> pPrefab;
		TRectangle<int32> rect;
		bool bType;
	};
	vector<SAttach> attachedPrefabs[SBlock::eAttachedPrefab_Count];
	vector<CReference<CPrefab> > scrollObjs[3];
	uint32 nMaxChunkHeight;

	vector<int8> blueprint;
	map<string, int8> mapTags;
	map<string, SChunk*> mapChunkNames;
	string strChunkName;
	SChunk* pLastLevelBarrier;

	CMyLevel* pLevel;
	SChunk* pParentChunk;
	uint32 nBlockSize;
	uint32 nSeed;
	bool bTest;
};

class CLevelGenerateNode : public CReferenceObject
{
public:
	virtual void Load( TiXmlElement* pXml, struct SLevelGenerateNodeLoadContext& context );
	virtual void Generate( SLevelBuildContext& context, const TRectangle<int32>& region );

	static CLevelGenerateNode* CreateNode( TiXmlElement* pXml, struct SLevelGenerateNodeLoadContext& context );

	enum
	{
		eEditType_Brush,
		eEditType_Fence,
		eEditType_Chain,
	};
	struct SMetadata
	{
		SMetadata() : bIsDesignValid( false ), minSize( 1, 1 ), maxSize( 1, 1 ), nMinLevel( 0 ), nMaxLevel( -1 ), nEditType( eEditType_Brush ), nSeed( 0 )
		{
			nDefaultChainType[0] = nDefaultChainType[1] = 0;
		}
		uint8 GetLayerType() const
		{
			if( nMinLevel == 0 && nMaxLevel == 0 )
				return 1;
			else if( nMinLevel == 1 && nMaxLevel == 1 )
				return 2;
			else if( nMinLevel == 0 && nMaxLevel == 1 )
				return 3;
			else
				return 0;
		}

		struct SType
		{
			string strName;
			int32 nType;
			int8 nChainType[2];
		};
		int8 GetChainType( int32 nType, uint8 nLayer ) const;
		CVector4 GetEditColor( int32 nType ) const;

		bool bIsDesignValid;
		int8 nMinLevel, nMaxLevel;
		int8 nEditType;
		TVector2<int32> minSize;
		TVector2<int32> maxSize;
		uint32 nSeed;
		int8 nDefaultChainType[2];
		vector<SType> vecTypes;
	};
	const SMetadata& GetMetadata() { return m_metadata; }

	SChunk* AddSubChunk( CChunkObject* pChunkObject, const TRectangle<int32>& region );
protected:
	int32 m_nFillBlueprint;
	CReference<CLevelGenerateNode> m_pNextLevel;
	float m_fNextLevelChance;
	SMetadata m_metadata;
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
	uint8 m_nLevelBarrierType;
	uint8 m_nLevelBarrierHeight;
	uint8 m_bCopyBlueprint;

	CReference<CLevelGenerateNode> m_pSubChunk;
};