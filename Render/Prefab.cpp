#include "stdafx.h"
#include "Prefab.h"
#include "DrawableGroup.h"
#include "Rope2D.h"
#include "ParticleSystem.h"
#include "FileUtil.h"
#include "ResourceManager.h"
#include "LightRendering.h"
#include "Image2D.h"

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
		m_pRenderObject->RemoveThis();
		m_pRenderObject = NULL;
	}
	m_pRenderObject = pRenderObject;
	if( pRenderObject )
	{
		pRenderObject->SetZOrder( -1 );
		AddChild( pRenderObject );
		pRenderObject->SetZOrder( 0 );
	}
}

bool CPrefabNode::SetResource( CResource* pResource )
{
	if( m_nType )
		return false;
	if( m_pPrefab )
	{
		if( pResource == m_pResource )
			return true;
		if( pResource && !m_pPrefab->CanAddDependency( pResource ) )
			return false;
	}

	CRenderObject2D* pRenderObject = CreateRenderObjectByResource( pResource, this );
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

	if( m_pPrefab )
	{
		if( m_onResourceRefreshBegin.IsRegistered() )
			m_onResourceRefreshBegin.Unregister();
		if( m_onResourceRefreshEnd.IsRegistered() )
			m_onResourceRefreshEnd.Unregister();
		if( m_pResource )
			m_pPrefab->RemoveDependency( m_pResource );
	}
	m_pResource = pResource;
	if( m_pPrefab )
	{
		if( m_pResource )
		{
			m_pPrefab->AddDependency( m_pResource );
			m_pResource->RegisterRefreshBegin( &m_onResourceRefreshBegin );
			m_pResource->RegisterRefreshEnd( &m_onResourceRefreshEnd );
		}
	}
	SetRenderObject( pRenderObject );
	return true;
}

void CPrefabNode::OnResourceRefreshBegin()
{
	if( m_pResource->GetResourceType() == eEngineResType_Prefab )
		SetRenderObject( NULL );
}

void CPrefabNode::OnResourceRefreshEnd()
{
	CReference<CResource> pResource = m_pResource;
	m_pResource = NULL;
	if( m_pResource->GetResourceType() == eEngineResType_Prefab )
	{
		SetResource( pResource );
	}
	else if( m_pResource->GetResourceType() == eEngineResType_DrawableGroup )
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
		}
		else if( pPreTileMap && pCurTileMap )
		{
			pCurTileMap->CopyData( pPreTileMap );
		}
	}
}

void CPrefabNode::UpdateTaggedNodePtrInfo()
{
	if( !m_obj.GetClassData() )
		return;
	map<string, STaggedNodePtrInfo*> mapInfo;
	function<void( SClassMetaData::SMemberData* pData, uint32 nOfs )> func = [this, &mapInfo]( SClassMetaData::SMemberData* pData, uint32 nOfs  )
	{
		m_vecTaggedNodePtrInfo.push_back( STaggedNodePtrInfo( pData, nOfs, 0 ) );
	};
	m_obj.GetClassData()->FindAllTaggedPtr( func );
	for( auto& item : m_vecTaggedNodePtrInfo )
	{
		mapInfo[item.pMemberData->strName] = &item;
	}

	uint32 nIndex = 0;
	UpdateTaggedNodePtrInfo( nIndex, GetName(), mapInfo );
}

void CPrefabNode::UpdateTaggedNodePtrInfo( uint32& nIndex, string curName, map<string, STaggedNodePtrInfo*>& mapInfo )
{
	if( nIndex > 0 )
	{
		auto itr = mapInfo.find( curName );
		if( itr != mapInfo.end() )
		{
			auto pInfo = itr->second;
			if( pInfo->pMemberData->pTypeData == CClassMetaDataMgr::Inst().GetClassData<CRenderObject2D>()
				|| m_obj.GetClassData() && m_obj.GetClassData()->Is( pInfo->pMemberData->pTypeData ) )
				pInfo->nChild = nIndex;
		}
	}
	nIndex++;

	for( CRenderObject2D* pChild = Get_Child(); pChild; pChild = pChild->NextChild() )
	{
		if( pChild == m_pRenderObject )
			continue;
		CPrefabNode* pNode = static_cast<CPrefabNode*>( pChild );
		pNode->UpdateTaggedNodePtrInfo( nIndex, curName.size() ? curName + "/" + pNode->GetName() : pNode->GetName(), mapInfo );
	}
}

CRenderObject2D * CPrefabNode::CreateInstance( vector<CRenderObject2D*>& vecInst )
{
	if( !strcmp( m_pPrefab->GetName(), "data/bullets/bulletexp_eft.pf" ) )
	{
		int a = 0;
	}
	if( m_bTaggedNodePtrInfoDirty )
	{
		UpdateTaggedNodePtrInfo();
		m_bTaggedNodePtrInfoDirty = false;
	}

	CPrefabBaseNode* pPrefabNode = m_obj.CreateObject();
	if( pPrefabNode )
	{
		pPrefabNode->SetName( GetName() );
		if( m_pResource )
			pPrefabNode->SetResource( m_pResource );
	}

	CRenderObject2D* pRenderObject = NULL;
	if( m_pResource )
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

	pRenderObject->x = x;
	pRenderObject->y = y;
	pRenderObject->r = r;
	pRenderObject->s = s;
	pRenderObject->SetZOrder( GetZOrder() );
	uint32 nBaseIndex = vecInst.size();
	vecInst.push_back( pRenderObject );

	vector<CRenderObject2D*> vecChildren;
	for( CRenderObject2D* pChild = Get_Child(); pChild; pChild = pChild->NextChild() )
	{
		if( pChild == m_pRenderObject )
			continue;
		CPrefabNode* pNode = static_cast<CPrefabNode*>( pChild );
		if( pNode )
			vecChildren.push_back( pNode );
	}
	for( int i = 0; i < vecChildren.size(); i++ )
	{
		vecChildren[i] = static_cast<CPrefabNode*>( vecChildren[i] )->CreateInstance( vecInst );
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

	return pRenderObject;
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

	for( CRenderObject2D* pChild = Get_Child(); pChild; pChild = pChild->NextChild() )
	{
		if( pChild == m_pRenderObject )
			continue;
		CPrefabNode* pNode = dynamic_cast<CPrefabNode*>( pChild );
		if( pNode )
			pNode->BindShaderResource( eShaderType, szName, pShaderResource );
	}
}

CPrefabNode* CPrefabNode::Clone( bool bForEditor )
{
	return Clone( bForEditor ? m_pPrefab : NULL );
}

CPrefabNode* CPrefabNode::Clone( CPrefab* pPrefab )
{
	CPrefabNode* pPrefabNode = new CPrefabNode( pPrefab );
	pPrefabNode->m_nType = m_nType;
	m_obj.CopyData( pPrefabNode->m_obj );
	pPrefabNode->m_strName = m_strName;
	pPrefabNode->x = x;
	pPrefabNode->y = y;
	pPrefabNode->r = r;
	pPrefabNode->s = s;
	pPrefabNode->SetZOrder( GetZOrder() );
	pPrefabNode->SetResource( m_pResource );
	if( m_pResource )
	{
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
	else
	{
		if( m_nType == 1 )
			pPrefabNode->SetRenderObject( new CDirectionalLightObject( *static_cast<CDirectionalLightObject*>( m_pRenderObject.GetPtr() ) ) );
		else if( m_nType == 2 )
			pPrefabNode->SetRenderObject( new CPointLightObject( *static_cast<CPointLightObject*>( m_pRenderObject.GetPtr() ) ) );
	}

	vector<CPrefabNode*> vecChildren;
	for( CRenderObject2D* pChild = Get_Child(); pChild; pChild = pChild->NextChild() )
	{
		if( pChild == m_pRenderObject )
			continue;
		CPrefabNode* pNode = static_cast<CPrefabNode*>( pChild );
		if( pNode )
			vecChildren.push_back( pNode );
	}
	for( int i = vecChildren.size() - 1; i >= 0; i-- )
	{
		pPrefabNode->AddChild( vecChildren[i]->Clone( pPrefab ) );
	}

	return pPrefabNode;
}

CRenderObject2D* CPrefabNode::CreateInstance()
{
	vector<CRenderObject2D*> vecInst;
	return CreateInstance( vecInst );
}

void CPrefabNode::Load( IBufReader& buf )
{
	buf.Read( m_nType );
	buf.Read( m_strName );
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
		CResource* pResource = NULL;
		const char* szExt = GetFileExtension( strResourceName.c_str() );
		if( !strcmp( szExt, "mtl" ) )
			pResource = CResourceManager::Inst()->CreateResource<CDrawableGroup>( strResourceName.c_str() );
		else if( !strcmp( szExt, "pts" ) )
			pResource = CResourceManager::Inst()->CreateResource<CParticleFile>( strResourceName.c_str() );
		else if( !strcmp( szExt, "pf" ) )
			pResource = CResourceManager::Inst()->CreateResource<CPrefab>( strResourceName.c_str() );
		if( pResource )
		{
			SetResource( pResource );
			if( pResource->GetResourceType() == eEngineResType_DrawableGroup )
			{
				auto nType = static_cast<CDrawableGroup*>( m_pResource.GetPtr() )->GetType();
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
				}
				else if( nType == CDrawableGroup::eType_TileMap )
				{
					CTileMap2D* pTileMap = static_cast<CTileMap2D*>( m_pRenderObject.GetPtr() );
					pTileMap->LoadData( extraData );
				}
			}
			else if( m_pResource->GetResourceType() == eEngineResType_ParticleSystem )
			{
				CParticleSystemObject* pParticle = static_cast<CParticleSystemObject*>( m_pRenderObject.GetPtr() );
				pParticle->LoadExtraData( extraData );
			}
		}
	}
	else
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

	m_obj.Load( buf, true );

	uint32 nChildren = buf.Read<uint32>();
	for( int i = 0; i < nChildren; i++ )
	{
		CPrefabNode* pChild = new CPrefabNode( m_pPrefab );
		pChild->Load( buf );
		AddChild( pChild );
	}
}

void CPrefabNode::Save( CBufFile& buf )
{
	buf.Write( m_nType );
	buf.Write( m_strName );
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
		{
			if( m_pResource->GetResourceType() == eEngineResType_DrawableGroup )
			{
				auto nType = static_cast<CDrawableGroup*>( m_pResource.GetPtr() )->GetType();
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
				}
				else if( nType == CDrawableGroup::eType_TileMap )
				{
					CTileMap2D* pTileMap = static_cast<CTileMap2D*>( m_pRenderObject.GetPtr() );
					pTileMap->SaveData( extraData );
				}
			}
			else if( m_pResource->GetResourceType() == eEngineResType_ParticleSystem )
			{
				CParticleSystemObject* pParticle = static_cast<CParticleSystemObject*>( m_pRenderObject.GetPtr() );
				pParticle->SaveExtraData( extraData );
			}
		}
	}
	else
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
	buf.Write( extraData );

	m_obj.Save( buf, true );

	vector<CPrefabNode*> vecChildren;
	for( CRenderObject2D* pChild = Get_Child(); pChild; pChild = pChild->NextChild() )
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
		vecChildren[i]->Save( buf );
	}
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
	m_pRoot->Load( buf );
}

void CPrefab::Save( CBufFile& buf )
{
	m_pRoot->Save( buf );
}