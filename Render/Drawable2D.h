#pragma once
#include "Element2D.h"
#include "RenderContext2D.h"
#include "BufFile.h"

class CDrawable2D
{
public:
	CDrawable2D() : m_bOpaque( true ), m_pElement( NULL ) {}
	virtual ~CDrawable2D() {}
	bool HasElement() { return m_pElement != NULL; }
	void AddElement( CElement2D* pElement )
	{
		Insert_Element( pElement );
	}

	IBlendState* GetBlendState( uint16 nType );
	IBlendState* LoadBlendState( IBufReader& buf );
	uint16 GetBlendStateIndex( IBlendState* pState );
	void SaveBlendState( CBufFile& buf, IBlendState* pState );
	IBlendState* LoadBlendState( const char* szBlendType );

	virtual void Flush( CRenderContext2D& context ) = 0;
	bool IsOpaque() { return m_bOpaque; }
protected:
	bool m_bOpaque;
private:
	LINK_LIST( CDrawable2D, Drawable );
	LINK_LIST_HEAD( m_pElement, CElement2D, Element );
};
