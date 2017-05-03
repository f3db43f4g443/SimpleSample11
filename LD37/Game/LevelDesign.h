#pragma once
#include "LevelGenerate.h"
#include "UICommon/UITreeView.h"
#include "UICommon/UIViewport.h"

class CChunkPreview : public CEntity
{
public:
	CChunkPreview() : m_pChunk( NULL ) { SET_BASEOBJECT_ID( CChunkPreview ); }
	CChunkPreview( const SClassCreateContext& context ) : CEntity( context ), m_pChunk( NULL ) { SET_BASEOBJECT_ID( CChunkPreview ); }
	virtual void OnRemovedFromStage() override { Clear(); }
	virtual void Set( CLevelGenerateNode* pNode, const TRectangle<int32>& region );
	virtual void Clear();
protected:
	SChunk* m_pChunk;
	CReference<CChunkObject> m_pChunkObject;
};

class CChunkEdit : public CChunkPreview
{
	friend void RegisterGameClasses();
public:
	CChunkEdit( const SClassCreateContext& context ) : CChunkPreview( context ) { SET_BASEOBJECT_ID( CChunkEdit ); }
	virtual void Set( CLevelGenerateNode* pNode, const TRectangle<int32>& region ) override;
private:
	void UpdateColor();

	CReference<CRenderObject2D> m_pFrameImg[8];
};

struct SLevelDesignItem : public CReferenceObject
{
	~SLevelDesignItem()
	{
		if( pEntity )
			pEntity->SetParentEntity( NULL );
	}

	string strFullName;
	CReference<CLevelGenerateNode> pGenNode;
	CReference<CChunkEdit> pEntity;
	TRectangle<int32> region;
};

struct SLevelDesignContext
{
	SLevelDesignContext( uint32 nWidth, uint32 nHeight ) : nWidth( nWidth ), nHeight( nHeight )
	{
		items.resize( nWidth * nHeight );
	}

	SLevelDesignItem* AddItem( CLevelGenerateNode* pNode, const TRectangle<int32>& region );
	void RemoveItem( SLevelDesignItem* pItem );

	uint32 nWidth, nHeight;
	vector<CReference<SLevelDesignItem> > items;
};

class CDesignLevel : public CEntity, protected SLevelDesignContext
{
	friend void RegisterGameClasses();
public:
	CDesignLevel( const SClassCreateContext& context ) : CEntity( context ), SLevelDesignContext( 32, 128 ), m_strChunkEditPrefab( context ) { SET_BASEOBJECT_ID( CDesignLevel ); }

	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;

	void Add( const char* szFullName, const TRectangle<int32>& region );
	void Remove( SLevelDesignItem* pItem ) { RemoveItem( pItem ); }

	CLevelGenerateNode* FindNode( const char* szFullName );
	void GenerateLevel( class CMyLevel* pLevel );

	void Load( IBufReader& buf );
	void Save( CBufFile& buf );

	CEntity* GetChunkRoot() { return m_pChunkRoot; }
	CEntity* GetChunkRoot1() { return m_pChunkRoot1; }

	static CDesignLevel* GetInst() { return s_pLevel; }
private:
	CString m_strChunkEditPrefab;
	CReference<CPrefab> m_pChunkEditPrefab;

	CReference<CEntity> m_pChunkRoot;
	CReference<CEntity> m_pChunkRoot1;
	CReference<CEntity> m_pChunkEditRoot;

	map<string, CReference<CLevelGenerateNode> > m_mapGenerateNodes;

	static CDesignLevel* s_pLevel;
};

class CDesignView : public CUIElement
{
public:
	CDesignView() : m_pSelectedFile( NULL ), m_pSelectedNode( NULL ) {}

	void SelectFile( class CDesignViewFileElem* pItem );
	void SelectNode( class CDesignViewNodeElem* pItem );
protected:
	virtual void OnInited() override;
	virtual void OnSetVisible( bool bVisible ) override;
private:
	CReference<CUIViewport> m_pMainViewport;
	CReference<CUITreeView> m_pFileView;
	CReference<CUIScrollView> m_pNodeView;

	class CDesignViewFileElem* m_pSelectedFile;
	class CDesignViewNodeElem* m_pSelectedNode;
};