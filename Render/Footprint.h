#pragma once
#include "Texture.h"
#include "Math3D.h"
#include "DefaultDrawable2D.h"
#include "Image2D.h"
#include "FixedSizeAllocator.h"
#include "Canvas.h"

#define FOOTPRINT_RENDER_TARGET_SIZE 2048u
class CFootprintReceiver : public CImage2D
{
	friend class CFootprintMgr;
public:
	CFootprintReceiver( CDrawable2D* pUpdateDrawable, CDrawable2D* pDrawable, CDrawable2D* pOcclusionDrawable, bool bGUI = false );
	~CFootprintReceiver();

	struct SFootprintInfo
	{
		float fTime;
		CRectangle rect;
		LINK_LIST( SFootprintInfo, Footprint );
	};

	struct SFootprintUpdateInfo
	{
		uint32 nSrcTexIndex;
		uint32 nDstTexIndex;
		CRectangle srcRTRect;
		CRectangle dstRTRect;
		CRectangle srcLogicRect;
		CRectangle dstLogicRect;
	};

	void SetMgr( CFootprintMgr* pMgr );
	void AddFootprint( CElement2D* pElement2D, float fTime );
	void SetReservedBound( const CRectangle& rect )
	{
		float left = floor( rect.x );
		float top = floor( rect.y );
		float right = ceil( rect.x + rect.width );
		float bottom = ceil( rect.y + rect.height );
		m_reservedBound = CRectangle( left, top, right - left, bottom - top );
	}
	void SetPersistent( const CRectangle& logicRect );
	void SetAutoRemove( bool bAutoRemove ) { m_bAutoRemove = bAutoRemove; }
	void ToPersistentCanvas( float fTime = 0 ) { m_fTimeToPersistentCanvas = m_fTime + fTime; }
	void EnableAutoSplit( float fLimitSize, float fToPersistentAfterSplit = -1 )
	{
		m_bAutoSplit = true;
		m_fLimitSize = Min( Max( fLimitSize, 32.0f ), (float)FOOTPRINT_RENDER_TARGET_SIZE ) - 2;
		m_allFootprintsBound = CRectangle( 0, 0, 0, 0 );
		m_fToPersistentAfterSplit = fToPersistentAfterSplit;
	}
	void DisableAutoSplit() { m_bAutoSplit = false; }
	void Split();

	ITexture* GetTexture();
	bool Update( float fElapsedTime );
	void SetFootprintRectExtension( uint32 nRectExtension ) { m_nRectExtension = nRectExtension; }
	CDrawable2D* GetUpdateDrawable() { return m_pUpdateDrawable; }
	void SetUpdateDrawable( CDrawable2D* pDrawable ) { m_pUpdateDrawable = pDrawable; }

	virtual void Render( CRenderContext2D& context ) override;
private:
	CDrawable2D* m_pUpdateDrawable;
	SFootprintUpdateInfo m_updateInfo;
	CFootprintMgr* m_pMgr;
	
	bool m_bPersistent;
	bool m_bAutoRemove;
	bool m_bAutoSplit;
	bool m_bToPersistentCanvas;
	float m_fLimitSize;
	float m_fToPersistentAfterSplit;
	CRectangle m_reservedBound;
	CRectangle m_allFootprintsBound;
	float m_fTime;
	float m_fTimeToPersistentCanvas;
	uint32 m_nRectExtension;
	SFootprintInfo m_footPrintTail;
	LINK_LIST_HEAD( m_pFootprints, SFootprintInfo, Footprint )
	LINK_LIST_HEAD( m_pElement, CElement2D, Element );
	LINK_LIST_REF( CFootprintReceiver, Receiver );
};

class CFootprintMgr : public CReferenceObject
{
public:
	CFootprintMgr();
	~CFootprintMgr() { Clear(); }

	void Clear();
	void AddReceiver( CFootprintReceiver* pReceiver );
	void RemoveReceiver( CFootprintReceiver* pReceiver );
	CRenderObject2D* GetFootprintRoot() { return m_pFootprintRoot; }
	void SetFootprintRoot( CRenderObject2D* pRenderObject ) { m_pFootprintRoot = pRenderObject; }

	ITexture* GetTexture( uint32 nIndex ) { return nIndex < m_vecRenderTargets.size() ? m_vecRenderTargets[nIndex] : NULL; }
	ITexture* GetPersistentTexture() { return m_persistentCanvas.GetTexture(); }
	void Update( float fElapsedTime, IRenderSystem* pRenderSystem );

	CElement2D* AllocElement();
private:
	void UpdateRenderTarget( CRenderTargetPool& renderTargetPool, CFootprintReceiver** pBegin, CFootprintReceiver** pEnd, ITexture* pSrc, CReference<ITexture>& pDst, CRenderContext2D& context );
	CCanvas m_persistentCanvas;

	vector<CReference<ITexture> > m_vecRenderTargets;
	CReference<CRenderObject2D> m_pFootprintRoot;

	CReference<CFootprintReceiver> m_pPersistentReceiver;
	TObjectAllocator<CElement2D> m_tempAllocator;
	LINK_LIST_REF_HEAD( m_pReceivers, CFootprintReceiver, Receiver );
	LINK_LIST_REF( CFootprintMgr, FootprintMgr );
};

class CFootprintUpdateDrawable : public CDefaultDrawable2D
{
public:
	void LoadXml( TiXmlElement* pRoot );
protected:
	virtual void OnApplyMaterial( CRenderContext2D& context ) override;
private:
	CShaderParamShaderResource m_paramTex;
};

class CFootprintDrawable : public CDefaultDrawable2D
{
public:
	void LoadXml( TiXmlElement* pRoot );
protected:
	virtual bool OnFlushElement( CRenderContext2D& context, CElement2D* pElement, bool bBreak ) override;
private:
	CShaderParamShaderResource m_paramTex;
};