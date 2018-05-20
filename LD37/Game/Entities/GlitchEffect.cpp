#include "stdafx.h"
#include "GlitchEffect.h"
#include "Stage.h"
#include "Common/Rand.h"
#include "Common/ResourceManager.h"

CGlitchEffect::CGlitchEffect( const SClassCreateContext& context )
	: CEntity( context )
	, m_strMaterial0( context )
	, m_strMaterial1( context )
	, m_strMaterial2( context )
	, m_strMaterial3( context )
	, m_nNodes( 0 )
	, m_pNodes( NULL )
	, m_onTickBeforeHitTest( this, &CGlitchEffect::OnTickBeforeHitTest )
{
	SET_BASEOBJECT_ID( CGlitchEffect );
	m_nP = 0;
	for( int i = 0; i < 4; i++ )
		m_nP += m_p[i];
}

void CGlitchEffect::OnAddedToStage()
{
	m_pMaterials[0] = CResourceManager::Inst()->CreateResource<CDrawableGroup>( m_strMaterial0.c_str() );
	m_pMaterials[1] = CResourceManager::Inst()->CreateResource<CDrawableGroup>( m_strMaterial1.c_str() );
	m_pMaterials[2] = CResourceManager::Inst()->CreateResource<CDrawableGroup>( m_strMaterial2.c_str() );
	m_pMaterials[3] = CResourceManager::Inst()->CreateResource<CDrawableGroup>( m_strMaterial3.c_str() );

	OnTickBeforeHitTest();
}

void CGlitchEffect::OnRemovedFromStage()
{
	if( m_onTickBeforeHitTest.IsRegistered() )
		m_onTickBeforeHitTest.Unregister();
}

void CGlitchEffect::OnTickBeforeHitTest()
{
	static int i = 0;
	i++;
	float fPercent = m_nNodes * 1.0f / m_nMaxNodes;
	float fRemove = fPercent * fPercent * 0.5f;
	float r = SRand::Inst().Rand( 0.0f, 1.0f );
	if( r < 0.5f )
		ChangeNode();
	else if( r < 0.5f + fRemove )
		RemoveNode();
	else
		AddNode();

	GetStage()->RegisterBeforeHitTest( 1, &m_onTickBeforeHitTest );
}

void CGlitchEffect::SNode::UpdateImg( CGlitchEffect* pOwner )
{
	uint32 r = SRand::Inst().Rand( 0u, pOwner->m_nP );
	uint32 i;
	for( i = 0; i < pOwner->m_nPrefabCount; i++ )
	{
		if( r < pOwner->m_p[i] )
			break;
		r -= pOwner->m_p[i];
	}

	if( nType != i )
	{
		if( pImg )
			pImg->RemoveThis();
		nType = i;
		pImg = pOwner->m_pMaterials[nType] ? static_cast<CImage2D*>( pOwner->m_pMaterials[nType]->CreateInstance() ) : NULL;
		if( pImg )
			pOwner->AddChild( pImg );
	}
}

void CGlitchEffect::SNode::UpdateChildren( CGlitchEffect* pOwner )
{
	if( !pLeft )
	{
		UpdateImg( pOwner );

		if( pImg )
		{
			pImg->SetRect( rect );
			if( rect.width * rect.height == 0 )
			{
				pImg->bVisible = false;
				return;
			}
			pImg->bVisible = true;
			CRectangle texRect = rect;
			texRect.x -= pOwner->m_effectRect.x;
			texRect.y = pOwner->m_effectRect.y + pOwner->m_effectRect.height - rect.y - rect.height;
			float fTexSize = pOwner->m_texSize[nType];
			float fTexOfs = pOwner->m_fMaxTexOfs[nType];
			texRect = texRect * ( 1.0f / fTexSize );
			texRect = texRect.Offset( pOwner->m_baseTex[nType] + CVector2( SRand::Inst().Rand( fTexOfs * -0.5f, fTexOfs * 0.5f ), SRand::Inst().Rand( fTexOfs * -0.5f, fTexOfs * 0.5f ) ) );

			texRect.x -= floor( texRect.x );
			texRect.y -= floor( texRect.y );

			pImg->SetTexRect( texRect );
		}
		return;
	}

	CRectangle& rect1 = pLeft->rect;
	CRectangle& rect2 = pRight->rect;
	if( bVertical )
	{
		uint32 nHeight1 = rect.height * fSplitPercent;
		uint32 nHeight2 = rect.height - nHeight1;
		rect1 = CRectangle( rect.x, rect.y, rect.width, nHeight1 );
		rect2 = CRectangle( rect.x, rect.y + nHeight1, rect.width, nHeight2 );
	}
	else
	{
		uint32 nWidth1 = rect.width * fSplitPercent;
		uint32 nWidth2 = rect.width - nWidth1;
		rect1 = CRectangle( rect.x, rect.y, nWidth1, rect.height );
		rect2 = CRectangle( rect.x + nWidth1, rect.y, nWidth2, rect.height );
	}
	pLeft->UpdateChildren( pOwner );
	pRight->UpdateChildren( pOwner );
}

void CGlitchEffect::AddNode()
{
	uint32 s = 0;
	SNode* pParent = NULL;
	for( auto pNode = m_pNodes; pNode; pNode = pNode->NextNode() )
	{
		if( pNode->pLeft )
			continue;

		uint32 s1 = pNode->rect.width * pNode->rect.height;
		if( s1 > s )
		{
			s = s1;
			pParent = pNode;
		}
	}

	SNode* pNewNode = new SNode;
	pNewNode->pLeft = pNewNode->pRight = pNewNode->pParent = NULL;
	pNewNode->nType = -1;

	if( !pParent )
	{
		pNewNode->rect = m_effectRect;
		pNewNode->UpdateChildren( this );
	}
	else
	{
		SNode* pNode1 = new SNode( *pParent );
		pNode1->nType = -1;
		pNode1->rect = pParent->rect;
		pNode1->pParent = pParent->pParent;
		if( pParent->pParent )
		{
			if( pNode1->pParent->pLeft == pParent )
				pNode1->pParent->pLeft = pNode1;
			else
				pNode1->pParent->pRight = pNode1;
		}
		
		pNode1->bVertical = !!( SRand::Inst().Rand( 0, 2 ) );
		pNode1->fSplitPercent = SRand::Inst().Rand( 0.2f, 0.8f );
		if( !!( SRand::Inst().Rand( 0, 2 ) ) )
		{
			pNode1->pLeft = pParent;
			pNode1->pRight = pNewNode;
		}
		else
		{
			pNode1->pLeft = pNewNode;
			pNode1->pRight = pParent;
		}
		pParent->pParent = pNode1;
		pNewNode->pParent = pNode1;
		pNode1->UpdateChildren( this );
		Insert_Node( pNode1 );
		m_nNodes++;
	}
	Insert_Node( pNewNode );
	m_nNodes++;
}

void CGlitchEffect::RemoveNode()
{
	uint32 s = 0xffffffff;
	SNode* pRemoved = NULL;
	int i;
	for( auto pNode = m_pNodes; pNode; pNode = pNode->NextNode() )
	{
		if( pNode->pLeft )
			continue;

		uint32 s1 = pNode->rect.width * pNode->rect.height;
		if( s1 < s )
		{
			s = s1;
			pRemoved = pNode;
		}
	}

	if( !pRemoved )
		return;

	SNode* pParent = pRemoved->pParent;
	if( pParent )
	{
		SNode* pOther;
		if( pParent->pLeft == pRemoved )
			pOther = pParent->pRight;
		else
			pOther = pParent->pLeft;

		auto pParentParent = pParent->pParent;
		pOther->pParent = pParentParent;
		if( pParentParent )
		{
			if( pParentParent->pLeft == pParent )
				pParentParent->pLeft = pOther;
			else
				pParentParent->pRight = pOther;
			pParentParent->UpdateChildren( this );
		}
		else
		{
			pOther->rect = m_effectRect;
			pOther->UpdateChildren( this );
		}
		pParent->RemoveFrom_Node();
		delete pParent;
		m_nNodes--;
	}

	if( pRemoved->pImg )
		pRemoved->pImg->RemoveThis();
	pRemoved->RemoveFrom_Node();
	delete pRemoved;
	m_nNodes--;
}

void CGlitchEffect::ChangeNode()
{
	if( !m_nNodes )
		return;
	uint32 nNode = SRand::Inst().Rand( 0u, m_nNodes );
	SNode* pNode = m_pNodes;
	for( int i = 0; i < nNode; i++ )
		pNode = pNode->NextNode();

	if( pNode )
	{
		pNode->bVertical = !!( SRand::Inst().Rand( 0, 2 ) );
		pNode->fSplitPercent = SRand::Inst().Rand( 0.2f, 0.8f );
		pNode->UpdateChildren( this );
	}
}