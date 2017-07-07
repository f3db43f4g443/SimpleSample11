#include "stdafx.h"
#include "MainMenu.h"
#include "PlayerData.h"
#include "GlobalCfg.h"
#include "MyLevel.h"
#include "GUI/MainUI.h"
#include "Stage.h"

void CMainMenuGenerateNode::Load( TiXmlElement * pXml, SLevelGenerateNodeLoadContext & context )
{
	CLevelGenerateSimpleNode::Load( pXml, context );
	m_pButtonValidNode = CreateNode( pXml->FirstChildElement( "button_valid" )->FirstChildElement(), context );
	m_pButtonInvalidNode = CreateNode( pXml->FirstChildElement( "button_invalid" )->FirstChildElement(), context );
	for( auto pChild = pXml->FirstChildElement( "button_pos" )->FirstChildElement(); pChild; pChild = pChild->NextSiblingElement() )
	{
		TVector2<int32> p;
		p.x = XmlGetAttr( pChild, "x", 0 );
		p.y = XmlGetAttr( pChild, "y", 0 );
		m_vecButtonPos.push_back( p );
	}
}

void CMainMenuGenerateNode::Generate( SLevelBuildContext & context, const TRectangle<int32>& region )
{
	auto pChunk = context.CreateChunk( *m_pChunkBaseInfo, region );
	if( pChunk )
	{
		pChunk->bIsLevelBarrier = m_bIsLevelBarrier;
		pChunk->nBarrierHeight = m_nLevelBarrierHeight;
		CLevelGenerateNode::Generate( context, region );

		if( m_pSubChunk )
		{
			SLevelBuildContext tempContext( context.pLevel, pChunk );
			if( m_pSubChunk )
				m_pSubChunk->Generate( tempContext, TRectangle<int32>( 0, 0, pChunk->nWidth, pChunk->nHeight ) );
			
			if( !CPlayerData::Inst().bIsDesign )
			{
				int32 nPassedLevel = CPlayerData::Inst().nPassedLevels;
				int32 nTotalLevel = CGlobalCfg::Inst().vecLevels.size();
				nTotalLevel = Min<int32>( nTotalLevel, m_vecButtonPos.size() );
				nPassedLevel = Min( nPassedLevel, nTotalLevel - 1 );

				int32 i;
				for( i = 0; i <= nPassedLevel; i++ )
				{
					m_pButtonValidNode->Generate( tempContext, TRectangle<int32>( m_vecButtonPos[i].x, m_vecButtonPos[i].y, 1, 1 ) );
					auto pBlock = tempContext.GetBlock( m_vecButtonPos[i].x, m_vecButtonPos[i].y, 1 );
					pBlock->pParent->pOwner->nDestroyShake = i;
				}
				for( ; i < nTotalLevel; i++ )
				{
					m_pButtonInvalidNode->Generate( tempContext, TRectangle<int32>( m_vecButtonPos[i].x, m_vecButtonPos[i].y, 1, 1 ) );
				}
			}

			tempContext.Build();
		}
	}
}

void CMainMenu::OnCreateComplete( CMyLevel * pLevel )
{
	for( auto pEntity = Get_ChildEntity(); pEntity; pEntity = pEntity->NextChildEntity() )
	{
		auto pChunkObject = SafeCast<CChunkObject>( pEntity );
		if( pChunkObject )
		{
			if( pChunkObject->GetName() == m_strButton.c_str() )
				m_vecButtons.push_back( pChunkObject );
		}
	}

	m_triggers.resize( m_vecButtons.size() );
	int32 iTrigger = 0;
	for( auto pChunkObject : m_vecButtons )
	{
		uint32 nLevel = pChunkObject->GetChunk()->nDestroyShake;

		CFunctionTrigger& trigger = m_triggers[iTrigger++];
		trigger.Set( [this, nLevel] ()
		{
			OnButton( nLevel );
		} );
		pChunkObject->RegisterKilledEvent( &trigger );
	}

	CMainUI::GetInst()->HideMinimap();
}

void CMainMenu::OnButton( int32 i )
{
	CMyLevel::GetInst()->BeginGenLevel( i );
	m_triggers.clear();
	GetStage()->RegisterBeforeHitTest( 1, &m_onKill );
}

void CMainMenu::OnKill()
{
	CMainUI::GetInst()->ShowMinimap();
	Kill();
}
