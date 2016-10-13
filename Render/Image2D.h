#pragma once
#include "RenderObject2D.h"

class CImage2D : public CRenderObject2D
{
public:
	CImage2D( CDrawable2D* pDrawable, CDrawable2D* pOcclusionDrawable, const CRectangle& rect, const CRectangle& texRect, bool bGUI = false );

	const CElement2D& GetElem() { return m_element2D; }
	CDrawable2D* GetColorDrawable() { return m_pColorDrawable; }
	CDrawable2D* GetOcclusionDrawable() { return m_pOcclusionDrawable; }
	CDrawable2D* GetGUIDrawable() { return m_pGUIDrawable; }
	void SetColorDrawable( CDrawable2D* pDrawable ) { m_pColorDrawable = pDrawable; }
	void SetOcclusionDrawable( CDrawable2D* pDrawable ) { m_pOcclusionDrawable = pDrawable; }
	void SetGUIDrawable( CDrawable2D* pDrawable ) { m_pGUIDrawable = pDrawable; }
	void SetRect( const CRectangle& rect ) { m_localBound = m_element2D.rect = rect; }
	void SetTexRect( const CRectangle& texRect ) { m_element2D.texRect = texRect; }
	void SetInstData( void* pInstData, uint32 nInstDataSize ) { m_element2D.pInstData = pInstData; m_element2D.nInstDataSize = nInstDataSize; }
	
	CVector4* GetParam();
	CVector4* GetParam( uint16& nTotalCount );
	void SetParam( uint16 nTotalCount, const CVector4* pData,
		uint16 nColorParamBeginIndex, uint16 nColorParamCount,
		uint16 nOcclusionParamBeginIndex, uint16 nOcclusionParamCount,
		uint16 nGUIParamBeginIndex, uint16 nGUIParamCount );

	virtual void Render( CRenderContext2D& context ) override;
protected:
	CElement2D m_element2D;
	CDrawable2D* m_pColorDrawable;
	CDrawable2D* m_pOcclusionDrawable;
	CDrawable2D* m_pGUIDrawable;

	uint16 m_nColorParamBeginIndex, m_nColorParamCount;
	uint16 m_nOcclusionParamBeginIndex, m_nOcclusionParamCount;
	uint16 m_nGUIParamBeginIndex, m_nGUIParamCount;
	vector<CVector4> m_params;
};

struct SImage2DFrameData
{
	SImage2DFrameData() : bound( 0, 0, 0, 0 ), fFramesPerSec( 0 ) {}
	CRectangle bound;
	float fFramesPerSec;
	struct SFrame
	{
		CRectangle rect;
		CRectangle texRect;
		vector<CVector4> params;
	};
	vector<SFrame> frames;
};

class CMultiFrameImage2D : public CImage2D
{
public:
	CMultiFrameImage2D( CDrawable2D* pDrawable, CDrawable2D* pOcclusionDrawable, SImage2DFrameData* pData, bool bGUI = false );
	SImage2DFrameData* GetData() { return m_pData; }
	void SetFrames( uint32 nBegin, uint32 nEnd, float fFramesPerSec );
	uint32 GetFrameBegin() { return m_nFrameBegin; }
	uint32 GetFrameEnd() { return m_nFrameEnd; }
	float GetFramesPerSec() { return m_fFramesPerSec; }

	virtual void OnTransformUpdated() override;
protected:
	void UpdateImage();
	SImage2DFrameData* m_pData;
	float m_fCurTime;
	uint32 m_nCurFrame;
	uint32 m_nFrameBegin;
	uint32 m_nFrameEnd;
	float m_fFramesPerSec;
};