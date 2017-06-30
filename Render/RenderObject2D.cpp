#include "stdafx.h"
#include "RenderObject2D.h"
#include "Scene2DManager.h"
#include "Animation.h"

CRenderObject2D::CRenderObject2D()
{
	SET_BASEOBJECT_ID( CRenderObject2D );
	m_pTransformParent = m_pRenderParent = NULL;
	m_pTransformChildren = m_pRenderChildren = NULL;
	m_depth = -1;
	m_isTransformDirty = false;
	m_isAABBDirty = false;
	m_bUpdated = false;
	m_bAutoUpdateAnim = false;
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
	pNode->m_pTransformParent = pNode->m_pRenderParent = this;
	pNode->_setDepth(m_depth < 0? -1: m_depth + 1);
	OnAddChild(pNode);
	pNode->OnAdded();

	CRenderObject2D** pInsertTo = &m_pRenderChildren;
	while( true )
	{
		CRenderObject2D* pNext = *pInsertTo;
		if( !pNext || pNext->m_nZOrder <= pNode->m_nZOrder )
			break;
		pInsertTo = &pNext->__pNextRenderChild;
	}
	pNode->InsertTo_RenderChild( *pInsertTo );
	Insert_TransformChild( pNode );

	pNode->SetTransformDirty();
}

void CRenderObject2D::AddChildAfter( CRenderObject2D* pNode, CRenderObject2D* pAfter )
{
	pNode->m_pTransformParent = pNode->m_pRenderParent = this;
	pNode->_setDepth(m_depth < 0? -1: m_depth + 1);
	pNode->m_nZOrder = pAfter->m_nZOrder;
	OnAddChild(pNode);
	pNode->OnAdded();
	pAfter->InsertAfter_RenderChild( pNode );
	Insert_TransformChild( pNode );
	pNode->SetTransformDirty();
}

void CRenderObject2D::AddChildBefore( CRenderObject2D* pNode, CRenderObject2D* pBefore )
{
	pNode->m_pTransformParent = pNode->m_pRenderParent = this;
	pNode->_setDepth(m_depth < 0? -1: m_depth + 1);
	pNode->m_nZOrder = pBefore->m_nZOrder;
	OnAddChild(pNode);
	pNode->OnAdded();
	pBefore->InsertBefore_RenderChild( pNode );
	Insert_TransformChild( pNode );
	pNode->SetTransformDirty();
}

void CRenderObject2D::AddTransformChild( CRenderObject2D * pNode )
{
	pNode->m_pTransformParent = this;
	pNode->_setDepth( m_depth < 0 ? -1 : m_depth + 1 );
	OnAddChild( pNode );
	pNode->OnAdded();
	Insert_TransformChild( pNode );
	pNode->SetTransformDirty();
}

void CRenderObject2D::AddRenderChild( CRenderObject2D * pNode )
{
	pNode->m_pRenderParent = this;

	CRenderObject2D** pInsertTo = &m_pRenderChildren;
	while( true )
	{
		CRenderObject2D* pNext = *pInsertTo;
		if( !pNext || pNext->m_nZOrder <= pNode->m_nZOrder )
			break;
		pInsertTo = &pNext->__pNextRenderChild;
	}
	pNode->InsertTo_RenderChild( *pInsertTo );
	pNode->SetBoundDirty();
}

void CRenderObject2D::AddRenderChildAfter( CRenderObject2D * pNode, CRenderObject2D * pAfter )
{
	pNode->m_pRenderParent = this;

	pNode->m_nZOrder = pAfter->m_nZOrder;
	pAfter->InsertAfter_RenderChild( pNode );
	pNode->SetBoundDirty();
}

void CRenderObject2D::AddRenderChildBefore( CRenderObject2D * pNode, CRenderObject2D * pBefore )
{
	pNode->m_pRenderParent = this;

	pNode->m_nZOrder = pBefore->m_nZOrder;
	pBefore->InsertBefore_RenderChild( pNode );
	pNode->SetBoundDirty();
}

void CRenderObject2D::RemoveTransformChild( CRenderObject2D* pNode )
{
	pNode->OnRemoved();
	pNode->_setDepth(-1);
	pNode->m_pTransformParent = NULL;
	OnRemoveChild(pNode);
	Remove_TransformChild( pNode );
}

void CRenderObject2D::RemoveRenderChild( CRenderObject2D * pNode )
{
	pNode->m_pRenderParent = NULL;
	Remove_RenderChild( pNode );
	SetBoundDirty();
}

void CRenderObject2D::SetRenderParent( CRenderObject2D * pNode )
{
	if( m_pRenderParent )
		m_pRenderParent->RemoveRenderChild( this );
	if( pNode )
		pNode->AddRenderChild( this );
}

void CRenderObject2D::SetRenderParentBefore( CRenderObject2D * pNode )
{
	if( m_pRenderParent )
		m_pRenderParent->RemoveRenderChild( this );
	if( pNode && pNode->m_pRenderParent )
		pNode->m_pRenderParent->AddRenderChildBefore( this, pNode );
}

void CRenderObject2D::SetRenderParentAfter( CRenderObject2D * pNode )
{
	if( m_pRenderParent )
		m_pRenderParent->RemoveRenderChild( this );
	if( pNode && pNode->m_pRenderParent )
		pNode->m_pRenderParent->AddRenderChildAfter( this, pNode );
}

void CRenderObject2D::RemoveAllChild()
{
	while( m_pTransformChildren )
		m_pTransformChildren->RemoveThis();
	while( m_pRenderChildren )
		RemoveRenderChild( m_pRenderChildren );
}

void CRenderObject2D::RemoveThis()
{
	if( m_pRenderParent )
		m_pRenderParent->RemoveRenderChild( this );
	if( m_pTransformParent )
		m_pTransformParent->RemoveTransformChild( this );
}

void CRenderObject2D::MoveToTopmost( CRenderObject2D* pNode, bool bKeepZOrder )
{
	CReference<CRenderObject2D> temp( pNode );
	if( bKeepZOrder )
	{
		while( pNode->__pPrevRenderChild != &m_pRenderChildren )
		{
			CRenderObject2D* pRenderObject = (CRenderObject2D*)( (uint8*)pNode->__pPrevRenderChild - ( (uint8*)&__pNextRenderChild - (uint8*)this ) );
			if( pRenderObject->m_nZOrder > pNode->m_nZOrder )
				break;
			pRenderObject->Shift_RenderChild();
		}
	}
	else
	{
		Remove_RenderChild( pNode );
		if( m_pRenderChildren )
			pNode->m_nZOrder = Max( pNode->m_nZOrder, m_pRenderChildren->m_nZOrder );
		Insert_RenderChild( pNode );
	}
}

void CRenderObject2D::MoveToTopmost( bool bKeepZOrder )
{
	if( m_pRenderParent )
		m_pRenderParent->MoveToTopmost( this, bKeepZOrder );
}

void CRenderObject2D::SetZOrder( int32 nZOrder )
{
	m_nZOrder = nZOrder;
	if( m_pRenderParent )
		m_pRenderParent->OnChildZOrderChanged( this );
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

const CMatrix2D& CRenderObject2D::GetTransform( uint16 nIndex )
{
	return GetAnimController()->GetTransform( m_nTransformIndex );
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

void CRenderObject2D::SetAutoUpdateAnim( bool bAutoUpdateAnim )
{
	if( m_bAutoUpdateAnim == bAutoUpdateAnim )
		return;
	m_bAutoUpdateAnim = bAutoUpdateAnim;
	if( m_depth >= 0 )
	{
		if( bAutoUpdateAnim )
			CScene2DManager::GetGlobalInst()->Insert_AutoUpdateAnimObject( this );
		else
			RemoveFrom_AutoUpdateAnimObject();
	}
}

void CRenderObject2D::SetTransformIndex( uint16 nIndex )
{
	m_nTransformIndex = nIndex;
	SetTransformDirty();
}

void CRenderObject2D::OnChildZOrderChanged( CRenderObject2D* pChild )
{
	while( pChild->__pPrevRenderChild != &m_pRenderChildren )
	{
		CRenderObject2D* pRenderObject = (CRenderObject2D*)( (uint8*)pChild->__pPrevRenderChild - ( (uint8*)&__pNextRenderChild - (uint8*)this ) );
		if( pRenderObject->m_nZOrder >= pChild->m_nZOrder )
			break;
		pRenderObject->Shift_RenderChild();
	}
	while( pChild->__pNextRenderChild )
	{
		CRenderObject2D* pRenderObject = pChild->__pNextRenderChild;
		if( pRenderObject->m_nZOrder <= pChild->m_nZOrder )
			break;
		pChild->Shift_RenderChild();
	}
}

bool CRenderObject2D::CalcAABB()
{
	CRectangle orig = globalAABB;
	globalAABB = m_localBound * globalTransform;

	for( CRenderObject2D* pChild = m_pRenderChildren; pChild; pChild = pChild->NextRenderChild() )
		globalAABB = globalAABB + pChild->globalAABB;
	return !( orig == globalAABB );
}

void CRenderObject2D::CalcGlobalTransform()
{
	if( m_nTransformIndex != INVALID_16BITID && m_pTransformParent )
		globalTransform = m_pTransformParent->GetTransform( m_nTransformIndex );
	else
	{
		CMatrix2D mat;
		mat.Transform( x, y, r, s );
		if( m_pTransformParent != NULL )
			globalTransform = m_pTransformParent->globalTransform * mat;
		else
			globalTransform = mat;
	}

	OnTransformUpdated();
	if( m_pAnimController )
		m_pAnimController->Update( globalTransform );
}

void CRenderObject2D::UpdateDirty()
{
	CalcGlobalTransform();
	SetBoundDirty();
	m_isTransformDirty = false;
	for( CRenderObject2D* pChild = m_pTransformChildren; pChild; pChild = pChild->NextTransformChild() ) {
		pChild->UpdateDirty();
	}
}
void CRenderObject2D::Dispose()
{
	if( m_pAnimController )
		delete m_pAnimController;
	m_isTransformDirty = true;
	RemoveAllChild();
	OnDispose();
}

bool CRenderObject2D::ForceUpdateTransform()
{
	bool bParentUpdated = false;
	if( m_depth > 0 )
		bParentUpdated = GetParent()->ForceUpdateTransform();
	if( bParentUpdated || m_isTransformDirty )
	{
		CalcGlobalTransform();
		return true;
	}
	return false;
}

void CRenderObject2D::_setDepth(int depth)
{
	if( m_depth == depth )
		return;
	m_depth = depth;
	if( m_bAutoUpdateAnim )
	{
		if( m_depth >= 0 )
			CScene2DManager::GetGlobalInst()->Insert_AutoUpdateAnimObject( this );
		else
			RemoveFrom_AutoUpdateAnimObject();
	}
	for( CRenderObject2D* pChild = m_pTransformChildren; pChild; pChild = pChild->NextTransformChild() ) {
		pChild->_setDepth(m_depth < 0? -1: m_depth + 1);
	}
}
