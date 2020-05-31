#pragma once

#include "Element2D.h"
#include "Math3D.h"
#include "RenderContext2D.h"
#include "ClassMetaData.h"

class CAnimationSet;
class CAnimationController;
class CRenderObject2D : public CBaseObject
{
	friend class CScene2DManager;
	friend class CRenderContext2D;
public:
	CRenderObject2D();
	~CRenderObject2D();

	CRenderObject2D* GetParent() { return m_pTransformParent; }
	void SetTransformDirty();
	void ResetTransformDirty() { m_isTransformDirty = false; }
	void SetBoundDirty();
	const CRectangle& GetLocalBound() { return m_localBound; }
	void SetLocalBound( const CRectangle& bound ) { m_localBound = bound; SetBoundDirty(); }

	void AddChild( CRenderObject2D* pNode );
	void AddChildAfter( CRenderObject2D* pNode, CRenderObject2D* pAfter );
	void AddChildBefore( CRenderObject2D* pNode, CRenderObject2D* pBefore );

	void SortChildrenRenderOrder( function<bool( CRenderObject2D*, CRenderObject2D* )> Func );
private:
	void AddTransformChild( CRenderObject2D* pNode );
	void AddRenderChild( CRenderObject2D* pNode );
	void AddRenderChildAfter( CRenderObject2D* pNode, CRenderObject2D* pAfter );
	void AddRenderChildBefore( CRenderObject2D* pNode, CRenderObject2D* pBefore );
	void RemoveTransformChild( CRenderObject2D* pNode );
	void RemoveRenderChild( CRenderObject2D* pNode );
public:
	void SetRenderParent( CRenderObject2D* pNode );
	void SetRenderParentBefore( CRenderObject2D* pNode );
	void SetRenderParentAfter( CRenderObject2D* pNode );
	void RemoveAllChild();
	void RemoveThis();
	void MoveToTopmost( CRenderObject2D* pNode, bool bKeepZOrder = false );
	void MoveToTopmost( bool bKeepZOrder = false );
	int32 GetZOrder() { return m_nZOrder; }
	void SetZOrder( int32 nZOrder );

	static CRenderObject2D* FindCommonParent( CRenderObject2D* a, CRenderObject2D* b );

	CVector2 GetPosition() { return CVector2( x, y ); }
	void SetPosition( const CVector2& position ) { x = position.x; y = position.y; SetTransformDirty(); }
	float GetRotation() { return r; }
	void SetRotation( float r ) { this->r = r; SetTransformDirty(); }

	CAnimationController* GetAnimController();
	virtual const CMatrix2D& GetTransform( uint16 nIndex );
	void SetAnim( CAnimationSet* pAnimationSet, uint32 nPose );
	void UpdateAnim( float fTime );
	bool IsAutoUpdateAnim() { return m_bAutoUpdateAnim; }
	void SetAutoUpdateAnim( bool bAutoUpdateAnim );
	uint16 GetTransformIndex() { return m_nTransformIndex; }
	void SetTransformIndex( uint16 nIndex );

	virtual bool CalcAABB();
	void CalcGlobalTransform();
	void UpdateDirty();
	void Dispose();
	bool IsUpdated() { return m_bUpdated; }
	void SetUpdated( bool bUpdated ) { m_bUpdated = bUpdated; }

	bool ForceUpdateTransform();

	template <typename T>
	void SortChildren( T t )
	{
		if( !m_pChildren )
			return;
		vector<CRenderObject2D*> vecChildren;
		for( auto pChild = m_pChildren; pChild; pChild = pChild->NextChild() )
		{
			vecChildren.push_back( pChild );
		}

		for( int i = vecChildren.size() - 2; i >= 0; i-- )
		{
			auto pChild = vecChildren[i];
			auto pChild1 = pChild->NextChild();
			auto ppHead = pChild->__pPrevChild;
			while( pChild1 )
			{
				if( !t( pChild1, pChild ) )
					break;
				ppHead = &pChild1->NextChild();
				pChild1 = *ppHead;
			}
			if( pChild->NextChild() != pChild1 )
			{
				pChild->AddRef();
				pChild->RemoveFrom_Child();
				pChild->InsertTo_Child( *ppHead );
				pChild->Release();
			}
		}
	}

	virtual void UpdateRendered( double dTime ) {}
	virtual void FixedUpdateRendered() {}
	virtual void Render( CRenderContext2D& context ) {}

	float x, y;
	float r;
	float s;

	CMatrix2D globalTransform;

	CRectangle globalAABB;

	bool bVisible;

	struct PointerDepth
	{
		unsigned int operator () (CRenderObject2D* pNode)
		{
			return pNode->m_nDepth;
		}
	};
	struct PointerLessDepth
	{
		bool operator () (CRenderObject2D* pLeft, CRenderObject2D* pRight)
		{
			return pLeft->m_nDepth < pRight->m_nDepth;
		}
	};
	struct PointerGreaterDepth
	{
		bool operator () (CRenderObject2D* pLeft, CRenderObject2D* pRight)
		{
			return pLeft->m_nDepth > pRight->m_nDepth;
		}
	};

	struct PointerRenderDepth
	{
		unsigned int operator () ( CRenderObject2D* pNode )
		{
			return pNode->m_nRenderDepth;
		}
	};
protected:
	virtual void OnAddChild( CRenderObject2D* pChild ) {}
	virtual void OnRemoveChild( CRenderObject2D* pChild ) {}
	virtual void OnAdded(){}
	virtual void OnRemoved(){}
	virtual void OnTransformUpdated() {}

	virtual void OnDispose(){delete this;}

	CRectangle m_localBound;
private:
	void OnChildZOrderChanged( CRenderObject2D* pChild );
	void _setDepth(int depth);
	void _setRenderDepth( int depth );

	CRenderObject2D* m_pTransformParent;
	CRenderObject2D* m_pRenderParent;
	int32 m_nZOrder;
	int16 m_nDepth;
	int16 m_nRenderDepth;
	bool m_isTransformDirty;
	bool m_isAABBDirty;
	bool m_bUpdated;
	bool m_bAutoUpdateAnim;
	uint16 m_nTransformIndex;
	CAnimationController* m_pAnimController;
	LINK_LIST_REF( CRenderObject2D, TransformChild );
	LINK_LIST_REF_HEAD( m_pTransformChildren, CRenderObject2D, TransformChild );
	LINK_LIST_REF( CRenderObject2D, RenderChild );
	LINK_LIST_REF_HEAD( m_pRenderChildren, CRenderObject2D, RenderChild );
	LINK_LIST_REF( CRenderObject2D, UpdatedObject );
	LINK_LIST( CRenderObject2D, AutoUpdateAnimObject );
};
