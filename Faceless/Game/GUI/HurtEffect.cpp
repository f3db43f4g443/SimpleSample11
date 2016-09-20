#include "stdafx.h"
#include "HurtEffect.h"
#include "Render/Image2D.h"
#include "Render/DefaultDrawable2D.h"
#include "MyGame.h"
#include "Common/FileUtil.h"
#include "Common/xml.h"

class CHurtEffectBloodBite : public CImage2D
{
public:
	CHurtEffectBloodBite( CDrawable2D* pDrawable, const CRectangle& rect, const CRectangle& texRect )
		: CImage2D( pDrawable, NULL, rect, texRect, true ), m_tick( this, &CHurtEffectBloodBite::OnTick )
	{
		m_element2D.nInstDataSize = sizeof( m_alphaScaleOffset );
		m_element2D.pInstData = &m_alphaScaleOffset;
	}

	class CFactory : public CHurtEffectMgr::IFactory
	{
	public:
		CFactory()
		{
			vector<char> content;
			GetFileContent( content, "materials/blood_bite.xml", true );
			TiXmlDocument doc;
			doc.LoadFromBuffer( &content[0] );
			CDefaultDrawable2D* pDrawable = new CDefaultDrawable2D;
			pDrawable->LoadXml( doc.RootElement()->FirstChildElement( "gui_pass" ) );
			m_pDrawable = pDrawable;

			m_rect = CRectangle( -128, -128, 256, 256 );
			m_texRect = CRectangle( 0, 0, 1, 1 );
		}

		virtual CRenderObject2D* Create( const CVector2& ofs ) override
		{
			CRenderObject2D* pRenderObject = new CHurtEffectBloodBite( m_pDrawable, m_rect, m_texRect );
			CVector2 maxOfs = CGame::Inst().GetScreenResolution() * 0.3f;
			CVector2 fixedOfs = ofs * CVector2( 1.0f / maxOfs.x, 1.0f / maxOfs.y );
			fixedOfs.x = atan( fixedOfs.x ) / PI;
			fixedOfs.y = atan( fixedOfs.y ) / PI;
			fixedOfs = fixedOfs * maxOfs;
			pRenderObject->x = fixedOfs.x;
			pRenderObject->y = fixedOfs.y;
			return pRenderObject;
		}
	private:
		CDrawable2D* m_pDrawable;
		CRectangle m_rect;
		CRectangle m_texRect;
	};
protected:
	virtual void OnAdded() override
	{
		CGame::Inst().Register( 1, &m_tick );
		m_t = 0;
		UpdateMaterial();
	}

	virtual void OnRemoved() override
	{
		if( m_tick.IsRegistered() )
			m_tick.Unregister();
	}

	void OnTick()
	{
		m_t += CGame::Inst().GetElapsedTimePerTick();
		UpdateMaterial();
		if( m_t < 4.5f )
			CGame::Inst().Register( 1, &m_tick );
		else
			RemoveThis();
	}

	void UpdateMaterial()
	{
		if( m_t < 1.75f )
		{
			float fMin = Max( 0.999f - m_t * 4.0f, 0.0f );
			float fMax = 1;
			m_alphaScaleOffset.x = 1.0f /  ( fMax - fMin );
			m_alphaScaleOffset.y = -fMin * m_alphaScaleOffset.x;
		}
		else
		{
			m_alphaScaleOffset.x = Max( ( 2.0f - m_t ) * 4.0f, 0.0f );
			m_alphaScaleOffset.y = 0;
		}
	}
private:
	TClassTrigger<CHurtEffectBloodBite> m_tick;
	float m_t;
	CVector2 m_alphaScaleOffset;
};

CHurtEffectMgr::CHurtEffectMgr()
{
	Register( "blood_bite", new CHurtEffectBloodBite::CFactory );
}

void CHurtEffectMgr::Register( const char* szEffect, IFactory* pFactory )
{
	m_mapFactories[szEffect] = pFactory;
}

CRenderObject2D* CHurtEffectMgr::Create( const char* szEffect, const CVector2& ofs )
{
	auto itr = m_mapFactories.find( szEffect );
	if( itr != m_mapFactories.end() )
		return itr->second->Create( ofs );
	return NULL;
}