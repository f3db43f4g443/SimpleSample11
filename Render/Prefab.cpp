#include "stdafx.h"
#include "Prefab.h"
#include "DrawableGroup.h"
#include "Rope2D.h"
#include "ParticleSystem.h"
#include "FileUtil.h"
#include "ResourceManager.h"
#include "LightRendering.h"
#include "Image2D.h"
#include <sstream>
#include <set>

CRenderObject2D* CPrefabBaseNode::CreateRenderObjectByResource( CResource* pResource, CRenderObject2D* pNode )
{
	CRenderObject2D* pRenderObject = NULL;
	if( pResource )
	{
		switch( pResource->GetResourceType() )
		{
		case eEngineResType_DrawableGroup:
			{
				auto pRenderGroup = static_cast<CDrawableGroup*>( pResource );
				pRenderObject = pRenderGroup->CreateInstance();
				switch( pRenderGroup->GetType() )
				{
				case CDrawableGroup::eType_MultiFrame:
					pRenderObject->SetAutoUpdateAnim( true );
					break;
				default:
					break;
				}
			}
			break;
		case eEngineResType_ParticleSystem:
			pRenderObject = static_cast<CParticleFile*>( pResource )->CreateInstance( pNode ? pNode->GetAnimController() : NULL );
			break;
		case eEngineResType_Prefab:
			pRenderObject = static_cast<CPrefab*>( pResource )->GetRoot()->CreateInstance();
			break;
		}
	}
	return pRenderObject;
}

void CPrefabBaseNode::SetRenderObject( CRenderObject2D* pRenderObject )
{
	if( m_pRenderObject == pRenderObject )
		return;
	if( m_pRenderObject )
	{
		if( pRenderObject )
			AddChildBefore( pRenderObject, m_pRenderObject );
		m_pRenderObject->RemoveThis();
		m_pRenderObject = pRenderObject;
	}
	else
	{
		m_pRenderObject = pRenderObject;
		if( pRenderObject )
		{
			pRenderObject->SetZOrder( -1 );
			AddChild( pRenderObject );
			pRenderObject->SetZOrder( 0 );
		}
	}
}

void CPrefabBaseNode::UpdatePreview()
{
	OnUpdatePreview();
	for( auto pChild = Get_TransformChild(); pChild; pChild = pChild->NextTransformChild() )
	{
		auto p = SafeCast<CPrefabBaseNode>( pChild );
		if( p )
			p->UpdatePreview();
	}
}

void CPrefabBaseNode::DebugDraw( class CUIViewport* pViewport, IRenderSystem* pRenderSystem )
{
	OnDebugDraw( pViewport, pRenderSystem );
	for( auto pChild = Get_TransformChild(); pChild; pChild = pChild->NextTransformChild() )
	{
		auto p = SafeCast<CPrefabBaseNode>( pChild );
		if( p )
			p->DebugDraw( pViewport, pRenderSystem );
	}
}

void CPrefabNode::AddRef1()
{
	m_pPrefab->AddRef();
}

void CPrefabNode::Release1()
{
	m_pPrefab->Release();
}

bool CPrefabNode::SetResource( CResource* pResource )
{
	if( m_nType )
		return false;
	bool bIsRefresh = false;
	if( m_pPrefab )
	{
		bIsRefresh = pResource == m_pResource;
		if( pResource && !m_pPrefab->CanAddDependency( pResource ) )
			return false;
	}

	if( m_pPatchedNode )
	{
		for( CRenderObject2D* p = this; p; p = p->GetParent() )
		{
			auto pPrefabBaseNode = SafeCast<CPrefabBaseNode>( p );
			if( !pPrefabBaseNode )
				break;
			if( static_cast<CPrefabNode*>( pPrefabBaseNode )->GetNameSpace().pNameSpaceKey )
				static_cast<CPrefabNode*>( pPrefabBaseNode )->NameSpaceClearNode( m_pPatchedNode );
		}
		m_pPatchedNode = NULL;
		if( m_pPreviewNode )
			m_pPreviewNode = NULL;
		SetRenderObject( NULL );
	}
	CRenderObject2D* pRenderObject = NULL;
	if( pResource && pResource->GetResourceType() == eEngineResType_Prefab )
	{
		auto p = static_cast<CPrefab*>( pResource )->GetRoot();
		SPatchContext context( p );
		m_pPatchedNode = p->Clone( m_pPrefab, NULL, &context );
		//SetRenderObject( m_pPatchedNode->CreateInstance() );
		pRenderObject = m_pPatchedNode;
	}
	else
	{
		pRenderObject = CreateRenderObjectByResource( pResource, this );
		if( pRenderObject )
		{
			switch( pResource->GetResourceType() )
			{
			case eEngineResType_DrawableGroup:
			{
				auto pRenderGroup = static_cast<CDrawableGroup*>( pResource );
				switch( pRenderGroup->GetType() )
				{
				case CDrawableGroup::eType_MultiFrame:
					pRenderObject->SetAutoUpdateAnim( true );
					break;
				default:
					break;
				}
			}
			break;
			case eEngineResType_ParticleSystem:
				static_cast<CParticleSystemObject*>( pRenderObject )->GetInstanceData()->SetAutoRestart( true );
				break;
			}
		}
	}

	if( m_pPrefab )
	{
		if( !bIsRefresh )
		{
			if( m_onResourceRefreshBegin.IsRegistered() )
				m_onResourceRefreshBegin.Unregister();
			if( m_onResourceRefreshEnd.IsRegistered() )
				m_onResourceRefreshEnd.Unregister();
			if( m_pResource )
				m_pPrefab->RemoveDependency( m_pResource );
		}
	}
	m_pResource = pResource;
	if( m_pPrefab )
	{
		if( !bIsRefresh )
		{
			if( m_pResource )
			{
				m_pPrefab->AddDependency( m_pResource );
				m_pResource->RegisterRefreshBegin( &m_onResourceRefreshBegin );
				m_pResource->RegisterRefreshEnd( &m_onResourceRefreshEnd );
			}
		}
	}
	SetRenderObject( pRenderObject );
	return true;
}

SClassMetaData* CPrefabNode::GetFinalClassData()
{
	auto pClassData = m_obj.GetClassData();
	if( pClassData )
		return pClassData;

	if( m_pResource && m_pResource->GetResourceType() == eEngineResType_Prefab )
		return static_cast<CPrefab*>( m_pResource.GetPtr() )->GetRoot()->GetFinalClassData();

	return NULL;
}

CPrefabBaseNode* CPrefabNode::GetFinalObjData()
{
	auto pClassData = m_obj.GetClassData();
	if( pClassData )
		return m_obj.GetObject();

	if( m_pResource && m_pResource->GetResourceType() == eEngineResType_Prefab )
	{
		if( m_pPatchedNode )
			return m_pPatchedNode->GetFinalObjData();
		return static_cast<CPrefab*>( m_pResource.GetPtr() )->GetRoot()->GetFinalObjData();
	}

	return NULL;
}

void CPrefabNode::OnResourceRefreshBegin()
{
	if( m_pResource->GetResourceType() == eEngineResType_Prefab )
		SetRenderObject( NULL );
}

void CPrefabNode::OnResourceRefreshEnd()
{
	CReference<CResource> pResource = m_pResource;
	if( pResource->GetResourceType() == eEngineResType_Prefab )
	{
		SetResource( pResource );
	}
	else if( pResource->GetResourceType() == eEngineResType_DrawableGroup )
	{
		auto nType = static_cast<CDrawableGroup*>( pResource.GetPtr() )->GetType();
		CReference<CImage2D> pPreImage = nType == CDrawableGroup::eType_Default ? static_cast<CImage2D*>( m_pRenderObject.GetPtr() ) : NULL;
		CReference<CRopeObject2D> pPreRope = nType == CDrawableGroup::eType_Rope ? static_cast<CRopeObject2D*>( m_pRenderObject.GetPtr() ) : NULL;
		CReference<CMultiFrameImage2D> pPreMultiImage = nType == CDrawableGroup::eType_MultiFrame ? static_cast<CMultiFrameImage2D*>( m_pRenderObject.GetPtr() ) : NULL;
		CReference<CTileMap2D> pPreTileMap = nType == CDrawableGroup::eType_TileMap ? static_cast<CTileMap2D*>( m_pRenderObject.GetPtr() ) : NULL;
		SetResource( pResource );
		CImage2D* pCurImage = nType == CDrawableGroup::eType_Default ? static_cast<CImage2D*>( m_pRenderObject.GetPtr() ) : NULL;
		CRopeObject2D* pCurRope = nType == CDrawableGroup::eType_Rope ? static_cast<CRopeObject2D*>( m_pRenderObject.GetPtr() ) : NULL;
		CMultiFrameImage2D* pCurMultiImage = nType == CDrawableGroup::eType_MultiFrame ? static_cast<CMultiFrameImage2D*>( m_pRenderObject.GetPtr() ) : NULL;
		CTileMap2D* pCurTileMap = nType == CDrawableGroup::eType_TileMap ? static_cast<CTileMap2D*>( m_pRenderObject.GetPtr() ) : NULL;
		if( pPreImage && pCurImage )
		{
			pCurImage->SetRect( pPreImage->GetElem().rect );
			pCurImage->SetTexRect( pPreImage->GetElem().texRect );
			uint16 nPreParamCount;
			CVector4* pPreParam = pPreImage->GetParam( nPreParamCount );
			uint16 nCurParamCount;
			CVector4* pCurParam = pCurImage->GetParam( nCurParamCount );
			memcpy( pCurParam, pPreParam, Min( nPreParamCount, nCurParamCount ) );
		}
		else if( pPreRope && pCurRope )
		{
			uint32 nData = pPreRope->GetData().data.size();
			pCurRope->SetTransformDirty();
			pCurRope->SetDataCount( nData );
			pCurRope->SetSegmentsPerData( pPreRope->GetData().nSegmentsPerData );
			for( int i = 0; i < nData; i++ )
			{
				auto& data = pPreRope->GetData().data[i];
				pCurRope->SetData( i, data.center, data.fWidth, data.tex0, data.tex1 );
			}
		}
		else if( pPreMultiImage && pCurMultiImage )
		{
			pCurMultiImage->SetFrames( pPreMultiImage->GetFrameBegin(), pPreMultiImage->GetFrameEnd(), pPreMultiImage->GetFramesPerSec() );
			pCurMultiImage->SetPlaySpeed( pPreMultiImage->GetPlaySpeed(), pPreMultiImage->IsLoop() );
		}
		else if( pPreTileMap && pCurTileMap )
		{
			pCurTileMap->CopyData( pPreTileMap );
		}
	}
	else if( pResource->GetResourceType() == eEngineResType_ParticleSystem )
	{
		CReference<CParticleSystemObject> pPreParticleSystem = static_cast<CParticleSystemObject*>( m_pRenderObject.GetPtr() );
		SetResource( pResource );
		CReference<CParticleSystemObject> pCurParticleSystem = static_cast<CParticleSystemObject*>( m_pRenderObject.GetPtr() );
		if( pPreParticleSystem && pCurParticleSystem )
			pPreParticleSystem->CopyData( pCurParticleSystem );
	}
}

void CPrefabNode::LoadResourceExtraData( CResource* pResource, IBufReader& extraData, CPrefabNodeNameSpace* pNameSpace )
{
	if( pResource->GetResourceType() == eEngineResType_Prefab )
	{
		auto pPrefab = static_cast<CPrefab*>( pResource );
		auto p = pPrefab->GetRoot();
		SPatchContext context( p );
		while( 1 )
		{
			string str;
			extraData.Read( str );
			if( !str.length() )
				break;
			auto& buf = context.mapPatches[str];
			buf.Clear();
			extraData.Read( buf );
		}

		if( m_pPrefab->CanAddDependency( pResource ) )
		{
			m_pResource = pResource;
			m_pPatchedNode = p->Clone( m_pPrefab, pNameSpace, &context );
			//SetRenderObject( m_pPatchedNode->CreateInstance() );
			SetRenderObject( m_pPatchedNode );
			m_pPrefab->AddDependency( pResource );
		}
	}
	else
	{
		SetResource( pResource );
		if( pResource->GetResourceType() == eEngineResType_DrawableGroup )
		{
			auto nType = static_cast<CDrawableGroup*>( pResource )->GetType();
			if( nType == CDrawableGroup::eType_Default )
			{
				CImage2D* pRenderObject = static_cast<CImage2D*>( m_pRenderObject.GetPtr() );
				CRectangle rect, texRect;
				extraData.Read( rect );
				extraData.Read( texRect );
				pRenderObject->SetRect( rect );
				pRenderObject->SetTexRect( texRect );
				uint16 nParamCount;
				CVector4* pParams = pRenderObject->GetParam( nParamCount );
				nParamCount = Min( nParamCount, extraData.Read<uint16>() );
				extraData.Read( pParams, nParamCount * sizeof( CVector4 ) );
			}
			else if( nType == CDrawableGroup::eType_Rope )
			{
				CRopeObject2D* pRope = static_cast<CRopeObject2D*>( m_pRenderObject.GetPtr() );
				uint32 nCount = extraData.Read<uint32>();
				pRope->SetTransformDirty();
				pRope->SetDataCount( nCount );
				pRope->SetSegmentsPerData( extraData.Read<uint32>() );
				for( int i = 0; i < nCount; i++ )
				{
					CVector2 center = extraData.Read<CVector2>();
					float fWidth = extraData.Read<float>();
					CVector2 tex0 = extraData.Read<CVector2>();
					CVector2 tex1 = extraData.Read<CVector2>();
					pRope->SetData( i, center, fWidth, tex0, tex1 );
				}
			}
			else if( nType == CDrawableGroup::eType_MultiFrame )
			{
				CMultiFrameImage2D* pMultiImage = static_cast<CMultiFrameImage2D*>( m_pRenderObject.GetPtr() );
				uint16 nBegin = extraData.Read<uint16>();
				uint16 nEnd = extraData.Read<uint16>();
				float fFrame = extraData.Read<float>();
				pMultiImage->SetFrames( nBegin, nEnd, fFrame );
				float fPlaySpeed = 1.0f;
				uint8 bLoop = 1;
				extraData.Read( fPlaySpeed );
				extraData.Read( bLoop );
				pMultiImage->SetPlaySpeed( fPlaySpeed, bLoop );
			}
			else if( nType == CDrawableGroup::eType_TileMap )
			{
				CTileMap2D* pTileMap = static_cast<CTileMap2D*>( m_pRenderObject.GetPtr() );
				pTileMap->LoadData( extraData );
			}
		}
		else if( pResource->GetResourceType() == eEngineResType_ParticleSystem )
		{
			CParticleSystemObject* pParticle = static_cast<CParticleSystemObject*>( m_pRenderObject.GetPtr() );
			pParticle->LoadExtraData( extraData );
		}
	}
}

void CPrefabNode::LoadOtherExtraData( IBufReader & extraData )
{
	if( m_nType == 1 )
	{
		CDirectionalLightObject* pLightObject = new CDirectionalLightObject;
		extraData.Read( pLightObject->Dir );
		extraData.Read( pLightObject->fShadowScale );
		extraData.Read( pLightObject->fMaxShadowDist );
		extraData.Read( pLightObject->baseColor );
		SetRenderObject( pLightObject );
	}
	else if( m_nType == 2 )
	{
		CPointLightObject* pLightObject = new CPointLightObject;
		extraData.Read( pLightObject->AttenuationIntensity );
		extraData.Read( pLightObject->fShadowScale );
		extraData.Read( pLightObject->fMaxRange );
		extraData.Read( pLightObject->fLightHeight );
		extraData.Read( pLightObject->baseColor );
		SetRenderObject( pLightObject );
	}
}

void CPrefabNode::SaveResourceExtraData( CResource* pResource, CBufFile& extraData, CPrefabNodeNameSpace* pNameSpace )
{
	if( pResource->GetResourceType() == eEngineResType_Prefab )
	{
		if( m_pPatchedNode )
		{
			auto pPrefab = static_cast<CPrefab*>( pResource );
			auto p = pPrefab->GetRoot();
			SPatchContext context( p );
			p->Diff( m_pPatchedNode, context, pNameSpace );
			if( context.mapPatches.size() )
			{
				for( auto& item : context.mapPatches )
				{
					extraData.Write( item.first );
					extraData.Write( item.second );
				}
				extraData.Write( string() );
			}
		}
	}
	else if( pResource->GetResourceType() == eEngineResType_DrawableGroup )
	{
		auto nType = static_cast<CDrawableGroup*>( pResource )->GetType();
		if( nType == CDrawableGroup::eType_Default )
		{
			CImage2D* pRenderObject = static_cast<CImage2D*>( m_pRenderObject.GetPtr() );
			extraData.Write( pRenderObject->GetElem().rect );
			extraData.Write( pRenderObject->GetElem().texRect );
			uint16 nParamCount;
			CVector4* pParams = pRenderObject->GetParam( nParamCount );
			extraData.Write( nParamCount );
			extraData.Write( pParams, nParamCount * sizeof( CVector4 ) );
		}
		else if( nType == CDrawableGroup::eType_Rope )
		{
			CRopeObject2D* pRope = static_cast<CRopeObject2D*>( m_pRenderObject.GetPtr() );
			auto& data = pRope->GetData();
			uint32 nCount = data.data.size();
			extraData.Write( nCount );
			extraData.Write( data.nSegmentsPerData );
			for( int i = 0; i < nCount; i++ )
			{
				auto& item = data.data[i];
				extraData.Write( item.center );
				extraData.Write( item.fWidth );
				extraData.Write( item.tex0 );
				extraData.Write( item.tex1 );
			}
		}
		else if( nType == CDrawableGroup::eType_MultiFrame )
		{
			CMultiFrameImage2D* pMultiImage = static_cast<CMultiFrameImage2D*>( m_pRenderObject.GetPtr() );
			extraData.Write<uint16>( pMultiImage->GetFrameBegin() );
			extraData.Write<uint16>( pMultiImage->GetFrameEnd() );
			extraData.Write<float>( pMultiImage->GetFramesPerSec() );
			extraData.Write<float>( pMultiImage->GetPlaySpeed() );
			extraData.Write<uint8>( pMultiImage->IsLoop() );
		}
		else if( nType == CDrawableGroup::eType_TileMap )
		{
			CTileMap2D* pTileMap = static_cast<CTileMap2D*>( m_pRenderObject.GetPtr() );
			pTileMap->SaveData( extraData );
		}
	}
	else if( pResource->GetResourceType() == eEngineResType_ParticleSystem )
	{
		CParticleSystemObject* pParticle = static_cast<CParticleSystemObject*>( m_pRenderObject.GetPtr() );
		pParticle->SaveExtraData( extraData );
	}
}

void CPrefabNode::SaveOtherExtraData( CBufFile & extraData )
{
	if( m_nType == 1 )
	{
		CDirectionalLightObject* pLightObject = static_cast<CDirectionalLightObject*>( m_pRenderObject.GetPtr() );
		extraData.Write( pLightObject->Dir );
		extraData.Write( pLightObject->fShadowScale );
		extraData.Write( pLightObject->fMaxShadowDist );
		extraData.Write( pLightObject->baseColor );
	}
	else if( m_nType == 2 )
	{
		CPointLightObject* pLightObject = static_cast<CPointLightObject*>( m_pRenderObject.GetPtr() );
		extraData.Write( pLightObject->AttenuationIntensity );
		extraData.Write( pLightObject->fShadowScale );
		extraData.Write( pLightObject->fMaxRange );
		extraData.Write( pLightObject->fLightHeight );
		extraData.Write( pLightObject->baseColor );
	}
}

void CPrefabNode::Diff( CPrefabNode* pNode, SPatchContext& context, CPrefabNodeNameSpace* pNameSpace )
{
	bool bPatch = false;
	CBufFile buf;
	CBufFile tmpBuf;
	if( m_obj.DiffData( pNode->m_obj, tmpBuf ) )
		bPatch = true;
	else
		tmpBuf.Clear();
	buf.Write( tmpBuf );

	int8 nPatchFlag = 0;
	int32 nPatchFlagOfs = buf.GetBufLen();
	buf.Write( nPatchFlag );
	if( pNode->x != x )
	{
		buf.Write( pNode->x );
		nPatchFlag |= 1;
	}
	if( pNode->y != y )
	{
		buf.Write( pNode->y );
		nPatchFlag |= 2;
	}
	if( pNode->r != r )
	{
		buf.Write( pNode->r );
		nPatchFlag |= 4;
	}
	if( pNode->s != s )
	{
		buf.Write( pNode->s );
		nPatchFlag |= 8;
	}

	CBufFile tmpBuf0;
	tmpBuf.Clear();
	if( GetResource() && GetResource() == pNode->GetResource() && GetResource()->GetResourceType() == eEngineResType_Prefab && pNode->m_pPatchedNode )
	{
		CPrefabNode* p = m_pPatchedNode;
		if( !p )
			p = static_cast<CPrefab*>( GetResource() )->GetRoot();
		SPatchContext context1( p );
		p->Diff( pNode->m_pPatchedNode, context1, pNameSpace );
		if( context1.mapPatches.size() )
		{
			for( auto& item : context1.mapPatches )
			{
				tmpBuf.Write( item.first );
				tmpBuf.Write( item.second );
			}
			tmpBuf.Write( string() );
			nPatchFlag |= 32;
			buf.Write( tmpBuf );
		}
	}
	else
	{
		for( int i = ( pNode->GetResource() && pNode->GetResource()->GetResourceType() == eEngineResType_Prefab ? 1 : 0 ); i < 2; i++ )
		{
			auto& b = i == 0 ? tmpBuf0 : tmpBuf;
			auto p = i == 0 ? this : pNode;
			b.Write( p->m_nType );
			if( p->m_nType == 0 )
			{
				auto pRes = p->GetResource();
				string s = pRes ? pRes->GetName() : "";
				b.Write( s );
				if( pRes )
					p->SaveResourceExtraData( pRes, b, pNameSpace );
			}
			else
				p->SaveOtherExtraData( b );
		}
		if( tmpBuf0 != tmpBuf )
		{
			nPatchFlag |= 16;
			buf.Write( tmpBuf );
		}
	}
	int32 nNamespaceID = pNameSpace->FindIDByNode( pNode );
	if( nNamespaceID )
	{
		nPatchFlag |= 64;
		buf.Write( nNamespaceID );
	}

	*( nPatchFlagOfs + (int8*)buf.GetBuffer() ) = nPatchFlag;
	bPatch = bPatch || nPatchFlag;

	vector<pair<CPrefabNode*, CPrefabNode*> > vecChildren;
	vector<pair<int32, CPrefabNode*> > vecExtraNodes;
	auto pChild = Get_RenderChild();
	auto pChild1 = pNode->Get_RenderChild();
	for( ; pChild; pChild = pChild->NextRenderChild() )
	{
		if( pChild == m_pRenderObject )
			continue;
		for( ;; pChild1 = pChild1->NextRenderChild() )
		{
			if( pChild1 == pNode->m_pRenderObject )
				continue;
			CPrefabNode* pNode1 = static_cast<CPrefabNode*>( pChild1 );
			if( !pNode1->m_bIsInstance )
			{
				vecExtraNodes.push_back( pair<int32, CPrefabNode*>( vecChildren.size(), pNode1 ) );
				continue;
			}
			break;
		}
		vecChildren.push_back( pair<CPrefabNode*, CPrefabNode*>( static_cast<CPrefabNode*>( pChild ), static_cast<CPrefabNode*>( pChild1 ) ) );
		pChild1 = pChild1->NextRenderChild();
	}
	for( ; pChild1; pChild1 = pChild1->NextRenderChild() )
	{
		if( pChild1 == pNode->m_pRenderObject )
			continue;
		vecExtraNodes.push_back( pair<int32, CPrefabNode*>( vecChildren.size(), static_cast<CPrefabNode*>( pChild1 ) ) );
	}
	bPatch = bPatch || vecExtraNodes.size();
	buf.Write( vecExtraNodes.size() );
	for( int i = vecExtraNodes.size() - 1; i >= 0; i-- )
	{
		vecExtraNodes[i].first = vecChildren.size() - vecExtraNodes[i].first;
		buf.Write( vecExtraNodes[i].first );
		vecExtraNodes[i].second->Save( buf, pNameSpace );
	}

	if( bPatch )
	{
		string str;
		GetPathString( context.pRoot, str );
		context.mapPatches[str].Write( buf.GetBuffer(), buf.GetBufLen() );
	}

	for( int i = vecChildren.size() - 1; i >= 0; i-- )
		vecChildren[i].first->Diff( vecChildren[i].second, context, pNameSpace );
}

void CPrefabNode::UpdateNameSpace( SNameSpaceUpdateContext* pContext )
{
	m_bNamespaceDirty = false;
	SNameSpaceUpdateContext curContext;
	if( m_nameSpace.pNameSpaceKey )
	{
		m_nameSpace.ClearInfo();
		curContext.pNameSpace = &m_nameSpace;
		curContext.pNext = pContext;
		pContext = &curContext;
	}
	m_vecNameSpaceInfo.clear();
	m_vecObjPtrTemp.clear();

	auto pClassData = GetClassData();
	if( pClassData )
	{
		function<void( SClassMetaData::SMemberData* pData, uint32 nOfs )> func = [this] ( SClassMetaData::SMemberData* pData, uint32 nOfs )
		{
			int8 nType;
			if( !pData )
				nType = 2;
			else if( pData->nType == SClassMetaData::SMemberData::eTypeObjRef )
				nType = 0;
			else
				nType = 1;
			if( nType == 2 && m_vecObjPtrTemp.back().nType == 1 )
				m_vecObjPtrTemp.pop_back();
			else
				m_vecObjPtrTemp.push_back( SResPtrInfo( pData, nOfs, nType ) );
		};
		pClassData->FindAllObjRef( func );
		auto pData = m_obj.GetObjData();
		CreateObjPtrInfo( pContext, pData, 0 );
	}

	for( auto p = pContext; p; p = p->pNext )
	{
		auto pNameSpaceInfo = p->pNameSpace->AddNamespaceInfo( this );
		if( pNameSpaceInfo )
			m_vecNameSpaceInfo.push_back( pNameSpaceInfo );
	}
	for( auto& info : m_vecObjPtrInfo )
	{
		info.pNameSpace->AddObjPtrInfo( &info );
	}

	if( m_pPatchedNode )
		m_pPatchedNode->UpdateNameSpace( pContext );
	for( CRenderObject2D* pChild = Get_RenderChild(); pChild; pChild = pChild->NextRenderChild() )
	{
		if( pChild == m_pRenderObject )
			continue;
		CPrefabNode* pNode = static_cast<CPrefabNode*>( pChild );
		if( pNode )
			pNode->UpdateNameSpace( pContext );
	}

	if( m_nameSpace.pNameSpaceKey )
		m_nameSpace.CheckInfos();
}

int32 CPrefabNode::CreateObjPtrInfo( SNameSpaceUpdateContext* pContext, uint8* pObjData, int32 n )
{
	for( ; n < m_vecObjPtrTemp.size(); n++ )
	{
		auto& info = m_vecObjPtrTemp[n];
		if( info.nType == 0 )
		{
			if( !!( info.pMemberData->nFlag & 2 ) )
			{
				auto nArraySize = ( ( TArray<TObjRef<CRenderObject2D> >* )( pObjData + info.nOfs ) )->Size();
				if( nArraySize )
				{
					auto p = *(uint8**)( pObjData + info.nOfs + TArray<TObjRef<CRenderObject2D> >::GetPtrOfs() );
					for( int i = 0; i < nArraySize; i++, p += info.pMemberData->GetDataSize() )
						AddObjPtrInfo( pContext, p, info.pMemberData );
				}
			}
			else
				AddObjPtrInfo( pContext, pObjData + info.nOfs, info.pMemberData );
		}
		else if( info.nType == 1 )
		{
			auto nArraySize = ( ( TArray<TObjRef<CRenderObject2D> >* )( pObjData + info.nOfs ) )->Size();
			if( nArraySize )
			{
				auto p = *(uint8**)( pObjData + info.nOfs + TArray<TObjRef<CRenderObject2D> >::GetPtrOfs() );
				int32 n1;
				for( int i = 0; i < nArraySize; i++, p += info.pMemberData->GetDataSize() )
					n1 = CreateObjPtrInfo( pContext, p, n + 1 );
				n = n1;
			}
			else
			{
				n++;
				for( int32 k = 1; n < m_vecObjPtrTemp.size(); n++ )
				{
					if( m_vecObjPtrTemp[n].nType == 1 )
						k++;
					else if( m_vecObjPtrTemp[n].nType == 2 )
					{
						k--;
						if( !k )
							break;
					}
				}
			}
		}
		else
			return n;
	}
	return m_vecObjPtrTemp.size();
}

void CPrefabNode::AddObjPtrInfo( SNameSpaceUpdateContext* pContext, uint8* p, SClassMetaData::SMemberData* pMemberData )
{
	auto& objptr = *( TObjRef<CRenderObject2D>* )( p );
	void* key = objptr._pt;
	CPrefabNodeNameSpace* pNameSpace = NULL;
	for( auto pCtx = pContext; pCtx; pCtx = pCtx->pNext )
	{
		if( pCtx->pNameSpace->pNameSpaceKey == key )
		{
			pNameSpace = pCtx->pNameSpace;
			break;
		}
	}
	if( pNameSpace )
	{
		m_vecObjPtrInfo.push_back( CPrefabNodeNameSpace::SObjPtrInfo( pNameSpace, pMemberData, objptr._n ) );
		objptr._user = m_vecObjPtrInfo.size();
	}
	else
		objptr._user = 0;
}

int32 CPrefabNode::CopyObjPtrInfo( uint8* pDest, uint8* pObjData, int32 n )
{
	for( ; n < m_vecObjPtrTemp.size(); n++ )
	{
		auto& info = m_vecObjPtrTemp[n];
		if( info.nType == 0 )
		{
			if( !!( info.pMemberData->nFlag & 2 ) )
			{
				auto nArraySize = ( ( TArray<TObjRef<CRenderObject2D> >* )( pObjData + info.nOfs ) )->Size();
				if( nArraySize )
				{
					auto p1 = *(uint8**)( pDest + info.nOfs + TArray<TObjRef<CRenderObject2D> >::GetPtrOfs() );
					auto p = *(uint8**)( pObjData + info.nOfs + TArray<TObjRef<CRenderObject2D> >::GetPtrOfs() );
					for( int i = 0; i < nArraySize; i++, p1 += info.pMemberData->GetDataSize(), p += info.pMemberData->GetDataSize() )
						CopyObjPtrInfo1( p1, p, info.pMemberData );
				}
			}
			else
				CopyObjPtrInfo1( pDest + info.nOfs, pObjData + info.nOfs, info.pMemberData );
		}
		else if( info.nType == 1 )
		{
			auto nArraySize = ( ( TArray<TObjRef<CRenderObject2D> >* )( pObjData + info.nOfs ) )->Size();
			if( nArraySize )
			{
				auto p1 = *(uint8**)( pDest + info.nOfs + TArray<TObjRef<CRenderObject2D> >::GetPtrOfs() );
				auto p = *(uint8**)( pObjData + info.nOfs + TArray<TObjRef<CRenderObject2D> >::GetPtrOfs() );
				int32 n1;
				for( int i = 0; i < nArraySize; i++, p1 += info.pMemberData->GetDataSize(), p += info.pMemberData->GetDataSize() )
					n1 = CopyObjPtrInfo( p1, p, n + 1 );
				n = n1;
			}
			else
			{
				n++;
				for( int32 k = 1; n < m_vecObjPtrTemp.size(); n++ )
				{
					if( m_vecObjPtrTemp[n].nType == 1 )
						k++;
					else if( m_vecObjPtrTemp[n].nType == 2 )
					{
						k--;
						if( !k )
							break;
					}
				}
			}
		}
		else
			return n;
	}
	return m_vecObjPtrTemp.size();
}

void CPrefabNode::CopyObjPtrInfo1( uint8* pDest, uint8* pObjData, SClassMetaData::SMemberData* pMemberData )
{
	auto& objptr = *( TObjRef<CRenderObject2D>* )( pObjData );
	auto& dstptr = *( TObjRef<CRenderObject2D>* )( pDest );
	if( objptr._user )
	{
		auto& info = m_vecObjPtrInfo[objptr._user - 1];
		info.pCreatedNodeData = pDest + dstptr.GetPtrOfs();
	}
}

void CPrefabNode::FixObjRef( map<int32, int32>& mapID, void* pNameSpaceKey, int8 nPass )
{
	auto pClassData = GetClassData();
	if( pClassData )
	{
		vector<SResPtrInfo> vecTmp;
		function<void( SClassMetaData::SMemberData* pData, uint32 nOfs )> func = [this, &vecTmp] ( SClassMetaData::SMemberData* pData, uint32 nOfs )
		{
			int8 nType;
			if( !pData )
				nType = 2;
			else if( pData->nType == SClassMetaData::SMemberData::eTypeObjRef )
				nType = 0;
			else
				nType = 1;
			if( nType == 2 && vecTmp.back().nType == 1 )
				vecTmp.pop_back();
			else
				vecTmp.push_back( SResPtrInfo( pData, nOfs, nType ) );
		};
		pClassData->FindAllObjRef( func );
		auto pData = m_obj.GetObjData();
		FixObjRefNode( mapID, pData, 0, vecTmp, pNameSpaceKey, nPass );
	}
	if( m_pPatchedNode )
		m_pPatchedNode->FixObjRef( mapID, pNameSpaceKey, nPass );
	for( CRenderObject2D* pChild = Get_RenderChild(); pChild; pChild = pChild->NextRenderChild() )
	{
		if( pChild == m_pRenderObject )
			continue;
		CPrefabNode* pNode = static_cast<CPrefabNode*>( pChild );
		if( pNode )
			pNode->FixObjRef( mapID, pNameSpaceKey, nPass );
	}
}

int32 CPrefabNode::FixObjRefNode( map<int32, int32>& mapID, uint8* pObjData, int32 n, vector<SResPtrInfo>& vec, void* pNameSpaceKey, int8 nPass )
{
	for( ; n < vec.size(); n++ )
	{
		auto& info = vec[n];
		if( info.nType == 0 )
		{
			if( !!( info.pMemberData->nFlag & 2 ) )
			{
				auto nArraySize = ( ( TArray<TObjRef<CRenderObject2D> >* )( pObjData + info.nOfs ) )->Size();
				if( nArraySize )
				{
					auto p = *(uint8**)( pObjData + info.nOfs + TArray<TObjRef<CRenderObject2D> >::GetPtrOfs() );
					for( int i = 0; i < nArraySize; i++, p += info.pMemberData->GetDataSize() )
						FixObjRefNode1( mapID, p, info.pMemberData, pNameSpaceKey, nPass );
				}
			}
			else
				FixObjRefNode1( mapID, pObjData + info.nOfs, info.pMemberData, pNameSpaceKey, nPass );
		}
		else if( info.nType == 1 )
		{
			auto nArraySize = ( ( TArray<TObjRef<CRenderObject2D> >* )( pObjData + info.nOfs ) )->Size();
			if( nArraySize )
			{
				auto p = *(uint8**)( pObjData + info.nOfs + TArray<TObjRef<CRenderObject2D> >::GetPtrOfs() );
				int32 n1;
				for( int i = 0; i < nArraySize; i++, p += info.pMemberData->GetDataSize() )
					n1 = FixObjRefNode( mapID, p, n + 1, vec, pNameSpaceKey, nPass );
				n = n1;
			}
			else
			{
				n++;
				for( int32 k = 1; n < vec.size(); n++ )
				{
					if( vec[n].nType == 1 )
						k++;
					else if( vec[n].nType == 2 )
					{
						k--;
						if( !k )
							break;
					}
				}
			}
		}
		else
			return n;
	}
	return vec.size();
}

void CPrefabNode::FixObjRefNode1( map<int32, int32>& mapID, uint8* pObjData, SClassMetaData::SMemberData* pMemberData, void* pNameSpaceKey, int8 nPass )
{
	auto& ptr = *( TObjRef<CRenderObject2D>* )( pObjData );
	if( ptr._pt == pNameSpaceKey )
	{
		if( nPass )
		{
			auto itr = mapID.find( ptr._n );
			if( itr != mapID.end() )
				ptr._n = itr->second;
		}
		else if( ptr._n )
			mapID[ptr._n] = ptr._n;
	}
}

void CPrefabNode::UpdateTaggedNodePtrInfo()
{
	auto pClassData = GetFinalClassData();
	if( !pClassData )
		return;
	map<string, STaggedNodePtrInfo*> mapInfo;
	function<void( SClassMetaData::SMemberData* pData, uint32 nOfs )> func = [this]( SClassMetaData::SMemberData* pData, uint32 nOfs  )
	{
		if( pData->strTypeName == typeid( CPrefabNode ).name() )
		{
			auto pNode = m_pPrefab->GetNode( pData->strName.c_str() );
			if( pNode )
				m_vecTaggedPrefabNodePtrInfo.push_back( STaggedPrefabNodePtrInfo( pData, nOfs, pNode ) );
		}
		else
			m_vecTaggedNodePtrInfo.push_back( STaggedNodePtrInfo( pData, nOfs, 0 ) );
	};
	pClassData->FindAllTaggedPtr( func );
	for( auto& item : m_vecTaggedNodePtrInfo )
	{
		mapInfo[item.pMemberData->strName] = &item;
	}

	uint32 nIndex = 0;
	UpdateTaggedNodePtrInfo( nIndex, "", mapInfo );
}

void CPrefabNode::UpdateTaggedNodePtrInfo( uint32& nIndex, string curName, map<string, STaggedNodePtrInfo*>& mapInfo )
{
	if( nIndex > 0 )
	{
		auto itr = mapInfo.find( curName );
		if( itr != mapInfo.end() )
		{
			auto pInfo = itr->second;
			auto pClassData = GetFinalClassData();
			if( pInfo->pMemberData->pTypeData == CClassMetaDataMgr::Inst().GetClassData<CRenderObject2D>()
				|| pClassData && pClassData->Is( pInfo->pMemberData->pTypeData ) )
				pInfo->nChild = nIndex;
		}
	}
	nIndex++;

	for( CRenderObject2D* pChild = Get_RenderChild(); pChild; pChild = pChild->NextRenderChild() )
	{
		if( pChild == m_pRenderObject )
			continue;
		CPrefabNode* pNode = static_cast<CPrefabNode*>( pChild );
		pNode->UpdateTaggedNodePtrInfo( nIndex, curName.size() ? curName + "/" + pNode->GetName().c_str() : pNode->GetName().c_str(), mapInfo );
	}
}

void CPrefabNode::UpdateResPtrInfo()
{
	auto pClassData = GetClassData();
	if( !pClassData )
		return;
	function<void( SClassMetaData::SMemberData* pData, uint32 nOfs )> func = [this] ( SClassMetaData::SMemberData* pData, uint32 nOfs )
	{
		int8 nType;
		if( !pData )
			nType = 2;
		else if( pData->pTypeData == CClassMetaDataMgr::Inst().GetClassData<CString>() && ( pData->nFlag & 1 ) )
			nType = 0;
		else
			nType = 1;
		if( nType == 2 && m_vecResPtrInfo.back().nType == 1 )
			m_vecResPtrInfo.pop_back();
		else
			m_vecResPtrInfo.push_back( SResPtrInfo( pData, nOfs, nType ) );
	};
	pClassData->FindAllResPtr( func );
	auto pData = m_obj.GetObjData();
	CreateResPtr( pData, 0 );
}

int32 CPrefabNode::CreateResPtr( uint8* pObjData, int32 n )
{
	for( ; n < m_vecResPtrInfo.size(); n++ )
	{
		auto& info = m_vecResPtrInfo[n];
		if( info.nType == 0 )
		{
			if( !!( info.pMemberData->nFlag & 2 ) )
			{
				auto nArraySize = ( ( TArray<TResourceRef<CPrefab> >* )( pObjData + info.nOfs ) )->Size();
				if( nArraySize )
				{
					auto p = *(uint8**)( pObjData + info.nOfs + TArray<TResourceRef<CPrefab> >::GetPtrOfs() );
					for( int i = 0; i < nArraySize; i++, p += info.pMemberData->GetDataSize() )
					{
						auto& name = *(CString*)( p );
						auto pResource = CResourceManager::Inst()->CreateResource( info.pMemberData->nFlag >> 16, name.c_str_safe() );
						*( CReference<CResource>* )( p + TResourceRef<CPrefab>::GetPtrOfs() ) = pResource;
					}
				}
			}
			else
			{
				auto& name = *(CString*)( pObjData + info.nOfs );
				auto pResource = CResourceManager::Inst()->CreateResource( info.pMemberData->nFlag >> 16, name.c_str_safe() );
				*( CReference<CResource>* )( pObjData + info.nOfs + TResourceRef<CPrefab>::GetPtrOfs() ) = pResource;
			}
		}
		else if( info.nType == 1 )
		{
			auto nArraySize = ( ( TArray<TResourceRef<CPrefab> >* )( pObjData + info.nOfs ) )->Size();
			if( nArraySize )
			{
				auto p = *(uint8**)( pObjData + info.nOfs + TArray<TResourceRef<CPrefab> >::GetPtrOfs() );
				int32 n1;
				for( int i = 0; i < nArraySize; i++, p += info.pMemberData->GetDataSize() )
					n1 = CreateResPtr( p, n + 1 );
				n = n1;
			}
			else
			{
				n++;
				for( int32 k = 1; n < m_vecResPtrInfo.size(); n++ )
				{
					if( m_vecResPtrInfo[n].nType == 1 )
						k++;
					else if( m_vecResPtrInfo[n].nType == 2 )
					{
						k--;
						if( !k )
							break;
					}
				}
			}
		}
		else
			return n;
	}
	return m_vecResPtrInfo.size();
}

int32 CPrefabNode::CopyResPtr( uint8* pDst, uint8* pObjData, int32 n )
{
	for( ; n < m_vecResPtrInfo.size(); n++ )
	{
		auto& info = m_vecResPtrInfo[n];
		if( info.nType == 0 )
		{
			if( !!( info.pMemberData->nFlag & 2 ) )
			{
				auto nArraySize = ( ( TArray<TResourceRef<CPrefab> >* )( pObjData + info.nOfs ) )->Size();
				if( nArraySize )
				{
					auto p = *(uint8**)( pObjData + info.nOfs + TArray<TResourceRef<CPrefab> >::GetPtrOfs() ) + TResourceRef<CPrefab>::GetPtrOfs();
					auto p1 = *(uint8**)( pDst + info.nOfs + TArray<TResourceRef<CPrefab> >::GetPtrOfs() ) + TResourceRef<CPrefab>::GetPtrOfs();
					for( int i = 0; i < nArraySize; i++, p += info.pMemberData->GetDataSize(), p1 += info.pMemberData->GetDataSize() )
						*( CReference<CResource>* )p1 = *( CReference<CResource>* )p;
				}
			}
			else
			{
				*( CReference<CResource>* )( pDst + info.nOfs + TResourceRef<CPrefab>::GetPtrOfs() )
					= *( CReference<CResource>* )( pObjData + info.nOfs + TResourceRef<CPrefab>::GetPtrOfs() );
			}
		}
		else if( info.nType == 1 )
		{
			auto nArraySize = ( ( TArray<TResourceRef<CPrefab> >* )( pObjData + info.nOfs ) )->Size();
			if( nArraySize )
			{
				auto p = *(uint8**)( pObjData + info.nOfs + TArray<TResourceRef<CPrefab> >::GetPtrOfs() );
				auto p1 = *(uint8**)( pDst + info.nOfs + TArray<TResourceRef<CPrefab> >::GetPtrOfs() );
				int32 n1;
				for( int i = 0; i < nArraySize; i++, p += info.pMemberData->GetDataSize(), p1 += info.pMemberData->GetDataSize() )
					n1 = CopyResPtr( p1, p, n + 1 );
				n = n1;
			}
			else
			{
				n++;
				for( int32 k = 1; n < m_vecResPtrInfo.size(); n++ )
				{
					if( m_vecResPtrInfo[n].nType == 1 )
						k++;
					else if( m_vecResPtrInfo[n].nType == 2 )
					{
						k--;
						if( !k )
							break;
					}
				}
			}
		}
		else
			return n;
	}
	return m_vecResPtrInfo.size();
}

void CPrefabNode::UpdatePrefabNodeRefInfo()
{
	auto pClassData = GetClassData();
	if( !pClassData )
		return;
	function<void( SClassMetaData::SMemberData* pData, uint32 nOfs )> func = [this] ( SClassMetaData::SMemberData* pData, uint32 nOfs )
	{
		int8 nType;
		if( !pData )
			nType = 2;
		else if( pData->nType == SClassMetaData::SMemberData::eTypeCustomTaggedPtr && pData->strTypeName == typeid( CPrefabNode ).name() )
			nType = 0;
		else
			nType = 1;
		if( nType == 2 && m_vecPrefabNodeRefInfo.back().nType == 1 )
			m_vecPrefabNodeRefInfo.pop_back();
		else
			m_vecPrefabNodeRefInfo.push_back( SPrefabNodeRefInfo( pData, nOfs, nType, NULL ) );
	};
	pClassData->FindAllCustomTaggedPtr( func );
	auto pData = m_obj.GetObjData();
	CreatePrefabNodeRef( pData, 0 );
}

int32 CPrefabNode::CreatePrefabNodeRef( uint8* pObjData, int32 n )
{
	for( ; n < m_vecPrefabNodeRefInfo.size(); n++ )
	{
		auto& info = m_vecPrefabNodeRefInfo[n];
		if( info.nType == 0 )
		{
			if( !!( info.pMemberData->nFlag & 2 ) )
			{
				auto nArraySize = ( ( TArray<CPrefabNodeRef>* )( pObjData + info.nOfs ) )->Size();
				if( nArraySize )
				{
					auto p = *(uint8**)( pObjData + info.nOfs + TArray<CPrefabNodeRef>::GetPtrOfs() );
					for( int i = 0; i < nArraySize; i++, p += info.pMemberData->GetDataSize() )
					{
						auto& name = *(CString*)( p );
						auto pNode = name.length() ? m_pPrefab->GetNode( name.c_str_safe() ) : NULL;
						*( CSubReference<CPrefabNode>* )( p + CPrefabNodeRef::GetPtrOfs() ) = pNode;
					}
				}
			}
			else
			{
				auto& name = *(CString*)( pObjData + info.nOfs );
				auto pNode = name.length() ? m_pPrefab->GetNode( name.c_str_safe() ) : NULL;
				*( CSubReference<CPrefabNode>* )( pObjData + info.nOfs + CPrefabNodeRef::GetPtrOfs() ) = pNode;
			}
		}
		else if( info.nType == 1 )
		{
			auto nArraySize = ( ( TArray<CPrefabNodeRef>* )( pObjData + info.nOfs ) )->Size();
			if( nArraySize )
			{
				auto p = *(uint8**)( pObjData + info.nOfs + TArray<CPrefabNodeRef>::GetPtrOfs() );
				int32 n1;
				for( int i = 0; i < nArraySize; i++, p += info.pMemberData->GetDataSize() )
					n1 = CreatePrefabNodeRef( p, n + 1 );
				n = n1;
			}
			else
			{
				n++;
				for( int32 k = 1; n < m_vecPrefabNodeRefInfo.size(); n++ )
				{
					if( m_vecPrefabNodeRefInfo[n].nType == 1 )
						k++;
					else if( m_vecPrefabNodeRefInfo[n].nType == 2 )
					{
						k--;
						if( !k )
							break;
					}
				}
			}
		}
		else
			return n;
	}
	return m_vecPrefabNodeRefInfo.size();
}

int32 CPrefabNode::CopyPrefabNodeRef( uint8* pDst, uint8* pObjData, int32 n )
{
	for( ; n < m_vecPrefabNodeRefInfo.size(); n++ )
	{
		auto& info = m_vecPrefabNodeRefInfo[n];
		if( info.nType == 0 )
		{
			if( !!( info.pMemberData->nFlag & 2 ) )
			{
				auto nArraySize = ( ( TArray<CPrefabNodeRef>* )( pObjData + info.nOfs ) )->Size();
				if( nArraySize )
				{
					auto p = *(uint8**)( pObjData + info.nOfs + TArray<CPrefabNodeRef>::GetPtrOfs() ) + CPrefabNodeRef::GetPtrOfs();
					auto p1 = *(uint8**)( pDst + info.nOfs + TArray<CPrefabNodeRef>::GetPtrOfs() ) + CPrefabNodeRef::GetPtrOfs();
					for( int i = 0; i < nArraySize; i++, p += info.pMemberData->GetDataSize(), p1 += info.pMemberData->GetDataSize() )
						*( CSubReference<CPrefabNode>* )p1 = *( CSubReference<CPrefabNode>* )p;
				}
			}
			else
			{
				*( CSubReference<CPrefabNode>* )( pDst + info.nOfs + CPrefabNodeRef::GetPtrOfs() )
					= *( CSubReference<CPrefabNode>* )( pObjData + info.nOfs + CPrefabNodeRef::GetPtrOfs() );
			}
		}
		else if( info.nType == 1 )
		{
			auto nArraySize = ( ( TArray<CPrefabNodeRef>* )( pObjData + info.nOfs ) )->Size();
			if( nArraySize )
			{
				auto p = *(uint8**)( pObjData + info.nOfs + TArray<CPrefabNodeRef>::GetPtrOfs() );
				auto p1 = *(uint8**)( pDst + info.nOfs + TArray<CPrefabNodeRef>::GetPtrOfs() );
				int32 n1;
				for( int i = 0; i < nArraySize; i++, p += info.pMemberData->GetDataSize(), p1 += info.pMemberData->GetDataSize() )
					n1 = CopyPrefabNodeRef( p1, p, n + 1 );
				n = n1;
			}
			else
			{
				n++;
				for( int32 k = 1; n < m_vecPrefabNodeRefInfo.size(); n++ )
				{
					if( m_vecPrefabNodeRefInfo[n].nType == 1 )
						k++;
					else if( m_vecPrefabNodeRefInfo[n].nType == 2 )
					{
						k--;
						if( !k )
							break;
					}
				}
			}
		}
		else
			return n;
	}
	return m_vecPrefabNodeRefInfo.size();
}

void CPrefabNode::OnDebugDraw( CUIViewport* pViewport, IRenderSystem* pRenderSystem )
{
	auto pNode = m_obj.GetObject();
	if( pNode )
		pNode->OnDebugDraw( pViewport, pRenderSystem );
}

void CPrefabNode::GetPathString( CPrefabNode* pRoot, string& str )
{
	str = "";
	auto p = this;
	while( p )
	{
		if( p == pRoot )
		{
			str += ":";
			break;
		}
		str += ":";
		str += p->GetName();
		p = static_cast<CPrefabNode*>( p->GetParent() );
	}
}

CRenderObject2D* CPrefabNode::CreateInstance( vector<CRenderObject2D*>& vecInst, bool bNameSpace )
{
	UpdateDirty();

	CPrefabBaseNode* pPrefabNode = m_obj.CreateObject();
	if( pPrefabNode )
	{
		if( m_strName.length() )
			pPrefabNode->SetName( m_strName );
		if( m_pResource )
			pPrefabNode->SetResource( m_pResource );

		uint8* pRawData = (uint8*)pPrefabNode;
		pRawData -= m_obj.GetCastOffset();
		CopyResPtr( pRawData, m_obj.GetObjData(), 0 );
		CopyPrefabNodeRef( pRawData, m_obj.GetObjData(), 0 );
		if( bNameSpace )
			CopyObjPtrInfo( pRawData, m_obj.GetObjData(), 0 );
	}

	CRenderObject2D* pRenderObject = NULL;
	if( m_pResource )
	{
		if( m_pResource->GetResourceType() == eEngineResType_Prefab && m_pPatchedNode )
			pRenderObject = m_pPatchedNode->CreateInstance( bNameSpace );
		else
		{
			pRenderObject = CPrefabBaseNode::CreateRenderObjectByResource( m_pResource, pPrefabNode );
			if( m_pResource->GetResourceType() == eEngineResType_DrawableGroup )
			{
				auto nType = static_cast<CDrawableGroup*>( m_pResource.GetPtr() )->GetType();
				if( nType == CDrawableGroup::eType_Default )
				{
					CImage2D* pImage = static_cast<CImage2D*>( m_pRenderObject.GetPtr() );
					CImage2D* pImage1 = static_cast<CImage2D*>( pRenderObject );
					pImage1->SetRect( pImage->GetElem().rect );
					pImage1->SetTexRect( pImage->GetElem().texRect );
					uint16 nParamCount;
					CVector4* pParam = pImage->GetParam( nParamCount );
					CVector4* pParam1 = pImage1->GetParam( nParamCount );
					memcpy( pParam1, pParam, nParamCount * sizeof( CVector4 ) );
				}
				else if( nType == CDrawableGroup::eType_Rope )
				{
					CRopeObject2D* pRope = static_cast<CRopeObject2D*>( m_pRenderObject.GetPtr() );
					CRopeObject2D* pRope1 = static_cast<CRopeObject2D*>( pRenderObject );
					uint32 nData = pRope->GetData().data.size();
					pRope1->SetTransformDirty();
					pRope1->SetDataCount( nData );
					pRope1->SetSegmentsPerData( pRope->GetData().nSegmentsPerData );
					for( int i = 0; i < nData; i++ )
					{
						auto& data = pRope->GetData().data[i];
						pRope1->SetData( i, data.center, data.fWidth, data.tex0, data.tex1 );
					}
				}
				else if( nType == CDrawableGroup::eType_MultiFrame )
				{
					CMultiFrameImage2D* pMultiImage = static_cast<CMultiFrameImage2D*>( m_pRenderObject.GetPtr() );
					CMultiFrameImage2D* pMultiImage1 = static_cast<CMultiFrameImage2D*>( pRenderObject );

					pMultiImage1->SetFrames( pMultiImage->GetFrameBegin(), pMultiImage->GetFrameEnd(), pMultiImage->GetFramesPerSec() );
					pMultiImage1->SetPlaySpeed( pMultiImage->GetPlaySpeed(), pMultiImage->IsLoop() );
				}
				else if( nType == CDrawableGroup::eType_TileMap )
				{
					CTileMap2D* pTileMap = static_cast<CTileMap2D*>( m_pRenderObject.GetPtr() );
					CTileMap2D* pTileMap1 = static_cast<CTileMap2D*>( pRenderObject );

					pTileMap1->CopyData( pTileMap );
				}
			}
			else if( m_pResource->GetResourceType() == eEngineResType_ParticleSystem )
			{
				CParticleSystemObject* pParticle = static_cast<CParticleSystemObject*>( m_pRenderObject.GetPtr() );
				CParticleSystemObject* pParticle1 = static_cast<CParticleSystemObject*>( pRenderObject );
				pParticle->CopyData( pParticle1 );
			}
		}
	}
	else
	{
		if( m_nType == 1 )
			pRenderObject = new CDirectionalLightObject( *static_cast<CDirectionalLightObject*>( m_pRenderObject.GetPtr() ) );
		else if( m_nType == 2 )
			pRenderObject = new CPointLightObject( *static_cast<CPointLightObject*>( m_pRenderObject.GetPtr() ) );
	}
	if( pPrefabNode )
	{
		pPrefabNode->SetRenderObject( pRenderObject );
		pRenderObject = pPrefabNode;
	}
	else if( !pRenderObject )
		pRenderObject = new CRenderObject2D;
	else
	{
		auto p = SafeCast<CPrefabBaseNode>( pRenderObject );
		if( p )
		{
			if( m_strName.length() )
				p->SetName( m_strName );
		}
	}

	pRenderObject->x = x;
	pRenderObject->y = y;
	pRenderObject->r = r;
	pRenderObject->s = s;
	pRenderObject->SetZOrder( GetZOrder() );
	uint32 nBaseIndex = vecInst.size();
	vecInst.push_back( pRenderObject );

	vector<CRenderObject2D*> vecChildren;
	for( CRenderObject2D* pChild = Get_RenderChild(); pChild; pChild = pChild->NextRenderChild() )
	{
		if( pChild == m_pRenderObject )
			continue;
		CPrefabNode* pNode = static_cast<CPrefabNode*>( pChild );
		if( pNode )
			vecChildren.push_back( pNode );
	}
	for( int i = 0; i < vecChildren.size(); i++ )
	{
		vecChildren[i] = static_cast<CPrefabNode*>( vecChildren[i] )->CreateInstance( vecInst, bNameSpace );
	}
	for( int i = vecChildren.size() - 1; i >= 0; i-- )
	{
		pRenderObject->AddChild( vecChildren[i] );
	}

	for( auto& item : m_vecTaggedNodePtrInfo )
	{
		if( !item.nChild )
			continue;
		CRenderObject2D** ppObject = (CRenderObject2D**)( ( (uint8*)pPrefabNode ) - m_obj.GetCastOffset() + item.nOfs );
		auto pRenderObject = vecInst[nBaseIndex + item.nChild];
		pRenderObject->AddRef();
		*ppObject = pRenderObject;
	}
	for( auto& item : m_vecTaggedPrefabNodePtrInfo )
	{
		CPrefabNode** ppObject = (CPrefabNode**)( ( (uint8*)pPrefabNode ) - m_obj.GetCastOffset() + item.nOfs );
		auto pRenderObject = item.pPrefabNode;
		pRenderObject->AddRef();
		*ppObject = pRenderObject;
	}

	if( bNameSpace )
	{
		for( auto* pInfo : m_vecNameSpaceInfo )
		{
			pInfo->pCreatedInst = pRenderObject;
		}

		if( m_nameSpace.pNameSpaceKey )
			m_nameSpace.FillData();
	}
	return pRenderObject;
}

CResource * CPrefabNode::LoadResource( const char * szName )
{
	const char* szExt = GetFileExtension( szName );
	if( !strcmp( szExt, "mtl" ) )
		return CResourceManager::Inst()->CreateResource<CDrawableGroup>( szName );
	else if( !strcmp( szExt, "pts" ) )
		return CResourceManager::Inst()->CreateResource<CParticleFile>( szName );
	else if( !strcmp( szExt, "pf" ) )
		return CResourceManager::Inst()->CreateResource<CPrefab>( szName );
	return NULL;
}

void CPrefabNode::DebugDrawPreview( CUIViewport* pViewport, IRenderSystem* pRenderSystem )
{
	if( m_pPreviewNode )
		m_pPreviewNode->DebugDraw( pViewport, pRenderSystem );
}

void CPrefabNode::OnEditorActive( bool bActive )
{
	if( !m_pPatchedNode )
		return;
	if( bActive )
	{
		if( !m_pPreviewNode )
			return;
		SetRenderObject( m_pPatchedNode );
		m_pPreviewNode = NULL;
	}
	else
	{
		if( !m_pPatchedNode->m_obj.GetClassData() )
			return;
		if( !m_pPatchedNode->m_obj.GetObject()->IsPreview() )
			return;
		if( m_pPreviewNode )
			return;
		auto p = SafeCast<CPrefabBaseNode>( m_pPatchedNode->CreateInstance( false ) );
		m_pPreviewNode = p;
		SetRenderObject( m_pPreviewNode );
		p->OnPreview();
	}
}

void CPrefabNode::BindShaderResource( EShaderType eShaderType, const char* szName, IShaderResourceProxy* pShaderResource )
{
	CResource* pResource = GetResource();

	do
	{
		auto pPrefab = dynamic_cast<CPrefab*>( pResource );
		if( pPrefab )
		{
			pPrefab->GetRoot()->BindShaderResource( eShaderType, szName, pShaderResource );
			break;
		}

		auto pDrawableGroup = dynamic_cast<CDrawableGroup*>( pResource );
		if( pDrawableGroup )
		{
			pDrawableGroup->BindShaderResource( eShaderType, szName, pShaderResource );
			break;
		}

		auto pParticleSystem = dynamic_cast<CParticleSystem*>( pResource );
		if( pParticleSystem )
		{
			pParticleSystem->BindShaderResource( eShaderType, szName, pShaderResource );
			break;
		}
	} while( 0 );

	for( CRenderObject2D* pChild = Get_RenderChild(); pChild; pChild = pChild->NextRenderChild() )
	{
		if( pChild == m_pRenderObject )
			continue;
		CPrefabNode* pNode = dynamic_cast<CPrefabNode*>( pChild );
		if( pNode )
			pNode->BindShaderResource( eShaderType, szName, pShaderResource );
	}
}

//CPrefabNode* CPrefabNode::Clone( bool bForEditor )
//{
//	return Clone( bForEditor ? m_pPrefab : NULL );
//}

CPrefabNode* CPrefabNode::Clone( CPrefab* pPrefab, CPrefabNodeNameSpace* pNameSpace, SPatchContext* pContext, SNameSpaceCopyContext* pNameSpaceCopyContext, bool bForRootElem )
{
	CBufFile* pPatch = NULL;
	if( pContext )
	{
		string str;
		GetPathString( pContext->pRoot, str );
		auto itr = pContext->mapPatches.find( str );
		if( itr != pContext->mapPatches.end() )
		{
			pPatch = &itr->second;
			if( pPatch->GetBuffer() != pPatch->GetCurBuffer() )
				pPatch = NULL;
		}
	}

	CPrefabNode* pPrefabNode = new CPrefabNode( pPrefab );
	SNameSpaceCopyContext curNameSpaceCopyContext;
	if( bForRootElem )
	{
		pPrefabNode->m_nameSpace.pNameSpaceKey = pPrefab->GetNameSpaceKey();
		pNameSpace = &pPrefabNode->m_nameSpace;
	}
	else if( m_nameSpace.pNameSpaceKey )
	{
		curNameSpaceCopyContext.pSrc = &m_nameSpace;
		curNameSpaceCopyContext.pDst = &pPrefabNode->m_nameSpace;
		pPrefabNode->m_nameSpace.pNameSpaceKey = m_nameSpace.pNameSpaceKey;
		curNameSpaceCopyContext.pNext = pNameSpaceCopyContext;
		pNameSpaceCopyContext = &curNameSpaceCopyContext;
	}
	for( auto p = pNameSpaceCopyContext; p; p = p->pNext )
	{
		auto n = p->pSrc->FindIDByNode( this );
		if( n )
			p->pDst->Add( pPrefabNode, n );
	}

	pPrefabNode->m_nType = m_nType;
	if( pContext )
		pPrefabNode->m_bIsInstance = true;
	else
		pPrefabNode->m_bIsInstance = m_bIsInstance;
	if( pPatch )
	{
		CBufFile tmpBuf;
		pPatch->Read( tmpBuf );
		if( tmpBuf.GetBufLen() )
			m_obj.PatchData( pPrefabNode->m_obj, tmpBuf, pNameSpace->pNameSpaceKey );
		else
			m_obj.CopyData( pPrefabNode->m_obj );
	}
	else
		m_obj.CopyData( pPrefabNode->m_obj );
	pPrefabNode->m_strName = m_strName;
	pPrefabNode->x = x;
	pPrefabNode->y = y;
	pPrefabNode->r = r;
	pPrefabNode->s = s;
	pPrefabNode->SetZOrder( GetZOrder() );
	int8 nPatchFlag = 0;
	if( pPatch )
	{
		pPatch->Read( nPatchFlag );
		if( !!( nPatchFlag & 1 ) )
			pPatch->Read( pPrefabNode->x );
		if( !!( nPatchFlag & 2 ) )
			pPatch->Read( pPrefabNode->y );
		if( !!( nPatchFlag & 4 ) )
			pPatch->Read( pPrefabNode->r );
		if( !!( nPatchFlag & 8 ) )
			pPatch->Read( pPrefabNode->s );
	}

	bool bLoadedRes = false;
	if( !!( nPatchFlag & 16 ) )
	{
		CBufReader tmpBuf( *pPatch );
		auto nType = tmpBuf.Read<uint8>();
		if( nType == m_nType )
		{
			if( m_nType == 0 )
			{
				string strResName;
				tmpBuf.Read( strResName );
				CResource* pResource = LoadResource( strResName.c_str() );
				if( pResource )
				{
					pPrefabNode->LoadResourceExtraData( pResource, tmpBuf, pNameSpace );
					bLoadedRes = true;
				}
			}
			else
			{
				pPrefabNode->LoadOtherExtraData( tmpBuf );
				bLoadedRes = true;
			}
		}
	}
	else if( !!( nPatchFlag & 32 ) )
	{
		CBufReader tmpBuf( *pPatch );
		auto pRes = GetResource();
		if( pRes && pRes->GetResourceType() == eEngineResType_Prefab )
		{
			CPrefabNode* p = m_pPatchedNode;
			if( !p )
				p = static_cast<CPrefab*>( pRes )->GetRoot();

			SPatchContext context1( p );
			while( 1 )
			{
				string str;
				tmpBuf.Read( str );
				if( !str.length() )
					break;
				auto& buf = context1.mapPatches[str];
				buf.Clear();
				tmpBuf.Read( buf );
			}

			if( !pPrefab || pPrefab->CanAddDependency( pRes ) )
			{
				pPrefabNode->m_pResource = pRes;
				pPrefabNode->m_pPatchedNode = p->Clone( pPrefab, pNameSpace, &context1, pNameSpaceCopyContext );
				//SetRenderObject( m_pPatchedNode->CreateInstance() );
				pPrefabNode->SetRenderObject( pPrefabNode->m_pPatchedNode );
				pPrefab->AddDependency( pRes );
			}
			bLoadedRes = true;
		}
	}
	if( nPatchFlag & 64 )
	{
		auto nID = pPatch->Read<int32>();
		if( nID )
			pNameSpace->Add( pPrefabNode, nID );
	}

	if( !bLoadedRes )
	{
		if( m_pResource )
		{
			if( m_pResource->GetResourceType() == eEngineResType_Prefab && m_pPatchedNode )
			{
				pPrefabNode->m_pResource = m_pResource;
				pPrefabNode->m_pPatchedNode = m_pPatchedNode->Clone( pPrefab, pNameSpace, NULL, pNameSpaceCopyContext );
				//pPrefabNode->SetRenderObject( pPrefabNode->m_pPatchedNode->CreateInstance() );
				pPrefabNode->SetRenderObject( pPrefabNode->m_pPatchedNode );
			}
			else
			{
				pPrefabNode->SetResource( m_pResource );
				if( m_pResource->GetResourceType() == eEngineResType_DrawableGroup )
				{
					auto nType = static_cast<CDrawableGroup*>( m_pResource.GetPtr() )->GetType();
					if( nType == CDrawableGroup::eType_Default )
					{
						CImage2D* pRenderObject = static_cast<CImage2D*>( m_pRenderObject.GetPtr() );
						CImage2D* pRenderObject1 = static_cast<CImage2D*>( pPrefabNode->m_pRenderObject.GetPtr() );
						pRenderObject1->SetRect( pRenderObject->GetElem().rect );
						pRenderObject1->SetTexRect( pRenderObject->GetElem().texRect );
						uint16 nParamCount;
						CVector4* pParam = pRenderObject->GetParam( nParamCount );
						CVector4* pParam1 = pRenderObject1->GetParam( nParamCount );
						memcpy( pParam1, pParam, nParamCount * sizeof( CVector4 ) );
					}
					else if( nType == CDrawableGroup::eType_Rope )
					{
						CRopeObject2D* pRenderObject = static_cast<CRopeObject2D*>( m_pRenderObject.GetPtr() );
						CRopeObject2D* pRenderObject1 = static_cast<CRopeObject2D*>( pPrefabNode->m_pRenderObject.GetPtr() );
						uint32 nData = pRenderObject->GetData().data.size();
						pRenderObject1->SetTransformDirty();
						pRenderObject1->SetDataCount( nData );
						pRenderObject1->SetSegmentsPerData( pRenderObject->GetData().nSegmentsPerData );
						for( int i = 0; i < nData; i++ )
						{
							auto& data = pRenderObject->GetData().data[i];
							pRenderObject1->SetData( i, data.center, data.fWidth, data.tex0, data.tex1 );
						}
					}
					else if( nType == CDrawableGroup::eType_MultiFrame )
					{
						CMultiFrameImage2D* pRenderObject = static_cast<CMultiFrameImage2D*>( m_pRenderObject.GetPtr() );
						CMultiFrameImage2D* pRenderObject1 = static_cast<CMultiFrameImage2D*>( pPrefabNode->m_pRenderObject.GetPtr() );

						pRenderObject1->SetFrames( pRenderObject->GetFrameBegin(), pRenderObject->GetFrameEnd(), pRenderObject->GetFramesPerSec() );
						pRenderObject1->SetPlaySpeed( pRenderObject->GetPlaySpeed(), pRenderObject->IsLoop() );
					}
					else if( nType == CDrawableGroup::eType_TileMap )
					{
						CTileMap2D* pRenderObject = static_cast<CTileMap2D*>( m_pRenderObject.GetPtr() );
						CTileMap2D* pRenderObject1 = static_cast<CTileMap2D*>( pPrefabNode->m_pRenderObject.GetPtr() );

						pRenderObject1->CopyData( pRenderObject );
					}
				}
				else if( m_pResource->GetResourceType() == eEngineResType_ParticleSystem )
				{
					CParticleSystemObject* pParticle = static_cast<CParticleSystemObject*>( m_pRenderObject.GetPtr() );
					CParticleSystemObject* pParticle1 = static_cast<CParticleSystemObject*>( pPrefabNode->m_pRenderObject.GetPtr() );
					pParticle->CopyData( pParticle1 );
				}
			}
		}
		else
		{
			if( m_nType == 1 )
				pPrefabNode->SetRenderObject( new CDirectionalLightObject( *static_cast<CDirectionalLightObject*>( m_pRenderObject.GetPtr() ) ) );
			else if( m_nType == 2 )
				pPrefabNode->SetRenderObject( new CPointLightObject( *static_cast<CPointLightObject*>( m_pRenderObject.GetPtr() ) ) );
		}
	}

	vector<CPrefabNode*> vecChildren;
	for( CRenderObject2D* pChild = Get_RenderChild(); pChild; pChild = pChild->NextRenderChild() )
	{
		if( pChild == m_pRenderObject )
			continue;
		CPrefabNode* pNode = static_cast<CPrefabNode*>( pChild );
		if( pNode )
			vecChildren.push_back( pNode );
	}
	int32 nExtraNodes = 0;
	vector<pair<int32, CPrefabNode*> > vecExtraNodes;
	if( pPatch )
	{
		pPatch->Read( nExtraNodes );
		for( int i = 0; i < nExtraNodes; i++ )
		{
			int32 n = pPatch->Read<int32>();
			CPrefabNode* pChild = new CPrefabNode( pPrefab );
			pChild->Load( *pPatch, pNameSpace );
			vecExtraNodes.push_back( pair<int32, CPrefabNode*>( n, pChild ) );
		}
	}
	int32 i1 = 0;
	for( int i = 0; i < vecChildren.size(); i++ )
	{
		for( ; i1 < vecExtraNodes.size() && vecExtraNodes[i1].first == i; i1++ )
			pPrefabNode->AddChild( vecExtraNodes[i1].second );
		pPrefabNode->AddChild( vecChildren[vecChildren.size() - 1 - i]->Clone( pPrefab, pNameSpace, pContext, pNameSpaceCopyContext ) );
	}
	for( ; i1 < vecExtraNodes.size(); i1++ )
		pPrefabNode->AddChild( vecExtraNodes[i1].second );

	return pPrefabNode;
}

CRenderObject2D* CPrefabNode::CreateInstance( bool bNameSpace )
{
	if( bNameSpace )
	{
		if( m_bNamespaceDirty )
			UpdateNameSpace();
	}
	vector<CRenderObject2D*> vecInst;
	auto p = CreateInstance( vecInst, bNameSpace );
	vecInst.resize( 0 );
	return p;
}

void CPrefabNode::NameSpaceClearNode( CPrefabNode* pNode )
{
	m_nameSpace.Remove( pNode );

	if( pNode->m_pPatchedNode )
		NameSpaceClearNode( pNode->m_pPatchedNode );
	for( CRenderObject2D* pChild = pNode->Get_RenderChild(); pChild; pChild = pChild->NextRenderChild() )
	{
		if( pChild == pNode->m_pRenderObject )
			continue;
		CPrefabNode* pNode1 = SafeCast<CPrefabNode>( pChild );
		if( pNode1 )
			NameSpaceClearNode( pNode1 );
	}
}

void CPrefabNode::FixNameSpace()
{
	map<int32, int32> mapID;
	FixObjRef( mapID, m_nameSpace.pNameSpaceKey, 0 );
	m_nameSpace.Fix( mapID );
	FixObjRef( mapID, m_nameSpace.pNameSpaceKey, 1 );
}

void CPrefabNode::Load( IBufReader& buf, CPrefabNodeNameSpace* pNameSpace )
{
	buf.Read( m_nType );
	string strName;
	buf.Read( strName );
	int nPos = strName.find( '\x01' );
	if( nPos != string::npos )
	{
		auto str1 = strName.substr( nPos + 1 );
		int32 nNamespaceID = atoi( str1.c_str() );
		strName = strName.substr( 0, nPos );
		if( nNamespaceID )
			pNameSpace->Add( this, nNamespaceID );
	}
	m_strName = strName.c_str();
	buf.Read( x );
	buf.Read( y );
	buf.Read( r );
	buf.Read( s );
	SetZOrder( buf.Read<int32>() );
	string strResourceName;
	if( !m_nType )
		buf.Read( strResourceName );
	CBufReader extraData( buf );

	if( !m_nType )
	{
		CResource* pResource = LoadResource( strResourceName.c_str() );
		if( pResource )
			LoadResourceExtraData( pResource, extraData, pNameSpace );
	}
	else
	{
		LoadOtherExtraData( extraData );
	}
	m_obj.Load( buf, true, pNameSpace->pNameSpaceKey );

	uint32 nChildren = buf.Read<uint32>();
	for( int i = 0; i < nChildren; i++ )
	{
		CPrefabNode* pChild = new CPrefabNode( m_pPrefab );
		pChild->Load( buf, pNameSpace );
		AddChild( pChild );
	}
}

void CPrefabNode::Save( CBufFile& buf, CPrefabNodeNameSpace* pNameSpace )
{
	buf.Write( m_nType );
	string strName = m_strName;
	int32 nNamespaceID = pNameSpace->FindIDByNode( this );
	if( nNamespaceID )
	{
		char buf[32];
		itoa( nNamespaceID, buf, 10 );
		strName = strName + "\x01" + buf;
	}
	buf.Write( strName );
	buf.Write( x );
	buf.Write( y );
	buf.Write( r );
	buf.Write( s );
	buf.Write( GetZOrder() );
	if( !m_nType )
	{
		string strResourceName = m_pResource ? m_pResource->GetName() : "";
		buf.Write( strResourceName );
	}
	
	CBufFile extraData;
	if( !m_nType )
	{
		if( m_pResource )
			SaveResourceExtraData( m_pResource, extraData, pNameSpace );
	}
	else
		SaveOtherExtraData( extraData );
	buf.Write( extraData );

	m_obj.Save( buf, true );

	vector<CPrefabNode*> vecChildren;
	for( CRenderObject2D* pChild = Get_RenderChild(); pChild; pChild = pChild->NextRenderChild() )
	{
		if( pChild == m_pRenderObject )
			continue;
		CPrefabNode* pNode = static_cast<CPrefabNode*>( pChild );
		if( pNode )
			vecChildren.push_back( pNode );
	}
	uint32 nChildren = vecChildren.size();
	buf.Write( nChildren );
	for( int i = nChildren - 1; i >= 0; i-- )
	{
		vecChildren[i]->Save( buf, pNameSpace );
	}
}


class CPrefabNameSpaceKeyMgr
{
public:
	typedef string KeyType;
	KeyType* Get( const char* sz )
	{
		auto itr = m_items.find( sz );
		if( itr == m_items.end() )
		{
			m_items.insert( sz );
			itr = m_items.find( sz );
		}
		return (KeyType*)&( *m_items.find( sz ) );
	}
	const char* GetString( KeyType* p )
	{
		return p->c_str();
	}

	DECLARE_GLOBAL_INST_REFERENCE( CPrefabNameSpaceKeyMgr );
private:
	set<KeyType> m_items;
};


void CPrefabNode::FormatNamespaceString( void* pt, int32 n, string& result )
{
	stringstream ss;
	ss << CPrefabNameSpaceKeyMgr::Inst().GetString( (CPrefabNameSpaceKeyMgr::KeyType*)pt );
	ss << "#" << n;
	result = ss.str();
}

CPrefabNode* CPrefab::GetNode( const char* szName )
{
	if( !szName || !szName[0] )
		return m_pRoot;
	auto itr = m_mapNodes.find( szName );
	if( itr != m_mapNodes.end() )
		return itr->second;
	return NULL;
}

void* CPrefab::GetNameSpaceKey()
{
	return CPrefabNameSpaceKeyMgr::Inst().Get( GetName() );
}

void CPrefab::SetNode( CPrefabNode* pNode, const char* szName )
{
	if( szName && szName[0] )
		m_mapNodes[szName] = pNode;
	else
		m_pRoot = pNode;
}

void CPrefab::Create()
{
	if( strcmp( GetFileExtension( GetName() ), "pf" ) )
		return;
	vector<char> content;
	if( GetFileContent( content, GetName(), false ) == INVALID_32BITID )
		return;
	CBufReader buf( &content[0], content.size() );
	Load( buf );
	m_bCreated = true;
}

void CPrefab::Load( IBufReader& buf )
{
	m_pRoot = new CPrefabNode( this );
	m_pRoot->GetNameSpace().pNameSpaceKey = GetNameSpaceKey();
	m_pRoot->Load( buf, &m_pRoot->GetNameSpace() );

	int32 nExtraNodes;
	if( buf.CheckedRead( nExtraNodes ) )
	{
		for( int i = 0; i < nExtraNodes; i++ )
		{
			CString key( "" );
			buf.Read( key );
			auto pNode = new CPrefabNode( this );
			pNode->GetNameSpace().pNameSpaceKey = GetNameSpaceKey();
			pNode->Load( buf, &pNode->GetNameSpace() );
			m_mapNodes[key] = pNode;
		}
	}
}

void CPrefab::Save( CBufFile& buf )
{
	m_pRoot->Save( buf, &m_pRoot->GetNameSpace() );
	int32 nExtraNodes = m_mapNodes.size();
	buf.Write( nExtraNodes );
	for( auto& item : m_mapNodes )
	{
		buf.Write( item.first );
		item.second->Save( buf, &item.second->GetNameSpace() );
	}
}

int32 CPrefabNodeNameSpace::FindIDByNode( CPrefabNode * pNode )
{
	auto itr = mapNodeToID.find( pNode );
	return itr != mapNodeToID.end() ? itr->second : 0;
}

int32 CPrefabNodeNameSpace::FindOrGenIDByNode( CPrefabNode * pNode )
{
	auto itr = mapNodeToID.find( pNode );
	if( itr != mapNodeToID.end() )
		return itr->second;
	auto newID = nLastID + 1;
	Add( pNode, newID );
	return newID;
}

CPrefabNode* CPrefabNodeNameSpace::FindNodeByID( int32 nID )
{
	auto itr = mapIDToNode.find( nID );
	return itr != mapIDToNode.end() ? itr->second.GetPtr() : NULL;
}

void CPrefabNodeNameSpace::Add( CPrefabNode * pNode, int32 nID )
{
	mapIDToNode[nID] = pNode;
	mapNodeToID[pNode] = nID;
	nLastID = Max( nID, nLastID );
}

void CPrefabNodeNameSpace::Remove( CPrefabNode * pNode )
{
	int32 nID = FindIDByNode( pNode );
	if( !nID )
		return;
	mapIDToNode.erase( nID );
	mapNodeToID.erase( pNode );
}

void CPrefabNodeNameSpace::Fix( map<int32, int32>& result )
{
	vector<pair<int32, CReference<CPrefabNode> > > vec;
	for( auto itr = mapIDToNode.begin(); itr != mapIDToNode.end(); itr++ )
	{
		if( result.find( itr->first ) != result.end() )
			vec.push_back( *itr );
	}

	ClearInfo();
	mapIDToNode.clear();
	mapNodeToID.clear();
	nLastID = 0;
	for( auto& item : vec )
	{
		nLastID++;
		mapIDToNode[nLastID] = item.second;
		mapNodeToID[item.second] = nLastID;
		result[item.first] = nLastID;
	}
}

CPrefabNodeNameSpace::SNameSpaceInfo* CPrefabNodeNameSpace::AddNamespaceInfo( CPrefabNode * pNode )
{
	auto itr = mapNodeToID.find( pNode );
	if( itr == mapNodeToID.end() )
		return NULL;
	auto info = &mapNameSpaceInfos[itr->second];
	info->pClassData = pNode->GetFinalClassData();
	return info;
}

void CPrefabNodeNameSpace::CheckInfos()
{
	for( auto& item : vecInfos )
	{
		auto itr = mapNameSpaceInfos.find( item.second->nID );
		if( itr == mapNameSpaceInfos.end() )
			continue;
		if( !itr->second.pClassData->Is( item.second->pMemberData->pTypeData ) )
			continue;
		item.first = &itr->second;
	}
}

void CPrefabNodeNameSpace::FillData()
{
	for( auto& item : vecInfos )
	{
		if( !item.first || !item.first->pCreatedInst || !item.second->pCreatedNodeData )
			continue;
		item.first->pCreatedInst->AddRef();
		*(void**)item.second->pCreatedNodeData = item.first->pCreatedInst - item.second->nCastOffset;
	}
}
