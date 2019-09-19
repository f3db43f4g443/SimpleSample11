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

	static CRenderObject2D* CreateRenderObjectByResource( CResource* pResource, CRenderObject2D* pNode );
protected:
	virtual void OnUpdatePreview() {}
	virtual void OnDebugDraw( class CUIViewport* pViewport, IRenderSystem* pRenderSystem ) {}
	CString m_strName;
	CReference<CRenderObject2D> m_pRenderObject;
	CReference<CResource> m_pResource;
};

class CPrefabNode : public CPrefabBaseNode
{
	friend class CPrefabEditor;
public:
	CPrefabNode( class CPrefab* pPrefab ) : m_pPrefab( pPrefab ), m_nType( 0 ), m_bIsInstance( false ), m_bTaggedNodePtrInfoDirty( true ),
		m_onResourceRefreshBegin( this, &CPrefabNode::OnResourceRefreshBegin ), m_onResourceRefreshEnd( this, &CPrefabNode::OnResourceRefreshEnd )
	{
		SetAutoUpdateAnim( true );
	}
	~CPrefabNode() { SetResource( NULL ); }
	CResource* GetResource() { return m_pResource; }
	bool SetResource( CResource* pResource );
	SClassMetaData* GetClassData() { return m_obj.GetClassData(); }
	SClassMetaData* GetFinalClassData();
	bool SetClassName( const char* szName ) { return m_obj.SetClassName( szName ); }
	uint8* GetObjData() { return m_obj.GetObjData(); }
	bool IsInstance() { return m_bIsInstance; }
	static CResource* LoadResource( const char* szName );
	CPrefabNode* GetPatchedNode() { return m_pPatchedNode; }
	void OnEdit() { m_bTaggedNodePtrInfoDirty = true; m_obj.SetDirty(); }
	void DebugDrawPreview( class CUIViewport* pViewport, IRenderSystem* pRenderSystem );

	void OnEditorActive( bool bActive );

	void BindShaderResource( EShaderType eShaderType, const char* szName, IShaderResourceProxy* pShaderResource );

	struct SPatchContext
	{
		SPatchContext( CPrefabNode* pRoot ) : pRoot( pRoot ) {}
		CPrefabNode* pRoot;
		map<string, CBufFile> mapPatches;
	};
	CPrefabNode* Clone( bool bForEditor = false );
	CPrefabNode* Clone( CPrefab* pPrefab, SPatchContext* pContext = NULL );
	CRenderObject2D* CreateInstance();

	template<class T>
	const T* GetStaticData()
	{
		if( m_bTaggedNodePtrInfoDirty )
		{
			UpdateTaggedNodePtrInfo();
			UpdateResPtrInfo();
			m_bTaggedNodePtrInfoDirty = false;
		}
		return (const T*)m_obj.GetObjData();
	}

	template<class T>
	const T* GetStaticDataSafe()
	{
		if( m_bTaggedNodePtrInfoDirty )
		{
			UpdateTaggedNodePtrInfo();
			UpdateResPtrInfo();
			m_bTaggedNodePtrInfoDirty = false;
		}
		return SafeCast<T>( m_obj.GetObject() );
	}
	
	void Load( IBufReader& buf );
	void Save( CBufFile& buf );
protected:
	virtual void OnDebugDraw( class CUIViewport* pViewport, IRenderSystem* pRenderSystem ) override;
	void GetPathString( CPrefabNode* pRoot, string& str );
	CRenderObject2D* CreateInstance( vector<CRenderObject2D*>& vecInst );
	void OnResourceRefreshBegin();
	void OnResourceRefreshEnd();
	void LoadResourceExtraData( CResource* pResource, IBufReader& extraData );
	void LoadOtherExtraData( IBufReader& extraData );
	void SaveResourceExtraData( CResource* pResource, CBufFile& extraData );
	void SaveOtherExtraData( CBufFile& extraData );

	void Diff( CPrefabNode* pNode, SPatchContext& context );

	CPrefab* m_pPrefab;
	uint8 m_nType;
	bool m_bIsInstance;
	TObjectPrototype<CPrefabBaseNode> m_obj;
	TClassTrigger<CPrefabNode> m_onResourceRefreshBegin;
	TClassTrigger<CPrefabNode> m_onResourceRefreshEnd;

	CReference<CPrefabNode> m_pPatchedNode;
	CReference<CPrefabBaseNode> m_pPreviewNode;

	struct STaggedNodePtrInfo
	{
		STaggedNodePtrInfo( SClassMetaData::SMemberData* pMemberData, uint32 nOfs, uint32 nChild ) : pMemberData( pMemberData ), nOfs( nOfs ), nChild( nChild ) {}
		SClassMetaData::SMemberData* pMemberData;
		uint32 nOfs;
		uint32 nChild;
	};
	bool m_bTaggedNodePtrInfoDirty;
	vector<STaggedNodePtrInfo> m_vecTaggedNodePtrInfo;
	void UpdateTaggedNodePtrInfo();
	void UpdateTaggedNodePtrInfo( uint32& nIndex, string curName, map<string, STaggedNodePtrInfo*>& mapInfo );

	struct SResPtrInfo
	{
		SResPtrInfo( SClassMetaData::SMemberData* pMemberData, uint32 nOfs, int8 nType ) : pMemberData( pMemberData ), nOfs( nOfs ), nType( nType ) {}
		SClassMetaData::SMemberData* pMemberData;
		uint32 nOfs;
		int8 nType;
	};
	vector<SResPtrInfo> m_vecResPtrInfo;
	void UpdateResPtrInfo();
	int32 CreateResPtr( uint8* pObjData, int32 n );
	int32 CopyResPtr( uint8* pDst, uint8* pObjData, int32 n );
};

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

	void SetNode( CPrefabNode* pNode ) { m_pRoot = pNode; }
private:
	CReference<CPrefabNode> m_pRoot;
};