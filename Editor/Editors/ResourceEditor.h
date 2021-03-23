#pragma once
#include "UICommon/UIViewport.h"
#include "UICommon/UIManager.h"
#include "Common/ResourceManager.h"
#include "Common/FileUtil.h"
#include "Render/Image2D.h"
#include "Render/DefaultDrawable2D.h"
#include "Render/LightRendering.h"
#include "Common/xml.h"

class CResourceEditor : public CUIElement
{
public:
	virtual void NewFile( const char* szFileName ) {}
	virtual void SetFileName( const char* szFileName, const char* szParam ) {}
	static bool& IsLighted() { static bool b = true; return b; }
};

template <class T>
class TResourceEditor : public CResourceEditor
{
public:
	virtual void NewFile( const char* szFileName ) override
	{
		m_pRes = CResourceManager::Inst()->CreateResource<T>( szFileName, true );
		Save();
	}

	const char* GetFileName()
	{
		if( !m_pRes )
			return "";
		return m_pRes->GetName();
	}
	virtual void SetFileName( const char* szFileName, const char* szParam = "" ) override
	{
		CReference<CResource> pTemp = m_pRes;
		m_pRes = szFileName && szFileName[0] ? CResourceManager::Inst()->CreateResource<T>( szFileName ) : NULL;

		m_mapParams.clear();
		if( szParam && szParam[0] )
		{
			char* sz = (char*)alloca( strlen( szParam ) + 1 );
			strcpy( sz, szParam );
			for( char* p = sz; p; )
			{
				char* p1 = strchr( p, ' ' );
				if( p1 )
					*p1 = 0;
				if( *p )
				{
					char* p2 = strchr( p, '=' );
					if( p2 )
					{
						*p2 = 0;
						m_mapParams[p] = p2 + 1;
					}
					else
						m_mapParams[p] = "";
				}
				if( !p1 )
					break;
				p = p1 + 1;
			}
		}
		if( m_pRes )
			OnOpenFile();
		Refresh();
	}
	virtual void OnOpenFile() {}
	virtual void Refresh() {}

	const char* GetParam( const char* szKey )
	{
		auto itr = m_mapParams.find( szKey );
		if( itr == m_mapParams.end() )
			return NULL;
		return itr->second.c_str();
	}
protected:
	virtual void OnInited() override
	{
		CreateViewport();
		m_onViewportStartDrag.Set( this, &TResourceEditor::OnViewportStartDrag );
		m_pViewport->Register( eEvent_StartDrag, &m_onViewportStartDrag );
		m_onViewportDragged.Set( this, &TResourceEditor::OnViewportDragged );
		m_pViewport->Register( eEvent_Dragged, &m_onViewportDragged );
		m_onViewportStopDrag.Set( this, &TResourceEditor::OnViewportStopDrag );
		m_pViewport->Register( eEvent_StopDrag, &m_onViewportStopDrag );
		m_onViewportMouseUp.Set( this, &TResourceEditor::OnViewportMouseUp );
		m_pViewport->Register( eEvent_MouseUp, &m_onViewportMouseUp );
		m_onViewportMouseWheel.Set( this, &TResourceEditor::OnViewportMouseWheel );
		m_pViewport->Register( eEvent_MouseWheel, &m_onViewportMouseWheel );
		m_onDebugDraw.Set( this, &TResourceEditor::OnDebugDraw );
		m_pViewport->Register( eEvent_Action, &m_onDebugDraw );
		m_onViewportKey.Set( this, &TResourceEditor::OnViewportKey );
		m_pViewport->Register( eEvent_Key, &m_onViewportKey );
		m_onViewportChar.Set( this, &TResourceEditor::OnViewportChar );
		m_pViewport->Register( eEvent_Char, &m_onViewportChar );
		
		m_camOfs = CVector2( 0, 0 );
		OnInitViewport();
	}

	virtual void CreateViewport()
	{
		m_pViewport = GetChildByName<CUIViewport>( "viewport" );
		m_pViewport->SetLight( IsLighted() );
	}

	virtual void OnInitViewport()
	{
		if( m_pViewport->GetRoot() )
		{
			CDirectionalLightObject* pDirectionalLight = new CDirectionalLightObject( CVector2( 0.6, -0.8 ), CVector3( 1, 1, 1 ), 8, 256 );
			m_pViewport->GetRoot()->AddChild( pDirectionalLight );
			m_pLight = pDirectionalLight;

			static CDefaultDrawable2D* pDrawable = NULL;
			static CDefaultDrawable2D* pDrawable1 = NULL;
			if( !pDrawable )
			{
				vector<char> content;
				GetFileContent( content, "EditorRes/Drawables/background.xml", true );
				TiXmlDocument doc;
				doc.LoadFromBuffer( &content[0] );
				pDrawable = new CDefaultDrawable2D;
				pDrawable->LoadXml( doc.RootElement()->FirstChildElement( "color_pass" ) );
				pDrawable1 = new CDefaultDrawable2D;
				pDrawable1->LoadXml( doc.RootElement()->FirstChildElement( "occlusion_pass" ) );
			}
			m_pBackground = new CRenderObject2D;
			m_pViewport->GetRoot()->AddChild( m_pBackground );
			for( int i = 0; i < 4; i++ )
			{
				for( int j = 0; j < 4; j++ )
				{
					CImage2D* pImage = new CImage2D( pDrawable, pDrawable1, CRectangle( -2048 + i * 1024, -2048 + j * 1024, 1024, 1024 ), CRectangle( 0, 0, 1, 1 ) );
					m_pBackground->AddChild( pImage );
				}
			}
		}
	}

	virtual void OnSetVisible( bool bVisible ) override
	{
		if( bVisible )
			Refresh();
		else
			SetFileName( "" );
	}
	
	void SetCamOfs( CVector2& ofs )
	{
		auto& cam = m_pViewport->GetCamera();
		cam.SetPosition( ofs.x, ofs.y );
		if( m_pLight )
			m_pLight->SetPosition( ofs );

		float fGrid = 1024;
		if( m_pBackground )
		{
			CVector2 backgroundPos( floor( ofs.x / fGrid + 0.5f ) * fGrid, floor( ofs.y / fGrid + 0.5f ) * fGrid );
			m_pBackground->SetPosition( backgroundPos );
		}
		m_camOfs = ofs;
	}

	virtual void Save()
	{
		RefreshPreview();
		CBufFile buf;
		m_pRes->Save( buf );
		SaveFile( m_pRes->GetName(), buf.GetBuffer(), buf.GetBufLen() );
	}

	virtual void RefreshPreview() {}
	virtual void OnDebugDraw( IRenderSystem* pRenderSystem ) {}
	CReference<T> m_pRes;
	CReference<CUIViewport> m_pViewport;

	virtual void OnViewportStartDrag( SUIMouseEvent* pEvent )
	{
		CVector2 fixOfs = pEvent->mousePos;
		m_startDragPos = fixOfs;
	}

	virtual void OnViewportDragged( SUIMouseEvent* pEvent )
	{
		CVector2 fixOfs = pEvent->mousePos;
		CVector2 dPos = m_pViewport->GetScenePos( fixOfs ) - m_pViewport->GetScenePos( m_startDragPos );
		m_startDragPos = fixOfs;
		SetCamOfs( m_camOfs - dPos );
	}

	virtual void OnViewportStopDrag( SUIMouseEvent* pEvent ) {}
	void OnViewportMouseUp( SUIMouseEvent* pEvent )
	{
		auto pDragDropObj = GetMgr()->GetDragDropObject();
		if( pDragDropObj )
			OnViewportDrop( pEvent->mousePos, pDragDropObj );
	}
	virtual void OnViewportMouseWheel( SUIMouseEvent* pEvent ) {}
	virtual void OnViewportDrop( const CVector2& mousePos, CUIElement* pParam ) {}
	virtual void OnViewportKey( SUIKeyEvent* pEvent ) {}
	virtual void OnViewportChar( uint32 nChar ) {}
	
	CVector2 m_startDragPos;
	CVector2 m_camOfs;
	map<string, string> m_mapParams;
private:
	CReference<CRenderObject2D> m_pLight;
	CReference<CRenderObject2D> m_pBackground;
	TClassTrigger1<TResourceEditor, SUIMouseEvent*> m_onViewportStartDrag;
	TClassTrigger1<TResourceEditor, SUIMouseEvent*> m_onViewportDragged;
	TClassTrigger1<TResourceEditor, SUIMouseEvent*> m_onViewportStopDrag;
	TClassTrigger1<TResourceEditor, SUIMouseEvent*> m_onViewportMouseUp;
	TClassTrigger1<TResourceEditor, SUIMouseEvent*> m_onViewportMouseWheel;
	TClassTrigger1<TResourceEditor, IRenderSystem*> m_onDebugDraw;
	TClassTrigger1<TResourceEditor, SUIKeyEvent*> m_onViewportKey;
	TClassTrigger1<TResourceEditor, uint32> m_onViewportChar;
};