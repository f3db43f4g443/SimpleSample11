#pragma once

#include "Element2D.h"
#include "Math3D.h"
#include "RenderContext2D.h"

class CAnimationSet;
class CAnimationController;
class CRenderObject2D : public CReferenceObject
{
	friend class CScene2DManager;
	friend class CRenderContext2D;
public:
	CRenderObject2D();
	~CRenderObject2D();

	CRenderObject2D* GetParent() { return m_pParent; }
	void SetTransformDirty();
	void SetBoundDirty();

	void AddChild( CRenderObject2D* pNode );
	void AddChildAfter( CRenderObject2D* pNode, CRenderObject2D* pAfter );
	void AddChildBefore( CRenderObject2D* pNode, CRenderObject2D* pBefore );
	void RemoveChild( CRenderObject2D* pNode );
	void RemoveAllChild();
	void RemoveThis() { if( m_pParent ) m_pParent->RemoveChild( this ); }
	void MoveToTopmost( CRenderObject2D* pNode );
	int32 GetZOrder() { return m_nZOrder; }
	void SetZOrder( int32 nZOrder );

	CAnimationController* GetAnimController();
	void SetAnim( CAnimationSet* pAnimationSet, uint32 nPose );
	void UpdateAnim( float fTime );
	uint16 GetTransformIndex() { return m_nTransformIndex; }
	void SetTransformIndex( uint16 nIndex );

	bool CalcAABB();
	void CalcGlobalTransform();
	void UpdateDirty();
	void Dispose();
	bool IsUpdated() { return m_bUpdated; }
	void SetUpdated( bool bUpdated ) { m_bUpdated = bUpdated; }

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
			return pNode->m_depth;
		}
	};
	struct PointerLessDepth
	{
		bool operator () (CRenderObject2D* pLeft, CRenderObject2D* pRight)
		{
			return pLeft->m_depth < pRight->m_depth;
		}
	};
	struct PointerGreaterDepth
	{
		bool operator () (CRenderObject2D* pLeft, CRenderObject2D* pRight)
		{
			return pLeft->m_depth > pRight->m_depth;
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

	CRenderObject2D* m_pParent;
	int32 m_nZOrder;
	int32 m_depth;
	bool m_isTransformDirty;
	bool m_isAABBDirty;
	bool m_bUpdated;
	uint16 m_nTransformIndex;
	CAnimationController* m_pAnimController;
	LINK_LIST_REF( CRenderObject2D, Child );
	LINK_LIST_REF_HEAD( m_pChildren, CRenderObject2D, Child );
	LINK_LIST_REF( CRenderObject2D, UpdatedObject );
};
