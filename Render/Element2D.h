#pragma once

#include "Reference.h"
#include "LinkList.h"
#include "Math3D.h"

class CDrawable2D;
class CElement2D
{
	friend class CDrawable2D;
public:
	CElement2D() : pInstData( NULL ), nInstDataSize( 0 ), depth( -1 ), specialOfs( 0, 0 ) {}
	CDrawable2D* GetDrawable() { return m_pDrawable; }
	void SetDrawable( CDrawable2D* pDrawable ) { m_pDrawable = pDrawable; }
	void OnFlushed() { depth = -1; RemoveFrom_Element(); }

	CRectangle rect;
	CRectangle texRect;
	CMatrix2D worldMat;
	CVector2 specialOfs;
	void* pInstData;
	uint32 nInstDataSize;

	int32 depth;
private:
	CDrawable2D* m_pDrawable;
	LINK_LIST( CElement2D, Element );
};
