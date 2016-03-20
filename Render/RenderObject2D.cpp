#include "stdafx.h"
#include "RenderObject2D.h"
#include "Scene2DManager.h"
#include "Animation.h"

CRenderObject2D::CRenderObject2D()
{
	m_pParent = NULL;
	m_pChildren = NULL;
	m_depth = -1;
	m_isTransformDirty = false;
	m_isAABBDirty = false;
	m_bUpdated = false;
	m_pAnimController = NULL;
	m_nTransformIndex = INVALID_16BITID;
	x = y = 0;
	r = 0;
	s = 1;
	m_nZOrder = 0;
	globalAABB = m_localBound = CRectangle( 0, 0, 0, 0 );
	bVisible = true;
}

CRenderObject2D::~CRenderObject2D()
{
	if( m_pAnimController )
		delete m_pAnimController;
	RemoveAllChild();
}


void CRenderObject2D::SetTransformDirty()
{
	if( m_depth >= 0 && !m_isTransformDirty )
	{
		CScene2DManager::GetGlobalInst()->AddDirtyRenderObject(this);
		m_isTransformDirty = true;
	}
}

void CRenderObject2D::SetBoundDirty()
{
	if( m_depth >= 0 && !m_isAABBDirty )
	{
		m_isAABBDirty = true;
		CScene2DManager::GetGlobalInst()->AddDirtyAABB(this);
	}
}

void CRenderObject2D::AddChild( CRenderObject2D* pNode )
{
	pNode->m_pParent = this;
	pNode->_setDepth(m_depth < 0? -1: m_depth + 1);
	OnAddChild(pNode);
	pNode->OnAdded();

	CRenderObject2D** pInsertTo = &m_pChildren;
	while( true )
	{
		CRenderObject2D* pNext = *pInsertTo;
		if( !pNext || pNext->m_nZOrder <= pNode->m_nZOrder )
			break;
		pInsertTo = &pNext->__pNextChild;
	}
	pNode->InsertTo_Child( *pInsertTo );

	pNode->SetTransformDirty();
}

void CRenderObject2D::AddChildAfter( CRenderObject2D* pNode, CRenderObject2D* pAfter )
{
	pNode->m_pParent = this;
	pNode->_setDepth(m_depth < 0? -1: m_depth + 1);
	pNode->m_nZOrder = pAfter->m_nZOrder;
	OnAddChild(pNode);
	pNode->OnAdded();
	pAfter->InsertAfter_Child( pNode );
	pNode->SetTransformDirty();
}

void CRenderObject2D::AddChildBefore( CRenderObject2D* pNode, CRenderObject2D* pBefore )
{
	pNode->m_pParent = this;
	pNode->_setDepth(m_depth < 0? -1: m_depth + 1);
	pNode->m_nZOrder = pBefore->m_nZOrder;
	OnAddChild(pNode);
	pNode->OnAdded();
	pBefore->InsertBefore_Child( pNode );
	pNode->SetTransformDirty();
}

void CRenderObject2D::RemoveChild( CRenderObject2D* pNode )
{
	pNode->OnRemoved();
	pNode->_setDepth(-1);
	pNode->m_pParent = NULL;
	OnRemoveChild(pNode);
	Remove_Child( pNode );
	SetTransformDirty();
}

void CRenderObject2D::RemoveAllChild()
{
	while( m_pChildren )
	{
		RemoveChild( m_pChildren );
	}
}

void CRenderObject2D::MoveToTopmost( CRenderObject2D* pNode, bool bKeepZOrder )
{
	CReference<CRenderObject2D> temp( pNode );
	if( bKeepZOrder )
	{
		while( pNode->__pPrevChild != &m_pChildren )
		{
			CRenderObject2D* pRenderObject = (CRenderObject2D*)( (uint8*)pNode->__pPrevChild - ( (uint8*)&__pNextChild - (uint8*)this ) );
			if( pRenderObject->m_nZOrder > pNode->m_nZOrder )
				break;
			pRenderObject->Shift_Child();
		}
	}
	else
	{
		Remove_Child( pNode );
		if( m_pChildren )
			pNode->m_nZOrder = Max( pNode->m_nZOrder, m_pChildren->m_nZOrder );
		Insert_Child( pNode );
	}
}

void CRenderObject2D::SetZOrder( int32 nZOrder )
{
	m_nZOrder = nZOrder;
	if( m_pParent )
		m_pParent->OnChildZOrderChanged( this );
}

CRenderObject2D* CRenderObject2D::FindCommonParent( CRenderObject2D* a, CRenderObject2D* b )
{
	if( a->m_depth < 0 || b->m_depth < 0 )
		return NULL;

	while( a->m_depth > b->m_depth )
		a = a->GetParent();
	while( b->m_depth > a->m_depth )
		b = b->GetParent();
	while( a && a != b )
	{
		a = a->GetParent();
		b = b->GetParent();
	}
	return a;
}

CAnimationController* CRenderObject2D::GetAnimController()
{
	if( !m_pAnimController )
	{
		class CEmptyAnimSet : public CAnimationSet
		{
		public:
			CEmptyAnimSet() { AddRef(); }
		};
		static CEmptyAnimSet g_emptyAnim;
		m_pAnimController = new CAnimationController( &g_emptyAnim, 0 );
	}
	return m_pAnimController;
}

void CRenderObject2D::SetAnim( CAnimationSet* pAnimationSet, uint32 nPose )
{
	if( m_pAnimController )
	{
		if( m_pAnimController->GetAnimSet() == pAnimationSet )
		{
			if( m_pAnimController->GetPose() == nPose )
				return;
			m_pAnimController->SetPose( nPose );
		}

		delete m_pAnimController;
		m_pAnimController = NULL;
	}
	else if( !pAnimationSet )
		return;

	if( pAnimationSet )
		m_pAnimController = new CAnimationController( pAnimationSet, nPose );
	SetTransformDirty();
}

void CRenderObject2D::UpdateAnim( float fTime )
{
	if( m_pAnimController )
	{
		m_pAnimController->UpdateTime( fTime );
		SetTransformDirty();
	}
}

void CRenderObject2D::SetTransformIndex( uint16 nIndex )
{
	m_nTransformIndex = nIndex;
	SetTransformDirty();
}

void CRenderObject2D::OnChildZOrderChanged( CRenderObject2D* pChild )
{
	while( pChild->__pPrevChild != &m_pChildren )
	{
		CRenderObject2D* pRenderObject = (CRenderObject2D*)( (uint8*)pChild->__pPrevChild - ( (uint8*)&__pNextChild - (uint8*)this ) );
		if( pRenderObject->m_nZOrder >= pChild->m_nZOrder )
			break;
		pRenderObject->Shift_Child();
	}
	while( pChild->__pNextChild )
	{
		CRenderObject2D* pRenderObject = pChild->__pNextChild;
		if( pRenderObject->m_nZOrder <= pChild->m_nZOrder )
			break;
		pChild->Shift_Child();
	}
}

bool CRenderObject2D::CalcAABB()
{
	CRectangle orig = globalAABB;
	globalAABB = m_localBound * globalTransform;

	for( CRenderObject2D* pChild = m_pChildren; pChild; pChild = pChild->NextChild() ) {
		globalAABB = globalAABB + pChild->globalAABB;
	}
	return !( orig == globalAABB );
}
void CRenderObject2D::CalcGlobalTransform()
{
	if( m_nTransformIndex != INVALID_16BITID && m_pParent && m_pParent->GetAnimController() )
		globalTransform = m_pParent->GetAnimController()->GetTransform( m_nTransformIndex );
	else
	{
		CMatrix2D mat;
		mat.Transform( x, y, r, s );
		if( m_pParent != NULL )
			globalTransform = m_pParent->globalTransform * mat;
		else
			globalTransform = mat;
	}

	if( m_pAnimController )
		m_pAnimController->Update( globalTransform );
}

void CRenderObject2D::UpdateDirty()
{
	CalcGlobalTransform();
	SetBoundDirty();
	m_isTransformDirty = false;
	OnTransformUpdated();
	for( CRenderObject2D* pChild = m_pChildren; pChild; pChild = pChild->NextChild() ) {
		pChild->UpdateDirty();
	}
}
void CRenderObject2D::Dispose()
{
	m_isTransformDirty = true;

	while( m_pChildren ) {
		RemoveChild( m_pChildren );
	}
	OnDispose();
}

void CRenderObject2D::_setDepth(int depth)
{
	m_depth = depth;
	for( CRenderObject2D* pChild = m_pChildren; pChild; pChild = pChild->NextChild() ) {
		pChild->_setDepth(m_depth < 0? -1: m_depth + 1);
	}
}
