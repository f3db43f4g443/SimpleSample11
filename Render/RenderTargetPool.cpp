#include "stdafx.h"
#include "Texture.h"
#include "FixedSizeAllocator.h"
#include "RenderSystem.h"

void CRenderTargetPool::Clear()
{
	for( auto& item : m_mapItems )
	{
		SItem* &pItems = item.second;
		while( pItems )
		{
			SItem* pItem = pItems;
			pItem->RemoveFrom_Item();
			pItem->pTexture = NULL;
			TObjectAllocator<SItem>::Inst().Free( pItem );
		}
	}
	m_mapItems.clear();
}

void CRenderTargetPool::AllocRenderTarget( CReference<ITexture>& pTexture, ETextureType eType, uint32 nDim1, uint32 nDim2, uint32 nDim3, uint32 nMipLevels, EFormat eFormat, void* data,
		bool bIsDynamic, bool bBindRenderTarget, bool bBindDepthStencil )
{
	CTextureDesc desc( eType, nDim1, nDim2, nDim3, nMipLevels, eFormat, bIsDynamic, bBindRenderTarget, bBindDepthStencil );
	AllocRenderTarget( pTexture, desc );
}

void CRenderTargetPool::AllocRenderTarget( CReference<ITexture>& pTexture, const CTextureDesc& desc )
{
	if( pTexture && pTexture->GetDesc() == desc )
		return;
	Release( pTexture );

	SItem* &pItems = m_mapItems[desc];
	if( pItems )
	{
		SItem* pItem = pItems;
		pItem->RemoveFrom_Item();
		pTexture = pItem->pTexture;
		pItem->pTexture = NULL;
		TObjectAllocator<SItem>::Inst().Free( pItem );
	}
	else
	{
		pTexture = IRenderSystem::Inst()->CreateTexture( desc.eType, desc.nDim1, desc.nDim2, desc.nDim3, desc.nMipLevels, desc.eFormat, NULL, desc.bIsDynamic, desc.bBindRenderTarget, desc.bBindDepthStencil );
	}
}

void CRenderTargetPool::Release( CReference<ITexture>& pTexture )
{
	if( !pTexture )
		return;
	SItem* &pItems = m_mapItems[pTexture->GetDesc()];
	SItem* pItem = new ( TObjectAllocator<SItem>::Inst().Alloc() ) SItem;
	pItem->pTexture = pTexture;
	pItem->InsertTo_Item( pItems );
	pTexture = NULL;
}