#include "stdafx.h"
#include "Footprint.h"
#include "FixedSizeAllocator.h"
#include "MathUtil.h"
#include <algorithm>

CFootprintReceiver::CFootprintReceiver( CDrawable2D* pUpdateDrawable, CDrawable2D* pDrawable, CDrawable2D* pOcclusionDrawable, bool bGUI )
	: CImage2D( pDrawable, pOcclusionDrawable, CRectangle( 0, 0, 0, 0 ), CRectangle( 0, 0, 0, 0 ), bGUI )
	, m_pUpdateDrawable( pUpdateDrawable )
	, m_pMgr( NULL )
	, m_nRectExtension( 0 )
	, m_pFootprints( NULL )
	, m_pElement( NULL )
	, m_reservedBound( 0, 0, 0, 0 )
	, m_allFootprintsBound( 0, 0, 0, 0 )
	, m_bPersistent( false )
	, m_bAutoRemove( false )
	, m_bAutoSplit( false )
	, m_bToPersistentCanvas( false )
	, m_fTimeToPersistentCanvas( -1 )
	, m_fToPersistentAfterSplit( -1 )
{
	m_element2D.pInstData = this;
	Insert_Footprint( &m_footPrintTail );
	m_updateInfo.srcLogicRect = m_updateInfo.dstLogicRect = CRectangle( 0, 0, 0, 0 );
}

CFootprintReceiver::~CFootprintReceiver()
{
	while( m_pElement )
		m_pElement->RemoveFrom_Element();
	while( m_pFootprints )
	{
		auto* pFootprint = m_pFootprints;
		pFootprint->RemoveFrom_Footprint();
		if( pFootprint != &m_footPrintTail )
			TObjectAllocator<SFootprintInfo>::Inst().Free( pFootprint );
	}
}

void CFootprintReceiver::SetMgr( CFootprintMgr* pMgr )
{
	if( pMgr == m_pMgr )
		return;
	CReference<CFootprintReceiver> temp = this;
	if( m_pMgr )
		m_pMgr->RemoveReceiver( this );
	if( pMgr )
		pMgr->AddReceiver( this );
}

void CFootprintReceiver::AddFootprint( CElement2D* pElement2D, float fTime )
{
	if( !m_pMgr )
		return;
	SFootprintInfo* pInfo = (SFootprintInfo*)TObjectAllocator<SFootprintInfo>::Inst().Alloc();
	CRectangle& bound = pInfo->rect;
	bound = pElement2D->rect * pElement2D->worldMat;

	float left = floor( bound.x - m_nRectExtension );
	float top = floor( bound.y - m_nRectExtension );
	float right = ceil( bound.x + bound.width + m_nRectExtension );
	float bottom = ceil( bound.y + bound.height + m_nRectExtension );
	bound.x = left;
	bound.y = top;
	bound.width = right - left;
	bound.height = bottom - top;
	pInfo->fTime = fTime;

	if( m_bAutoSplit )
	{
		CRectangle bound1 = m_allFootprintsBound = m_allFootprintsBound.width > 0 ? m_allFootprintsBound + bound : bound;
		if( bound1.width > m_fLimitSize || bound1.height > m_fLimitSize )
		{
			Split();
			m_allFootprintsBound = bound;
		}
		else
			m_allFootprintsBound = bound1;
	}

	CElement2D* pElem1 = m_pMgr->AllocElement();
	memcpy( pElem1, pElement2D, sizeof( CElement2D ) );
	Insert_Element( pElem1 );
	if( !m_footPrintTail.NextFootprint() || m_footPrintTail.NextFootprint()->fTime != fTime )
		m_footPrintTail.InsertAfter_Footprint( pInfo );
	else
	{
		m_footPrintTail.NextFootprint()->rect = m_footPrintTail.NextFootprint()->rect + pInfo->rect;
		TObjectAllocator<SFootprintInfo>::Inst().Free( pInfo );
	}
}

void CFootprintReceiver::SetPersistent( const CRectangle& logicRect )
{
	m_bPersistent = true;
	m_updateInfo.dstLogicRect = logicRect;
	m_updateInfo.dstRTRect = CRectangle( 0, 0, logicRect.width, logicRect.height );
}

void CFootprintReceiver::Split()
{
	if( !GetParent() )
		return;

	bool bGUI = m_pGUIDrawable;
	CFootprintReceiver* pReceiver = new CFootprintReceiver( m_pUpdateDrawable, !bGUI? m_pColorDrawable: m_pGUIDrawable, !bGUI? m_pOcclusionDrawable: NULL, bGUI );
	pReceiver->SetMgr( m_pMgr );
	pReceiver->SetAutoRemove( true );
	pReceiver->m_fTime = m_fTime;
	if( m_fTimeToPersistentCanvas >= m_fTime )
		m_fTimeToPersistentCanvas -= m_fTime;
	m_fTime = 0;
	pReceiver->m_nRectExtension = m_nRectExtension;
	if( m_fToPersistentAfterSplit >= 0 )
		pReceiver->ToPersistentCanvas( m_fToPersistentAfterSplit );

	memcpy( &pReceiver->m_updateInfo, &m_updateInfo, sizeof( m_updateInfo ) );
	m_updateInfo.nDstTexIndex = m_updateInfo.nSrcTexIndex = INVALID_32BITID;
	m_updateInfo.srcLogicRect = m_updateInfo.dstLogicRect = CRectangle( 0, 0, 0, 0 );

	if( m_pFootprints != &m_footPrintTail )
	{
		m_pFootprints->Transplant_Footprint( pReceiver->m_pFootprints );
		m_footPrintTail.Transplant_Footprint( m_pFootprints );
	}
	if( m_footPrintTail.NextFootprint() )
	{
		m_footPrintTail.NextFootprint()->Transplant_Footprint( pReceiver->m_footPrintTail.NextFootprint() );
	}
	
	pReceiver->x = x;
	pReceiver->y = y;
	pReceiver->r = r;
	GetParent()->AddChildAfter( pReceiver, this );
	GetParent()->MoveToTopmost( this );
}

ITexture* CFootprintReceiver::GetTexture()
{
	if( !m_pMgr )
		return NULL;
	if( m_bPersistent )
		return m_pMgr->GetPersistentTexture();
	else
		return m_pMgr->GetTexture( m_updateInfo.nDstTexIndex );
}

bool CFootprintReceiver::Update( float fElapsedTime )
{
	if( m_bPersistent )
	{
		m_element2D.rect = m_localBound = m_updateInfo.dstLogicRect;
		return true;
	}

	m_updateInfo.srcLogicRect = m_updateInfo.dstLogicRect;
	m_updateInfo.srcRTRect = m_updateInfo.dstRTRect;
	m_updateInfo.nSrcTexIndex = m_updateInfo.nDstTexIndex;

	bool bUpdateSize = false;
	m_fTime += fElapsedTime;
	if( m_fTimeToPersistentCanvas >= 0 && m_fTime > m_fTimeToPersistentCanvas )
		m_bToPersistentCanvas = true;

	SFootprintInfo* pFirst = NULL;
	while( m_pFootprints != &m_footPrintTail )
	{
		auto pFootprint = m_pFootprints;
		pFirst = pFootprint;
		if( pFootprint->fTime > m_fTime )
			break;
		bUpdateSize = true;
		pFootprint->RemoveFrom_Footprint();
		TObjectAllocator<SFootprintInfo>::Inst().Free( pFootprint );
	}

	while( m_footPrintTail.NextFootprint() )
	{
		SFootprintInfo* pLast = m_footPrintTail.NextFootprint();
		pLast->fTime += m_fTime;
		if( !bUpdateSize )
			m_updateInfo.dstLogicRect = m_updateInfo.dstLogicRect.width > 0 && m_updateInfo.dstLogicRect.height > 0 ? m_updateInfo.dstLogicRect + pLast->rect : pLast->rect;
		if( pFirst )
		{
			CRectangle rect1 = pFirst->rect + pLast->rect;
			if( rect1.width < pLast->rect.width * 1.5f && rect1.height < pLast->rect.width * 1.5f )
			{
				pFirst->rect = rect1;
				pFirst->fTime = pLast->fTime;
				pLast->RemoveFrom_Footprint();
				continue;
			}
		}
		pFirst = pLast;
		m_footPrintTail.Shift_Footprint();
	}

	if( m_bAutoRemove && !m_bToPersistentCanvas && m_pFootprints == &m_footPrintTail )
		return false;

	if( bUpdateSize )
	{
		if( m_pFootprints == &m_footPrintTail )
			m_updateInfo.dstLogicRect = CRectangle( 0, 0, 0, 0 );
		else
		{
			m_updateInfo.dstLogicRect = m_pFootprints->rect;
			for( auto pFootprint = m_pFootprints->NextFootprint(); pFootprint != &m_footPrintTail; pFootprint = pFootprint->NextFootprint() )
			{
				m_updateInfo.dstLogicRect = m_updateInfo.dstLogicRect + pFootprint->rect;
			}
		}
	}

	if( m_reservedBound.width > 0 && m_reservedBound.height > 0 )
	{
		if( m_updateInfo.dstLogicRect.width > 0 && m_updateInfo.dstLogicRect.height > 0 )
			m_updateInfo.dstLogicRect = m_updateInfo.dstLogicRect + m_reservedBound;
		else
			m_updateInfo.dstLogicRect = m_reservedBound;
	}
	m_element2D.rect = m_localBound = m_updateInfo.dstLogicRect;
	SetBoundDirty();
	float fSize = m_updateInfo.dstLogicRect.width <= 0 || m_updateInfo.dstLogicRect.height <= 0? 0: Max( m_updateInfo.dstLogicRect.width, m_updateInfo.dstLogicRect.height );
	m_updateInfo.dstRTRect.width = m_updateInfo.dstRTRect.height = Min( FOOTPRINT_RENDER_TARGET_SIZE, Max( 32u, Pow2Ceil( fSize + 2 ) ) );
	return true;
}

void CFootprintReceiver::Render( CRenderContext2D& context )
{
	if( !m_pMgr || !GetTexture() )
		return;
	
	auto& texRect = m_element2D.texRect;
	if( m_bPersistent )
	{
		texRect = CRectangle( 0, 0, 1, 1 );
	}
	else
	{
		texRect.x = m_updateInfo.dstRTRect.x + 1;
		texRect.y = m_updateInfo.dstRTRect.y + 1;
		texRect.width = m_updateInfo.dstLogicRect.width;
		texRect.height = m_updateInfo.dstLogicRect.height;
		texRect = texRect * ( 1.0f / FOOTPRINT_RENDER_TARGET_SIZE );
		texRect.y = 1 - texRect.y - texRect.height;
	}
	CImage2D::Render( context );
}

CFootprintMgr::CFootprintMgr()
	: m_pReceivers( NULL )
	, m_tempAllocator( 4096 )
	, m_persistentCanvas( false, 0, 0, EFormat::EFormatR8G8B8A8UNorm, CCanvas::eDepthStencilType_None )
{
	m_persistentCanvas.SetRoot( new CRenderObject2D );
}

void CFootprintMgr::Clear()
{
	while( m_pReceivers )
		m_pReceivers->SetMgr( NULL );
	auto& renderTargetPool = CRenderTargetPool::GetSizeIndependentPool();
	for( auto& tex : m_vecRenderTargets )
	{
		renderTargetPool.Release( tex );
	}
}

void CFootprintMgr::AddReceiver( CFootprintReceiver* pReceiver )
{
	Insert_Receiver( pReceiver );
	pReceiver->m_pMgr = this;
	pReceiver->m_fTime = 0;
	auto& info = pReceiver->m_updateInfo;
	info.nDstTexIndex = info.nSrcTexIndex = INVALID_32BITID;

	if( pReceiver->m_bPersistent )
	{
		if( m_pPersistentReceiver )
			m_pPersistentReceiver->SetMgr( NULL );
		m_pPersistentReceiver = pReceiver;
	}
}

void CFootprintMgr::RemoveReceiver( CFootprintReceiver* pReceiver )
{
	if( m_pPersistentReceiver.GetPtr() == pReceiver )
		m_pPersistentReceiver = NULL;
	pReceiver->m_pMgr = NULL;
	Remove_Receiver( pReceiver );
}

void CFootprintMgr::Update( float fElapsedTime, IRenderSystem* pRenderSystem )
{
	vector<CReference<ITexture> > vecPreRTs;
	vector<uint32> vecPreRTReceiversLeft;
	for( auto& item : m_vecRenderTargets )
		vecPreRTs.push_back( item );
	vecPreRTReceiversLeft.resize( vecPreRTs.size() );
	m_vecRenderTargets.clear();

	vector<CFootprintReceiver*> vecReceivers;
	for( auto pReceiver = m_pReceivers; pReceiver; )
	{
		if( !pReceiver->Update( fElapsedTime ) )
		{
			auto pReceiver1 = pReceiver;
			pReceiver = pReceiver->NextReceiver();
			pReceiver1->SetMgr( NULL );
			pReceiver1->RemoveThis();
			continue;
		}

		if( pReceiver->m_updateInfo.nSrcTexIndex != INVALID_32BITID )
			vecPreRTReceiversLeft[pReceiver->m_updateInfo.nSrcTexIndex]++;
		if( !pReceiver->m_bPersistent )
			vecReceivers.push_back( pReceiver );
		pReceiver = pReceiver->NextReceiver();
	}
	auto& renderTargetPool = CRenderTargetPool::GetSizeIndependentPool();
	for( int i = 0; i < vecPreRTs.size(); i++ )
	{
		if( !vecPreRTReceiversLeft[i] )
			renderTargetPool.Release( vecPreRTs[i] );
	}
	if( !vecReceivers.size() )
		return;

	SViewport viewport = { 0, 0, FOOTPRINT_RENDER_TARGET_SIZE, FOOTPRINT_RENDER_TARGET_SIZE, 0, 1 };
	pRenderSystem->SetViewports( &viewport, 1 );
	pRenderSystem->SetDepthStencilState( IDepthStencilState::Get<false, EComparisonAlways>() );
	pRenderSystem->SetRasterizerState( IRasterizerState::Get<>() );

	CRenderContext2D context;
	context.pRenderSystem = pRenderSystem;
	context.screenRes = context.lightMapRes = CVector2( FOOTPRINT_RENDER_TARGET_SIZE, FOOTPRINT_RENDER_TARGET_SIZE );
	context.dTime = pRenderSystem->GetElapsedTime();
	context.rectScene = context.rectViewport = CRectangle( 0, 0, FOOTPRINT_RENDER_TARGET_SIZE, FOOTPRINT_RENDER_TARGET_SIZE );
	
	CMatrix& mat = context.mat;
	mat.Identity();
	mat.m00 = 2.0f / context.rectScene.width;
	mat.m11 = 2.0f / context.rectScene.height;
	mat.m22 = 0.0f;
	mat.m03 = -context.rectScene.GetCenterX() * mat.m00;
	mat.m13 = -context.rectScene.GetCenterY() * mat.m11;
	mat.m23 = 0.5f;
	
	struct SLess
	{
		bool operator () ( CFootprintReceiver* pLeft, CFootprintReceiver* pRight )
		{
			if( pLeft->m_updateInfo.dstRTRect.width > pRight->m_updateInfo.dstRTRect.width )
				return true;
			if( pLeft->m_updateInfo.dstRTRect.width < pRight->m_updateInfo.dstRTRect.width )
				return false;
			if( pLeft->m_updateInfo.nSrcTexIndex < pRight->m_updateInfo.nSrcTexIndex )
				return true;
			if( pLeft->m_updateInfo.nSrcTexIndex > pRight->m_updateInfo.nSrcTexIndex )
				return false;
			return pLeft->GetUpdateDrawable() > pRight->GetUpdateDrawable();
		}
	};
	std::sort( vecReceivers.begin(), vecReceivers.end(), SLess() );

	uint32 nRT = 0;
	uint32 nCur = 0;
	for( int i = 0; i < vecReceivers.size(); i++ )
	{
		uint32 nWidth = vecReceivers[i]->m_updateInfo.dstRTRect.width;
		if( !nWidth )
		{
			vecReceivers[i]->m_updateInfo.nDstTexIndex = INVALID_32BITID;
			continue;
		}
		uint16 x, y;
		ZCurveOrderInv( nCur, x, y );
		vecReceivers[i]->m_updateInfo.dstRTRect.x = x;
		vecReceivers[i]->m_updateInfo.dstRTRect.y = y;
		vecReceivers[i]->m_updateInfo.nDstTexIndex = nRT;

		nCur += nWidth * nWidth;
		if( nCur >= FOOTPRINT_RENDER_TARGET_SIZE * FOOTPRINT_RENDER_TARGET_SIZE )
		{
			nCur = 0;
			nRT++;
		}
	}
	if( nCur )
		nRT++;
	m_vecRenderTargets.resize( nRT );

	uint32 nSrcRT = INVALID_32BITID;
	uint32 nDstRT = INVALID_32BITID;
	CFootprintReceiver** ppReceiver = NULL;
	for( auto& pReceiver : vecReceivers )
	{
		uint32 nSrcIndex = pReceiver->m_updateInfo.nSrcTexIndex;
		uint32 nDstIndex = pReceiver->m_updateInfo.nDstTexIndex;
		if( nSrcIndex == INVALID_32BITID || nDstIndex == INVALID_32BITID )
			continue;
		if( !pReceiver->GetUpdateDrawable() )
			continue;
		if( nSrcIndex != nSrcRT || nDstIndex != nDstRT )
		{
			if( nSrcRT != INVALID_32BITID )
			{
				UpdateRenderTarget( renderTargetPool, ppReceiver, &pReceiver, vecPreRTs[nSrcRT], m_vecRenderTargets[nDstRT], context );
			
				if( !vecPreRTReceiversLeft[nSrcRT] )
					renderTargetPool.Release( vecPreRTs[nSrcRT] );
			}
			nSrcRT = nSrcIndex;
			nDstRT = nDstIndex;
			ppReceiver = &pReceiver;
		}

		vecPreRTReceiversLeft[nSrcIndex]--;
	}

	if( nSrcRT != INVALID_32BITID )
	{
		UpdateRenderTarget( renderTargetPool, ppReceiver, &vecReceivers.back() + 1, vecPreRTs[nSrcRT], m_vecRenderTargets[nDstRT], context );
		renderTargetPool.Release( vecPreRTs[nSrcRT] );
	}
	for( auto& dst : m_vecRenderTargets )
	{
		if( !dst )
			UpdateRenderTarget( renderTargetPool, NULL, NULL, NULL, dst, context );
	}
	
	nDstRT = INVALID_32BITID;
	CDrawable2D* pDrawables = NULL;
	for( CFootprintReceiver* pReceiver : vecReceivers )
	{
		if( !pReceiver->Get_Element() )
			continue;
		uint32 nDstIndex = pReceiver->m_updateInfo.nDstTexIndex;
		if( nDstIndex == INVALID_32BITID )
			continue;
		if( nDstIndex != nDstRT )
		{
			while( pDrawables )
			{
				pDrawables->Flush( context );
				pDrawables->RemoveFrom_Drawable();
			}
			nDstRT = nDstIndex;
			pRenderSystem->SetRenderTarget( m_vecRenderTargets[nDstRT]->GetRenderTarget(), NULL );
		}

		CVector2 ofs;
		ofs.x = pReceiver->m_updateInfo.dstRTRect.x + 1 - pReceiver->m_updateInfo.dstLogicRect.x;
		ofs.y = pReceiver->m_updateInfo.dstRTRect.y + 1 - pReceiver->m_updateInfo.dstLogicRect.y;
		while( pReceiver->Get_Element()  )
		{
			CElement2D* pElement = pReceiver->Get_Element();
			pElement->RemoveFrom_Element();
			
			pElement->worldMat.m02 += ofs.x;
			pElement->worldMat.m12 += ofs.y;
			pElement->specialOfs = ofs;

			CDrawable2D* pDrawable = pElement->GetDrawable();
			if( !pDrawable->HasElement() )
			{
				pDrawable->InsertTo_Drawable( pDrawables );
			}
			pDrawable->AddElement( pElement );
		}
	}
	while( pDrawables )
	{
		pDrawables->Flush( context );
		pDrawables->RemoveFrom_Drawable();
	}
	m_tempAllocator.Clear();

	if( m_pPersistentReceiver )
	{
		vecReceivers.clear();
		for( auto pRenderObject = m_pFootprintRoot->Get_Child(); pRenderObject; pRenderObject = pRenderObject->NextChild() )
		{
			auto pReceiver = dynamic_cast<CFootprintReceiver*>( pRenderObject );
			if( pReceiver && !pReceiver->m_bPersistent )
			{
				if( pReceiver->m_bToPersistentCanvas )
					vecReceivers.push_back( pReceiver );
				else if( pReceiver->m_fTimeToPersistentCanvas >= 0 || pReceiver->m_bAutoRemove )
					vecReceivers.clear();
			}
		}

		if( vecReceivers.size() )
		{
			for( int i = vecReceivers.size() - 1; i >= 0; i-- )
			{
				auto pReceiver = vecReceivers[i];
				pReceiver->RemoveThis();
				m_persistentCanvas.GetRoot()->AddChild( pReceiver );
			}
			m_persistentCanvas.GetRoot()->CalcAABB();
			context.eRenderPass = eRenderPass_Color;
			auto& cam = m_persistentCanvas.GetCamera();
			cam.SetViewport( m_pPersistentReceiver->m_updateInfo.dstRTRect );
			cam.SetViewArea( m_pPersistentReceiver->m_updateInfo.dstLogicRect );
			m_persistentCanvas.SetSize( m_pPersistentReceiver->m_updateInfo.dstRTRect.GetSize() );
			m_persistentCanvas.Render( context );

			for( int i = vecReceivers.size() - 1; i >= 0; i-- )
			{
				vecReceivers[i]->RemoveThis();
				vecReceivers[i]->SetMgr( NULL );
			}
		}
	}
}

CElement2D* CFootprintMgr::AllocElement()
{
	return new ( m_tempAllocator.Alloc() ) CElement2D;
}

void CFootprintMgr::UpdateRenderTarget( CRenderTargetPool& renderTargetPool, CFootprintReceiver** pBegin, CFootprintReceiver** pEnd, ITexture* pSrc, CReference<ITexture>& pDst, CRenderContext2D& context )
{
	IRenderSystem* pRenderSystem = context.pRenderSystem;
	if( !pDst )
	{
		renderTargetPool.AllocRenderTarget( pDst, ETextureType::Tex2D, FOOTPRINT_RENDER_TARGET_SIZE, FOOTPRINT_RENDER_TARGET_SIZE, 1, 0, EFormat::EFormatR8G8B8A8UNorm, NULL, false, true );
		pRenderSystem->ClearRenderTarget( CVector4( 0, 0, 0, 0 ), pDst->GetRenderTarget() );
	}

	uint32 nElems = pEnd - pBegin;
	if( !nElems )
		return;
	pRenderSystem->SetRenderTarget( pDst->GetRenderTarget(), NULL );
	CElement2D* pElems = (CElement2D*)alloca( nElems * sizeof( CElement2D ) );
	for( int i = 0; i < nElems; i++ )
		new ( pElems + i ) CElement2D();
	nElems = 0;
	CDrawable2D* pDrawables = NULL;
	for( ; pBegin != pEnd; pBegin++ )
	{
		CFootprintReceiver* pReceiver = *pBegin;
		auto& info = pReceiver->m_updateInfo;
		if( info.nSrcTexIndex == INVALID_32BITID )
			continue;

		CRectangle copyLogicRect = info.srcLogicRect * info.dstLogicRect;
		if( copyLogicRect.width <= 0 || copyLogicRect.height <= 0 )
			continue;
		CRectangle srcRect;
		CRectangle dstRect;
		srcRect.x = info.srcRTRect.x + 1 + copyLogicRect.x - info.srcLogicRect.x;
		srcRect.y = info.srcRTRect.y + 1 + copyLogicRect.y - info.srcLogicRect.y;
		dstRect.x = info.dstRTRect.x + 1 + copyLogicRect.x - info.dstLogicRect.x;
		dstRect.y = info.dstRTRect.y + 1 + copyLogicRect.y - info.dstLogicRect.y;
		srcRect.width = dstRect.width = copyLogicRect.width;
		srcRect.height = dstRect.height = copyLogicRect.height;
		
		CElement2D& elem = pElems[nElems++];
		elem.rect = dstRect;
		elem.texRect = srcRect * ( 1.0f / FOOTPRINT_RENDER_TARGET_SIZE );
		elem.texRect.y = 1.0f - elem.texRect.y - elem.texRect.height;
		elem.worldMat.Identity();
		elem.pInstData = pSrc;

		CDrawable2D* pDrawable = pReceiver->GetUpdateDrawable();
		if( !pDrawable->HasElement() )
		{
			pDrawable->InsertTo_Drawable( pDrawables );
		}
		pDrawable->AddElement( &elem );
	}
	
	while( pDrawables )
	{
		pDrawables->Flush( context );
		pDrawables->RemoveFrom_Drawable();
	}
}

void CFootprintUpdateDrawable::OnApplyMaterial( CRenderContext2D& context )
{
	CElement2D* pElem = m_pElement;
	if( !pElem )
		return;
	ITexture* pTex = (ITexture*)pElem->pInstData;
	if( m_paramTex.bIsBound )
		m_paramTex.Set( context.pRenderSystem, pTex->GetShaderResource() );
}

void CFootprintUpdateDrawable::LoadXml( TiXmlElement* pRoot )
{
	CDefaultDrawable2D::LoadXml( pRoot );
	IShader* pPS = m_material.GetShader( EShaderType::PixelShader );
	if( pPS )
	{
		auto& info = pPS->GetShaderInfo();
		info.Bind( m_paramTex, "Texture0" );
	}
}

bool CFootprintDrawable::OnFlushElement( CRenderContext2D& context, CElement2D* pElement, bool bBreak )
{
	CFootprintReceiver* pReceiver = (CFootprintReceiver*)pElement->pInstData;
	ITexture* pTex = pReceiver->GetTexture();

	bBreak = bBreak || ( pElement->NextElement() && ( (CFootprintReceiver*)pElement->NextElement()->pInstData )->GetTexture() != pTex );
	if( bBreak && m_paramTex.bIsBound )
		m_paramTex.Set( context.pRenderSystem, pTex->GetShaderResource() );
	return bBreak;
}

void CFootprintDrawable::LoadXml( TiXmlElement* pRoot )
{
	CDefaultDrawable2D::LoadXml( pRoot );
	IShader* pPS = m_material.GetShader( EShaderType::PixelShader );
	if( pPS )
	{
		auto& info = pPS->GetShaderInfo();
		info.Bind( m_paramTex, "Texture0" );
	}
}