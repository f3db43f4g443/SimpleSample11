#pragma once
#include "RenderObject2D.h"
#include "ClassMetaData.h"
#include "StringUtil.h"

class CPrefabBaseNode : public CRenderObject2D
{
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

	template<class T = CPrefabBaseNode>
	T* GetChildByName( const char* szName )
	{
		for( auto pChild = Get_Child(); pChild; pChild = pChild->NextChild() )
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
	CString m_strName;
	CReference<CRenderObject2D> m_pRenderObject;
	CReference<CResource> m_pResource;
};

class CPrefabNode : public CPrefabBaseNode
{
	friend class CPrefabEditor;
public:
	CPrefabNode( class CPrefab* pPrefab ) : m_pPrefab( pPrefab ), m_nType( 0 ), m_bTaggedNodePtrInfoDirty( true ),
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

	void BindShaderResource( EShaderType eShaderType, const char* szName, IShaderResourceProxy* pShaderResource );

	CPrefabNode* Clone( bool bForEditor = false );
	CPrefabNode* Clone( CPrefab* pPrefab );
	CRenderObject2D* CreateInstance();

	template<class T>
	const T* GetStaticData()
	{
		return (const T*)m_obj.GetObjData();
	}

	template<class T>
	const T* GetStaticDataSafe()
	{
		return SafeCast<T>( GetStaticData<CPrefabBaseNode>() );
	}
	
	void Load( IBufReader& buf );
	void Save( CBufFile& buf );
private:
	void OnResourceRefreshBegin();
	void OnResourceRefreshEnd();

	CPrefab* m_pPrefab;
	uint8 m_nType;
	TObjectPrototype<CPrefabBaseNode> m_obj;
	TClassTrigger<CPrefabNode> m_onResourceRefreshBegin;
	TClassTrigger<CPrefabNode> m_onResourceRefreshEnd;

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

	CRenderObject2D* CreateInstance( vector<CRenderObject2D*>& vecInst );
};

class CPrefab : public CResource
{
	friend class CPrefabEditor;
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
private:
	CReference<CPrefabNode> m_pRoot;
};