#include "stdafx.h"
#include "Menu.h"
#include "MyGame.h"
#include "Common/FileUtil.h"
#include "Common/Rand.h"

void CStartMenu::Init()
{
	SetRenderObject( NULL );
	const char* szMenuText[] = { "CONTINUE", "NEW GAME", "QUIT" };
	for( int i = 0; i < ELEM_COUNT( m_pMenuItem ); i++ )
	{
		bool bValid = true;
		if( i == 0 )
		{
			if( !IsFileExist( "save/a" ) )
				bValid = false;
		}
		if( bValid )
		{
			m_pMenuItem[m_vecValidMenuItem.size()]->Set( szMenuText[i] );
			m_vecValidMenuItem.push_back( i );
		}
	}
	for( int i = m_vecValidMenuItem.size(); i < ELEM_COUNT( m_pMenuItem ); i++ )
		m_pMenuItem[i]->bVisible = false;
	for( int i = 0; i < m_vecValidMenuItem.size(); i++ )
		m_pMenuItem[i]->SetParam( i == m_nCurSelectedItem ? m_textSelected : m_textUnSelected );

	auto pLevel = SafeCast<CMyLevel>( m_pBackLevelPrefab->GetRoot()->CreateInstance() );
	m_pBackLevel = pLevel;
	pLevel->SetZOrder( -1 );
	pLevel->SetParentEntity( this );
	pLevel->SetPosition( pLevel->GetCamPos() * -1 );
	pLevel->Init();
	pLevel->Begin();
}

int8 CStartMenu::Update()
{
	if( CGame::Inst().IsInputDown( eInput_Up ) )
	{
		m_nCurSelectedItem--;
		if( m_nCurSelectedItem < 0 )
			m_nCurSelectedItem = m_vecValidMenuItem.size() - 1;
	}
	if( CGame::Inst().IsInputDown( eInput_Down ) )
	{
		m_nCurSelectedItem++;
		if( m_nCurSelectedItem >= m_vecValidMenuItem.size() )
			m_nCurSelectedItem = 0;
	}
	if( CGame::Inst().IsKeyDown( VK_RETURN ) || CGame::Inst().IsKeyDown( ' ' ) )
	{
		int8 nResult = OnSelect();
		if( nResult )
		{
			m_pBackLevel->End();
			return nResult;
		}
	}
	for( int i = 0; i < m_vecValidMenuItem.size(); i++ )
		m_pMenuItem[i]->SetParam( i == m_nCurSelectedItem ? m_textSelected : m_textUnSelected );

	m_pBackLevel->Update();
	UpdateEffect();
	return 0;
}

void CStartMenu::Render( CRenderContext2D & context )
{
	auto pDrawableGroup = static_cast<CDrawableGroup*>( GetResource() );
	auto pColorDrawable = pDrawableGroup->GetColorDrawable();
	auto pOcclusionDrawable = pDrawableGroup->GetOcclusionDrawable();
	auto pGUIDrawable = pDrawableGroup->GetGUIDrawable();

	uint32 nPass = -1;
	uint32 nGroup = 0;
	switch( context.eRenderPass )
	{
	case eRenderPass_Color:
		if( pColorDrawable )
			nPass = 0;
		else if( pGUIDrawable )
		{
			nPass = 2;
			nGroup = 1;
		}
		break;
	case eRenderPass_Occlusion:
		if( pOcclusionDrawable )
			nPass = 1;
		break;
	}
	if( nPass == -1 )
		return;
	CDrawable2D* pDrawables[3] = { pColorDrawable, pOcclusionDrawable, pGUIDrawable };

	for( auto& elem : m_vecElems )
	{
		elem.worldMat = globalTransform;
		elem.SetDrawable( pDrawables[nPass] );
		context.AddElement( &elem, nGroup );
	}
}

int8 CStartMenu::OnSelect()
{
	if( m_vecValidMenuItem[m_nCurSelectedItem] == 0 )
		return eStartMenuResult_Continue;
	else if( m_vecValidMenuItem[m_nCurSelectedItem] == 1 )
		return eStartMenuResult_NewGame;
	return eStartMenuResult_Quit;
}

void CStartMenu::UpdateEffect()
{
	m_vecElems.resize( 0 );
	m_vecElemParams.resize( 0 );
	auto camSize = GetStage()->GetCamera().GetViewArea().GetSize();
	CRectangle r0( -camSize.x * 0.5f, m_pMenuItem[m_nCurSelectedItem]->y, camSize.x, 28 );

	static SRand rand0_1 = SRand::Inst<eRand_Render>();
	SRand rand1 = rand0_1;
	if( !SRand::Inst<eRand_Render>().Rand( 0, 8 ) )
		rand0_1.Rand();
	for( int n = rand1.Rand( -6, 4 ); n > 0; n-- )
	{
		CRectangle rect = r0;
		rect.width = rand1.Rand( 20, 70 ) * 4;
		rect.x = rand1.Rand<int32>( r0.x / 2, ( r0.GetRight() - rect.width ) / 2 + 1 ) * 2;
		rect.y += rand1.Rand( 0, 4 ) * 2;
		int32 nOfs0 = rand1.Rand( 0, 4 );
		m_vecElems.resize( m_vecElems.size() + 1 );
		auto& elem = m_vecElems.back();
		elem.rect = rect;
		elem.texRect = CRectangle( 0, 0.25f, 0.0625f, 0.0625f );

		CVector3 color( rand1.Rand( 0.0f, 1.0f ), rand1.Rand( 0.0f, 1.0f ), rand1.Rand( 0.0f, 1.0f ) );
		float f = Min( color.x, Min( color.y, color.z ) );
		color = ( color - CVector3( f, f, f ) ) * rand1.Rand( 1.0f, 1.3f );
		m_vecElemParams.push_back( CVector4( color.x, color.y, color.z, nOfs0 * 2 ) );
		m_vecElemParams.push_back( CVector4( color.x, color.y, color.z, 0 ) * rand1.Rand( 0.01f, 0.2f ) );
	}

	static SRand rand0_2 = SRand::Inst<eRand_Render>();
	rand1 = rand0_2;
	if( !SRand::Inst<eRand_Render>().Rand( 0, 16 ) )
		rand0_2.Rand();
	int8 nType = rand1.Rand( -4, 3 );
	if( nType == 0 || nType == 1 )
	{
		CRectangle rect = r0;
		rect.height += rand1.Rand( 100, 200 ) * 2;
		rect.SetTop( rand1.Rand<int32>( r0.y / 2, r0.GetBottom() / 2 ) * 2 );
		m_vecElems.resize( m_vecElems.size() + 1 );
		auto& elem = m_vecElems.back();
		elem.rect = rect;
		elem.texRect = CRectangle( 0, 0.25f, 0.0625f, 0.0625f );

		CVector3 color( rand1.Rand( 0.0f, 0.025f ), rand1.Rand( 0.0f, 0.025f ), rand1.Rand( 0.0f, 0.025f ) );
		m_vecElemParams.push_back( CVector4( 1, 1, 1, rand1.Rand( -3, 4 ) * 2 ) );
		m_vecElemParams.push_back( CVector4( -color.x, -color.y, -color.z, rand1.Rand( -3, 4 ) * 2 ) );
	}
	else if( nType == 2 )
	{
		CRectangle rect = r0;
		rect.height += rand1.Rand( 100, 200 ) * 2;
		rect.SetTop( rand1.Rand<int32>( r0.y / 2, r0.GetBottom() / 2 ) * 2 );
		rect.x = rand1.Rand( -16, 64 ) * 2;
		rect.width = rand1.Rand( 18, 56 ) * 2;
		int32 nCount = ( r0.GetRight() - rect.x ) / rect.width;
		int8 nDir = rand1.Rand( 0, 2 );
		if( nDir )
			rect.x = r0.x + r0.GetRight() - rect.GetRight();
		float fOfsY = rand1.Rand( -3, 4 ) * 2;
		CVector3 color( rand1.Rand( 0.0f, 0.07f ), rand1.Rand( 0.0f, 0.07f ), rand1.Rand( 0.0f, 0.07f ) );
		for( int i = 0; i < nCount; i++ )
		{
			m_vecElems.resize( m_vecElems.size() + 1 );
			auto& elem = m_vecElems.back();
			elem.rect = rect;
			elem.texRect = CRectangle( 0, 0.25f, 0.0625f, 0.0625f );
			m_vecElemParams.resize( m_vecElemParams.size() + 2 );
			m_vecElemParams.push_back( CVector4( 1, 1, 1, ( nDir ? rect.width : -rect.width ) * ( i + 1 ) ) );
			auto color1 = color * -i;
			m_vecElemParams.push_back( CVector4( color1.x, color1.y, color1.z, -fOfsY * ( i + 1 ) ) );
			rect.x += nDir ? -rect.width : rect.width;
		}
	}

	CRectangle rect = r0;
	int32 nOfs0 = SRand::Inst<eRand_Render>().Rand( 0, 4 );
	rect.x += nOfs0 * 8;
	m_vecElems.resize( m_vecElems.size() + 1 );
	auto& elem = m_vecElems.back();
	elem.rect = rect;
	elem.texRect = CRectangle( 0, 0.25f, 0.0625f, 0.0625f );
	m_vecElemParams.push_back( CVector4( 0.88f, 0.88f, 0.88f, nOfs0 * 2 ) );
	m_vecElemParams.push_back( CVector4( 0.25f, -0.05f, -0.05f, 0 ) );

	static SRand rand0_3 = SRand::Inst<eRand_Render>();
	rand1 = rand0_3;
	if( !SRand::Inst<eRand_Render>().Rand( 0, 16 ) )
		rand0_3.Rand();
	r0 = CRectangle( -288, -camSize.y * 0.5f, 128, camSize.y );
	for( int i = rand1.Rand( -2, 6 ); i > 0; i-- )
	{
		auto rect = r0;
		rect.width = rand1.Rand( 1, 8 ) * 2;
		rect.x = r0.x + rand1.Rand<int32>( 0, ( r0.width - rect.width ) / 2 ) * 2;
		m_vecElems.resize( m_vecElems.size() + 1 );
		auto& elem = m_vecElems.back();
		elem.rect = rect;
		elem.texRect = CRectangle( 0, 0.25f, 0.0625f, 0.0625f );
		CVector3 color;
		auto n = rand1.Rand( 0, 3 );
		if( n == 0 )
			color = CVector3( rand1.Rand( 0.0f, 0.125f ), rand1.Rand( 0.25f, 0.3f ), rand1.Rand( 0.2f, 0.25f ) );
		else if( n == 1 )
			color = CVector3( rand1.Rand( 0.35f, 0.4f ), rand1.Rand( 0.2f, 0.25f ), rand1.Rand( 0.0f, 0.125f ) );
		else
			color = CVector3( rand1.Rand( 0.3f, 0.35f ), rand1.Rand( 0.0f, 0.125f ), rand1.Rand( 0.15f, 0.2f ) );
		m_vecElemParams.push_back( CVector4( 1 - color.x, 1 - color.y, 1 - color.z, 0 ) );
		m_vecElemParams.push_back( CVector4( color.x, color.y, color.z, 0 ) * rand1.Rand( 0.1f, 0.8f ) );
	}

	for( int i = 0; i < m_vecElems.size(); i++ )
	{
		auto& elem = m_vecElems[i];
		elem.nInstDataSize = sizeof( CVector4 ) * 2;
		elem.pInstData = &m_vecElemParams[i * 2];
	}
}

void RegisterGameClasses_Menu()
{
	REGISTER_CLASS_BEGIN( CStartMenu )
		REGISTER_BASE_CLASS( CEntity )
		REGISTER_MEMBER( m_pBackLevelPrefab )
		REGISTER_MEMBER( m_textSelected )
		REGISTER_MEMBER( m_textUnSelected )
		REGISTER_MEMBER_TAGGED_PTR( m_pMenuItem[0], 0 )
		REGISTER_MEMBER_TAGGED_PTR( m_pMenuItem[1], 1 )
		REGISTER_MEMBER_TAGGED_PTR( m_pMenuItem[2], 2 )
	REGISTER_CLASS_END()
}