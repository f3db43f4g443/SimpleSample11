#pragma once
#include "RenderObject2D.h"
#include "ClassMetaData.h"
#include "StringUtil.h"

class CPrefabBaseNode : public CRenderObject2D
{
	friend class CPrefabNode;
public:
	CPrefabBaseNode() : m_strName( "" ) { SET_BASEOBJECT_ID( CPrefabBaseNode ); }
	CPrefabBaseNode( const SClassCreateContext& context ) : m_strName( "" ) {}
	const CString& GetName() const { return m_strName; }
	void SetName( const char* szName ) { m_strName = szName; }
	void SetName( const CString& strName ) { m_strName = strName; }
	CResource* GetResource() { return m_pResource; }
	void SetResource( CResource* pResource ) { m_pResource = pResource; }
	CRenderObject2D* GetRenderObject() { return m_pRenderObject; }
	void ClearRenderObject() { m_pRenderObject = NULL; }
	virtual void SetRenderObject( CRenderObject2D* pRenderObject );
	virtual bool IsPreview() { return false; }
	virtual void OnPreview() {}
	void UpdatePreview();
	void DebugDraw( class CUIViewport* pViewport, IRenderSystem* pRenderSystem );

	template<class T = CPrefabBaseNode>
	T* GetChildByName( const char* szName )
	{
		for( auto pChild = Get_TransformChild(); pChild; pChild = pChild->NextTransformChild() )
		{
			auto pT = dynamic_cast<T*>( pChild );
			if( !pT )
				continue;
			if( pT->m_strName == szName )
				return pT;
		}
		return NULL;
	}

	template<class T = CPrefabBaseNode>
	T* GetChildByName_Fast( const char* szName )
	{
		for( auto pChild = Get_TransformChild(); pChild; pChild = pChild->NextTransformChild() )
		{
			auto pT = SafeCast<T>( pChild );
			if( !pT )
				continue;
			if( pT->m_strName == szName )
				return pT;
		}
		return NULL;
	}

	class CPrefab* GetInstanceOwner();
	class CPrefabNode* GetInstanceOwnerNode();
	static CRenderObject2D* CreateRenderObjectByResource( CResource* pResource, CRenderObject2D* pNode );
protected:
	virtual void OnUpdatePreview() {}
	virtual void OnDebugDraw( class CUIViewport* pViewport, IRenderSystem* pRenderSystem ) {}
	CString m_strName;
	CReference<CRenderObject2D> m_pRenderObject;
	CReference<CResource> m_pResource;
private:
	CReference<CResource> m_pInstanceOwner;
	CReference<CRenderObject2D> m_pInstanceOwnerNode;
};

struct CPrefabNodeNameSpace
{
	CPrefabNodeNameSpace() : pNameSpaceKey( NULL ), nLastID( 0 ) {}
	int32 FindIDByNode( CPrefabNode* pNode );
	int32 FindOrGenIDByNode( CPrefabNode* pNode );
	CPrefabNode* FindNodeByID( int32 nID );
	void Add( CPrefabNode* pNode, int32 nID );
	void Remove( CPrefabNode* pNode );
	void Fix( map<int32, int32>& result );

	void* pNameSpaceKey;
	map<int32, CReference<CPrefabNode> > mapIDToNode;
	map<CPrefabNode*, int32> mapNodeToID;
	int32 nLastID;

	struct SNameSpaceInfo
	{
		SNameSpaceInfo() : pCreatedInst( NULL ), pClassData( NULL ) {}
		SClassMetaData* pClassData;
		CRenderObject2D* pCreatedInst;
	};
	struct SObjPtrInfo
	{
		SObjPtrInfo( CPrefabNodeNameSpace* pNameSpace, SClassMetaData::SMemberData* pMemberData, int32 nID )
			: pNameSpace( pNameSpace ), pMemberData( pMemberData ), nID( nID ), pCreatedNodeData( NULL )
			, nCastOffset( pMemberData->pTypeData->GetBaseClassOfs( CClassMetaDataMgr::Inst().GetClassData<CRenderObject2D>() ) ) { }
		CPrefabNodeNameSpace* pNameSpace;
		SClassMetaData::SMemberData* pMemberData;
		int32 nID;
		int32 nCastOffset;
		uint8* pCreatedNodeData;
	};
	SNameSpaceInfo* AddNamespaceInfo( class CPrefabNode* pNode );
	void AddObjPtrInfo( SObjPtrInfo* pInfo )
	{
		vecInfos.push_back( pair<SNameSpaceInfo*, SObjPtrInfo*>( NULL, pInfo ) );
	}
	void CheckInfos();
	void FillData();
	void ClearInfo()
	{
		mapNameSpaceInfos.clear();
		vecInfos.clear();
	}
	map<int32, SNameSpaceInfo> mapNameSpaceInfos;
	vector<pair<SNameSpaceInfo*, SObjPtrInfo*> > vecInfos;
};

class CPrefabNode : public CPrefabBaseNode
{
	friend class CPrefabEditor;
public:
	CPrefabNode( class CPrefab* pPrefab ) : m_pPrefab( pPrefab ), m_nType( 0 ), m_bIsInstance( false ), m_bTaggedNodePtrInfoDirty( true ), m_bNamespaceDirty( true ),
		m_pPatchedNodeParent( NULL ), m_onResourceRefreshBegin( this, &CPrefabNode::OnResourceRefreshBegin ), m_onResourceRefreshEnd( this, &CPrefabNode::OnResourceRefreshEnd )
	{
		SetAutoUpdateAnim( true );
	}
	~CPrefabNode() { SetResource( NULL ); }
	CPrefab* GetPrefab() { return m_pPrefab; }
	void Invalidate();
	void AddRef1();
	void Release1();
	CResource* GetResource() { return m_pResource; }
	bool SetResource( CResource* pResource );
	SClassMetaData* GetClassData() { return m_obj.GetClassData(); }
	SClassMetaData* GetFinalClassData();
	CPrefabBaseNode* GetFinalObjData();
	bool SetClassName( const char* szName ) { return m_obj.SetClassName( szName ); }
	uint8* GetObjData() { return m_obj.GetObjData(); }
	bool IsInstance() { return m_bIsInstance; }
	static CResource* LoadResource( const char* szName );
	CPrefabNode* GetPatchedNode() { return m_pPatchedNode; }
	void OnEdit() { m_bTaggedNodePtrInfoDirty = true; m_obj.SetDirty(); }
	void OnEditorMove( CPrefabNode* pRoot );
	void DebugDrawPreview( class CUIViewport* pViewport, IRenderSystem* pRenderSystem );
	CPrefabNode* GetPatchedNodeOwner();
	void PatchedNodeForceCalcTransform();

	void OnEditorActive( bool bActive );

	void BindShaderResource( EShaderType eShaderType, const char* szName, IShaderResourceProxy* pShaderResource );

	struct SPatchContext
	{
		SPatchContext( CPrefabNode* pRoot ) : pRoot( pRoot ) {}
		CPrefabNode* pRoot;
		map<string, CBufFile> mapPatches;
	};
	struct SNameSpaceCopyContext
	{
		SNameSpaceCopyContext() : pSrc( NULL ), pDst( NULL ), pNext( NULL ) {}
		CPrefabNodeNameSpace* pSrc;
		CPrefabNodeNameSpace* pDst;
		SNameSpaceCopyContext* pNext;
	};
	CPrefabNode* Clone( CPrefab* pPrefab, CPrefabNodeNameSpace* pNameSpace = NULL, SPatchContext* pContext = NULL, SNameSpaceCopyContext* pNameSpaceCopyContext = NULL, bool bForRootElem = false );
	CRenderObject2D* CreateInstance( bool bNameSpace = true );
	CPrefabNodeNameSpace& GetNameSpace() { return m_nameSpace; }
	void NameSpaceClearNode( CPrefabNode* pNode );
	void FixNameSpace();

	template<class T>
	const T* GetStaticData()
	{
		UpdateDirty();
		return (const T*)m_obj.GetObjData();
	}

	template<class T>
	const T* GetStaticDataSafe()
	{
		UpdateDirty();
		return SafeCast<T>( GetFinalObjData() );
	}
	
	void Load( IBufReader& buf, CPrefabNodeNameSpace* pNameSpace );
	void Save( CBufFile& buf, CPrefabNodeNameSpace* pNameSpace );
	static void FormatNamespaceString( void* pt, int32 n, string& result );

	void ExtractData( IBufReader& bufIn, CBufFile& bufOut, function<void( CPrefabNode*, int8 )>& funcNodeHandler,
		function<bool( SClassMetaData*, SClassMetaData::SMemberData*, int32, IBufReader&, CBufFile& )>& funcDataHandler );
protected:
	virtual void OnDebugDraw( class CUIViewport* pViewport, IRenderSystem* pRenderSystem ) override;
	void GetPathString( CPrefabNode* pRoot, string& str );
	CRenderObject2D* CreateInstance( vector<CRenderObject2D*>& vecInst, bool bNameSpace );
	void OnResourceRefreshBegin();
	void OnResourceRefreshEnd();
	void LoadResourceExtraData( CResource* pResource, IBufReader& extraData, CPrefabNodeNameSpace* pNameSpace );
	void LoadOtherExtraData( IBufReader& extraData );
	void SaveResourceExtraData( CResource* pResource, CBufFile& extraData, CPrefabNodeNameSpace* pNameSpace );
	void SaveOtherExtraData( CBufFile& extraData );
	void ExtractResourceExtraData( IBufReader& bufIn, CBufFile& bufOut, function<void( CPrefabNode*, int8 )>& funcNodeHandler,
		function<bool( SClassMetaData*, SClassMetaData::SMemberData*, int32, IBufReader&, CBufFile& )>& funcDataHandler );
	void Diff( CPrefabNode* pNode, SPatchContext& context, CPrefabNodeNameSpace* pNameSpace );
	void ExtractPatchData( CPrefabNode* pPatchedNode, SPatchContext& contextIn, SPatchContext& contextOut, function<void( CPrefabNode*, int8 )> funcNodeHandler,
		function<bool( SClassMetaData*, SClassMetaData::SMemberData*, int32, IBufReader&, CBufFile& )>& funcDataHandler );

	void UpdateDirty()
	{
		if( m_bTaggedNodePtrInfoDirty )
		{
			UpdateTaggedNodePtrInfo();
			UpdateResPtrInfo();
			UpdatePrefabNodeRefInfo();
			m_bTaggedNodePtrInfoDirty = false;
		}
	}

	CPrefab* m_pPrefab;
	uint8 m_nType;
	bool m_bIsInstance;
	bool m_bTaggedNodePtrInfoDirty;
	bool m_bNamespaceDirty;
	TObjectPrototype<CPrefabBaseNode> m_obj;
	TClassTrigger<CPrefabNode> m_onResourceRefreshBegin;
	TClassTrigger<CPrefabNode> m_onResourceRefreshEnd;

	CReference<CPrefabNode> m_pPatchedNode;
	CReference<CPrefabBaseNode> m_pPreviewNode;
	CPrefabNode* m_pPatchedNodeParent;
	CPrefabNodeNameSpace m_nameSpace;

	struct STaggedNodePtrInfo
	{
		STaggedNodePtrInfo() {}
		STaggedNodePtrInfo( SClassMetaData::SMemberData* pMemberData, uint32 nOfs, uint32 nChild ) : pMemberData( pMemberData ), nOfs( nOfs ), nChild( nChild ) {}
		SClassMetaData::SMemberData* pMemberData;
		uint32 nOfs;
		uint32 nChild;
	};
	struct STaggedPrefabNodePtrInfo
	{
		STaggedPrefabNodePtrInfo() {}
		STaggedPrefabNodePtrInfo( SClassMetaData::SMemberData* pMemberData, uint32 nOfs, CPrefabNode* pPrefabNode )
			: pMemberData( pMemberData ), nOfs( nOfs ), pPrefabNode( pPrefabNode ) {}
		SClassMetaData::SMemberData* pMemberData;
		uint32 nOfs;
		CPrefabNode* pPrefabNode;
	};
	struct SResPtrInfo
	{
		SResPtrInfo() {}
		SResPtrInfo( SClassMetaData::SMemberData* pMemberData, uint32 nOfs, int8 nType ) : pMemberData( pMemberData ), nOfs( nOfs ), nType( nType ) {}
		SClassMetaData::SMemberData* pMemberData;
		uint32 nOfs;
		int8 nType;
	};
	struct SPrefabNodeRefInfo
	{
		SPrefabNodeRefInfo() {}
		SPrefabNodeRefInfo( SClassMetaData::SMemberData* pMemberData, uint32 nOfs, int8 nType, CPrefabNode* pPrefabNode )
			: pMemberData( pMemberData ), nOfs( nOfs ), nType( nType ), pPrefabNode( pPrefabNode ) {}
		SClassMetaData::SMemberData* pMemberData;
		uint32 nOfs;
		int8 nType;
		CPrefabNode* pPrefabNode;
	};

	vector<CPrefabNodeNameSpace::SNameSpaceInfo*> m_vecNameSpaceInfo;
	vector<CPrefabNodeNameSpace::SObjPtrInfo> m_vecObjPtrInfo;
	vector<SResPtrInfo> m_vecObjPtrTemp;
	struct SNameSpaceUpdateContext
	{
		SNameSpaceUpdateContext() : pNameSpace( NULL ), pNext( NULL ) {}
		CPrefabNodeNameSpace* pNameSpace;
		SNameSpaceUpdateContext* pNext;
	};
	void UpdateNameSpace( SNameSpaceUpdateContext* pContext = NULL );
	int32 CreateObjPtrInfo( SNameSpaceUpdateContext* pContext, uint8* pObjData, int32 n );
	void AddObjPtrInfo( SNameSpaceUpdateContext* pContext, uint8* p, SClassMetaData::SMemberData* pMemberData );
	int32 CopyObjPtrInfo( uint8* pDest, uint8* pObjData, int32 n );
	void CopyObjPtrInfo1( uint8* pDest, uint8* pObjData, SClassMetaData::SMemberData* pMemberData );
	void FixObjRef( map<int32, int32>& mapID, void* pNameSpaceKey, int8 nPass );
	int32 FixObjRefNode( map<int32, int32>& mapID, uint8* pObjData, int32 n, vector<SResPtrInfo>& vec, void* pNameSpaceKey, int8 nPass );
	void FixObjRefNode1( map<int32, int32>& mapID, uint8* pObjData, SClassMetaData::SMemberData* pMemberData, void* pNameSpaceKey, int8 nPass );

	vector<STaggedNodePtrInfo> m_vecTaggedNodePtrInfo;
	vector<STaggedPrefabNodePtrInfo> m_vecTaggedPrefabNodePtrInfo;
	void UpdateTaggedNodePtrInfo();
	void UpdateTaggedNodePtrInfo( uint32& nIndex, string curName, map<string, STaggedNodePtrInfo*>& mapInfo );

	vector<SResPtrInfo> m_vecResPtrInfo;
	void UpdateResPtrInfo();
	int32 CreateResPtr( uint8* pObjData, int32 n );
	int32 CopyResPtr( uint8* pDst, uint8* pObjData, int32 n );

	vector<SPrefabNodeRefInfo> m_vecPrefabNodeRefInfo;
	void UpdatePrefabNodeRefInfo();
	int32 CreatePrefabNodeRef( uint8* pObjData, int32 n );
	int32 CopyPrefabNodeRef( uint8* pDst, uint8* pObjData, int32 n );
};
typedef TCustomTaggedRef<CPrefabNode> CPrefabNodeRef;

class CPrefab : public CResource
{
public:
	enum EType
	{
		eResType = eEngineResType_Prefab,
	};
	CPrefab( const char* name, int32 type ) : CResource( name, type ) {}
	void Create();
	void Load( IBufReader& buf );
	void Save( CBufFile& buf );
	CPrefabNode* GetRoot() { return m_pRoot; }
	CPrefabNode* GetNode( const char* szName );
	void* GetNameSpaceKey();
	void ExtractData( CBufFile& bufOut, function<void( CPrefabNode*, int8 )>& funcNodeHandler,
		function<bool( SClassMetaData*, SClassMetaData::SMemberData*, int32, IBufReader&, CBufFile& )>& funcDataHandler );

	void SetNode( CPrefabNode* pNode, const char* szName = NULL );
	map<CString, CReference<CPrefabNode> >& GetAllExtraNodes() { return m_mapNodes; }
	void ClearExtraNode();
private:
	CReference<CPrefabNode> m_pRoot;
	map<CString, CReference<CPrefabNode> > m_mapNodes;
};