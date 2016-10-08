#include "stdafx.h"
#include "Useable.h"
#include "Stage.h"
#include "GUI/HUDCircle.h"
#include "Render/Font.h"
#include "Render/FontRendering.h"
#include "Common/ResourceManager.h"
#include "Common/FileUtil.h"
#include "Common/xml.h"
#include "Common/Utf8Util.h"

CFontFile* GetDefaultFont()
{
	return CResourceManager::Inst()->CreateResource<CFontFile>( "bluehigh.ttf" );
}

CFontDrawable* GetDefaultFontDrawable()
{
	static CFontDrawable* pDrawable;
	if( !pDrawable )
	{
		pDrawable = new CFontDrawable;

		string strPath = "materials/default_font.xml";
		vector<char> content;
		GetFileContent( content, strPath.c_str(), true );
		TiXmlDocument doc;
		doc.LoadFromBuffer( &content[0] );
		pDrawable->LoadXml( doc.RootElement() );
		doc.Clear();
	}
	return pDrawable;
}

CUseable::CUseable( const char* szText, float fTime, float fCircleSize )
	: m_strText( szText ), m_fTime( fTime ), m_fCurTime( 0 ), m_bEnabled( true ), m_fCircleSize( fCircleSize )
{
	SetHitType( eEntityHitType_Sensor );
	
	CFontObject* pFontObject = new CFontObject( GetDefaultFont(), 24, GetDefaultFontDrawable(), NULL,
		CRectangle( -fCircleSize, -fCircleSize, fCircleSize * 2, fCircleSize * 2 ), 0, true );
	
	pFontObject->SetAlignment( CFontObject::eAlign_Center | ( CFontObject::eAlign_Center << 2 ) );
	pFontObject->SetText( Utf8ToUnicode( szText ).c_str() );
	pFontObject->SetGlobalClip( false, CRectangle() );
	m_pText = pFontObject;
	AddChild( m_pText );
	m_pText->bVisible = false;
}

CUseable::CUseable( const SClassCreateContext& context ) : CEntity( context ), m_strText( context ), m_fCurTime( 0 ), m_bEnabled( true )
{
	SetHitType( eEntityHitType_Sensor );
	
	CFontObject* pFontObject = new CFontObject( GetDefaultFont(), 24, GetDefaultFontDrawable(), NULL,
		CRectangle( -m_fCircleSize, -m_fCircleSize, m_fCircleSize * 2, m_fCircleSize * 2 ), 0, true );
	
	pFontObject->SetAlignment( CFontObject::eAlign_Center | ( CFontObject::eAlign_Center << 2 ) );
	pFontObject->SetText( Utf8ToUnicode( m_strText.c_str() ).c_str() );
	pFontObject->SetGlobalClip( false, CRectangle() );
	m_pText = pFontObject;
	AddChild( m_pText );
	m_pText->bVisible = false;
}

void CUseable::SetText( const char* szText )
{
	m_strText = szText;
	CFontObject* pFontObject = dynamic_cast<CFontObject*>( m_pText.GetPtr() );
	pFontObject->SetText( Utf8ToUnicode( szText ).c_str() );
}

void CUseable::SetUsing( bool bUsing, float fTime )
{
	if( bUsing )
	{
		m_fCurTime += fTime;
		if( m_fCurTime >= m_fTime )
		{
			m_fCurTime = 0;
			Use();
		}
		if( !m_pPercentCircle )
		{
			m_pPercentCircle = new CHUDCircle( m_fCircleSize, 32.0f, CVector4( 0, 0, 0, 1 ), CVector4( 1, 1, 1, 1 ), true );
			m_pPercentCircle->SetParentEntity( this );
			m_pText->bVisible = true;
		}
		static_cast<CHUDCircle*>( m_pPercentCircle.GetPtr() )->SetPercent( m_fCurTime / m_fTime );
	}
	else
	{
		m_fCurTime = 0;
		if( m_pPercentCircle )
		{
			m_pPercentCircle->SetParentEntity( NULL );
			m_pPercentCircle = NULL;
			m_pText->bVisible = false;
		}
	}
}

void CUseable::ResetUsing()
{
	m_fCurTime = 0;
	if( m_pPercentCircle )
	{
		static_cast<CHUDCircle*>( m_pPercentCircle.GetPtr() )->SetPercent( m_fCurTime / m_fTime );
	}
}

CFastUseable::CFastUseable( const char* szText, float fTime, float fCircleSize )
	: CUseable( szText, fTime, fCircleSize )
	, m_onUse( this, &CFastUseable::OnUse )
{
}

CFastUseable::CFastUseable( const SClassCreateContext& context )
	: CUseable( context )
	, m_onUse( this, &CFastUseable::OnUse )
{
}

void CFastUseable::OnAddedToStage()
{
	RegisterEntityEvent( eEntityEvent_PlayerUse, &m_onUse );
}

void CFastUseable::OnRemovedFromStage()
{
	if( m_onUse.IsRegistered() )
		m_onUse.Unregister();
}