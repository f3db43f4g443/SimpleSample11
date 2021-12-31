#include "stdafx.h"
#include "CharacterMisc.h"
#include "Render/DrawableGroup.h"
#include "Common/Rand.h"
#include "Bullet.h"
#include "MyLevel.h"
#include "Stage.h"
#include "CharacterMove.h"
#include "MyGame.h"
#include "Common/MathUtil.h"
#include "Entities/UtilEntities.h"
#include "GlobalCfg.h"

bool CCommonMoveableObject::Damage( SDamageContext& context )
{
	bool b = false;
	if( context.nType == eDamageHitType_Kick_Special )
	{
		m_vel = context.hitDir * context.fDamage1 / m_fWeight;
		SetImpactLevel( 0, 0 );
		b = true;
	}
	else if( context.nType == eDamageHitType_Kick_Begin )
	{
		m_nKickCounter++;
		m_vel = context.hitDir / m_fWeight;
		SetImpactLevel( 0, 0 );
		b = true;
	}
	else if( context.nType >= eDamageHitType_Kick_End && context.nType <= eDamageHitType_Kick_End_4 )
	{
		if( m_nKickCounter )
		{
			m_nKickCounter--;
			if( !m_nKickCounter )
			{
				auto nImpactLevel = context.nType - eDamageHitType_Kick_End;
				if( nImpactLevel > m_nBlockImpactLevel )
				{
					m_vel = context.hitDir / m_fWeight;
					SetImpactLevel( nImpactLevel, context.fDamage1 );
				}
			}
		}
		b = true;
	}
	b = CCharacter::Damage( context ) || b;
	return b;
}

void CCommonMoveableObject::OnTickBeforeHitTest()
{
	CCharacter::OnTickBeforeHitTest();
	if( m_nImpactTick )
	{
		m_nImpactTick--;
		if( !m_nImpactTick )
		{
			SetImpactLevel( 0, 0 );
			m_vel = m_vel * 0.5f;
		}
	}
}

void CCommonMoveableObject::OnTickAfterHitTest()
{
	CEntity* p = this;
	SIgnoreEntityHit ignoreHit( &p, 1 );
	CVector2 gravity( 0, -1 );
	CVector2 gravity0( 0, 0 );
	auto pGravity = CanHitPlatform() ? &gravity : &gravity0;
	if( !m_moveData.ResolvePenetration( this, &m_vel, m_fFrac, NULL, NULL, pGravity ) )
	{
		//Crush();
		//return;
	}
	auto dPos = HandleCommonMove();
	CEntity* pTested = this;
	m_moveData.TryMove1XY( this, 1, &pTested, dPos, m_vel, m_fFrac, pGravity );
	PostMove();
}

bool CCommonMoveableObject::ImpactHit( int32 nLevel, const CVector2& vec, CEntity* pEntity )
{
	if( m_nKickCounter || !m_nImpactHitTime )
		return false;
	if( nLevel > m_nBlockImpactLevel )
	{
		auto pCharacter = SafeCast<CCharacter>( pEntity );
		auto fWeight = pCharacter ? pCharacter->GetWeight() : 0;
		m_vel = fWeight > 0 && m_fWeight > 0 ? vec * fWeight / m_fWeight : vec;
		SetImpactLevel( nLevel, m_nImpactHitTime );
		return true;
	}
	return false;
}

bool CCommonMoveableObject::CheckImpact( CEntity* pEntity, SRaycastResult& result, bool bCast )
{
	auto p1 = SafeCast<CCharacter>( pEntity );
	if( p1 && p1->GetKillImpactLevel() && m_nBlockImpactLevel >= 0 && m_nImpactLevel >= p1->GetKillImpactLevel() )
	{
		auto norm = result.normal;
		if( bCast )
			norm = norm * -1;
		auto d = norm.Dot( m_vel );
		if( d > m_fImpactSpeed0 )
			//p1->Kill();
			return false;
	}
	return __super::CheckImpact( pEntity, result, bCast );
}

int8 CCommonMoveableObject::CheckPush( SRaycastResult& hit, const CVector2& dir, float& fDist, SPush& context, int32 nPusher )
{
	if( hit.nUser )
	{
		auto pPusher = context.vecChars[nPusher].pChar;
		if( m_moveData.setOpenPlatforms.find( pPusher ) != m_moveData.setOpenPlatforms.end() )
			return -1;
		float f = hit.normal.Dot( CanHitPlatform() ? CVector2( 0, -1 ) : CVector2( 0, 0 ) );
		if( f < PLATFORM_THRESHOLD )
		{
			m_moveData.setOpenPlatforms.insert( pPusher );
			return -1;
		}
	}
	CEntity* pTested = this;
	auto mat = globalTransform;
	fDist = GetLevel()->Push( this, context, dir, fDist, 1, &pTested, &mat, MOVE_SIDE_THRESHOLD );
	return 1;
}

void CCommonMoveableObject::HandlePush( const CVector2& dir, float fDist, int8 nStep )
{
	if( nStep == 0 )
	{
		if( fDist > 0 )
		{
			SetPosition( GetPosition() + dir * fDist );
			SetDirty();
		}
	}
	else if( nStep == 1 )
	{
		if( fDist > 0 )
			GetLevel()->GetHitTestMgr().Update( this );
	}
	else
	{
		CEntity* pTested = this;
		m_moveData.CleanUpOpenPlatforms( this, 1, &pTested );
	}
}

bool CCommonMoveableObject::CanHitPlatform()
{
	return !( m_nKickCounter && m_vel.y < -1.0f );
}

void CCommonMoveableObject::SetImpactLevel( int32 nLevel, int32 nTick )
{
	if( !nLevel && m_nImpactLevel )
		Trigger( eCharacterEvent_ImpactLevelEnd );
	m_nImpactLevel = nLevel;
	m_nImpactTick = nTick;
	if( nLevel )
		Trigger( eCharacterEvent_ImpactLevelBegin );
	if( m_nImpactTick )
	{
		if( m_nUpdatePhase >= 2 )
			m_nImpactTick++;
	}
	if( m_nBlockImpactLevel >= 0 )
	{
		if( m_nImpactLevel )
			m_fImpactSpeed0 = m_vel.Length() * 0.7f;
		auto p = SafeCastToInterface<IImageEffectTarget>( GetRenderObject() );
		if( p )
		{
			if( m_nImpactLevel )
				p->SetCommonEffectEnabled( eImageCommonEffect_Phantom, true, CGlobalCfg::Inst().vecAttackLevelColor[m_nImpactLevel - 1] );
			else
				p->SetCommonEffectEnabled( eImageCommonEffect_Phantom, false, CVector4( 0, 0, 0, 0 ) );
		}
	}
}

CVector2 CCommonMoveableObject::HandleCommonMove()
{
	float fDeltaTime = GetLevel()->GetElapsedTimePerTick();
	CVector2 gravityDir( 0, -1 );
	CVector2 dPos = m_vel * fDeltaTime;
	if( m_nKickCounter )
	{
		float l = m_vel.Length();
		if( l > 0 && m_fAirborneFrac > 0 )
		{
			float d = fDeltaTime * m_fAirborneFrac;
			float t0 = Min( fDeltaTime, l / m_fAirborneFrac );

			auto vel1 = m_vel * ( Max( 0.0f, l - d ) / Max( l, d ) );
			dPos = vel1 * fDeltaTime + ( m_vel - vel1 ) * ( t0 * t0 * 0.5f / fDeltaTime );
			m_vel = vel1;
		}
	}
	else if( /*m_nBlockImpactLevel == -1 ||*/ !m_nImpactLevel )
	{
		float fNormalVelocity = m_vel.Dot( gravityDir );
		float fDeltaSpeed = Max( 0.0f, m_fMaxFallSpeed - fNormalVelocity );
		float t0 = Min( fDeltaTime, fDeltaSpeed / m_fGravity );
		auto dVelocity = gravityDir * ( m_fGravity * t0 );
		dPos = dPos + gravityDir * ( m_fGravity * t0 * ( fDeltaTime - t0 * 0.5f ) );
		m_vel = m_vel + dVelocity;
	}
	return dPos;
}

void CCommonMoveableObject::PostMove()
{
	CEntity* pTested = this;
	PostMove( 1, &pTested );
}

void CCommonMoveableObject::PostMove( int32 nTestEntities, CEntity** pTestEntities )
{
	bool bStopImpact = false;
	for( int i = 0; i < nTestEntities; i++ )
	{
		GetLevel()->GetHitTestMgr().Update( pTestEntities[i] );
		if( m_nBlockImpactLevel >= 0 && m_nImpactLevel )
		{
			for( auto pManifold = pTestEntities[i]->Get_Manifold(); pManifold; pManifold = pManifold->NextManifold() )
			{
				if( pManifold->normal.Length2() < MOVE_SIDE_THRESHOLD * MOVE_SIDE_THRESHOLD )
					continue;
				auto p = static_cast<CEntity*>( pManifold->pOtherHitProxy );
				auto pCharacter = SafeCast<CCharacter>( p );
				if( pCharacter )
				{
					if( pCharacter->GetKillImpactLevel() && m_nImpactLevel >= pCharacter->GetKillImpactLevel() )
					{
						if( m_nImpactLevel == pCharacter->GetKillImpactLevel() )
							bStopImpact = true;
						pCharacter->Kill();
					}
					else if( !pCharacter->GetHitChannnel()[GetHitType()] )
						pCharacter->ImpactHit( m_nImpactLevel, m_vel, this );
				}
			}
		}
	}
	if( bStopImpact )
		SetImpactLevel( 0, 0 );
	m_moveData.CleanUpOpenPlatforms( this, nTestEntities, pTestEntities );
}


bool CCommonGrabbable::CheckGrab( class CPlayer* pPlayer, SGrabDesc& desc )
{
	if( m_nGrabDir && pPlayer->GetDir() != m_nGrabDir )
		return false;
	desc.grabOfs = m_grabOfs;
	desc.fDropThreshold = m_fDropThreshold;
	desc.nGrabDir = m_nGrabDir;
	desc.nDetachType = m_nDetachType;
	return true;
}

void CCommonGrabbable::OnAttached( CPlayer* pPlayer )
{
	m_bAttached = true;
}

void CCommonGrabbable::OnDetached( CPlayer* pPlayer )
{
	m_bAttached = false;
}

void CLever::OnAddedToStage()
{
	CCommonGrabbable::OnAddedToStage();
	if( !m_bInited )
	{
		m_bInited = true;
		m_state0 = CVector3( x, y, r );
	}
}

void CLever::OnTickBeforeHitTest()
{
	CCommonGrabbable::OnTickBeforeHitTest();
	if( m_nNxtState != m_nCurState )
	{
		auto targetState = m_nNxtState ? m_state0 + m_arrState[m_nNxtState - 1] : m_state0;
		m_nStateTransferTime--;
		if( !m_nStateTransferTime )
		{
			SetPosition( CVector2( targetState.x, targetState.y ) );
			SetRotation( targetState.z );
			m_nCurState = m_nNxtState;
		}
		else
		{
			auto curState = CVector3( x, y, r );
			curState = curState + ( targetState - curState ) * ( 1.0f / ( m_nStateTransferTime + 1 ) );
			SetPosition( CVector2( curState.x, curState.y ) );
			SetRotation( curState.z );
		}
	}
}

bool CLever::CheckGrab( class CPlayer* pPlayer, SGrabDesc& desc )
{
	if( !CCommonGrabbable::CheckGrab( pPlayer, desc ) )
		return false;

	if( desc.bAttached )
	{
		if( m_nCurState == m_nNxtState && CGame::Inst().IsKey( 'J' ) )
		{
			for( int i = 0; i < m_arrTransfer.Size(); i++ )
			{
				auto& transfer = m_arrTransfer[i];
				if( transfer.x != m_nCurState )
					continue;
				int32 nKey = 0;
				if( transfer.w == 0 )
					nKey = 'D';
				else if( transfer.w == 1 )
					nKey = 'W';
				else if( transfer.w == 2 )
					nKey = 'A';
				else if( transfer.w == 3 )
					nKey = 'S';
				if( !CGame::Inst().IsKey( nKey ) )
					continue;
				m_nNxtState = transfer.y;
				m_nStateTransferTime = transfer.z;
				break;
			}
		}
		if( m_nCurState != m_nNxtState )
			desc.nDetachType = 0;
	}
	return true;
}

void CChunk::OnAddedToStage()
{
	Init();
	CCharacter::OnAddedToStage();
}

void CChunk::Render( CRenderContext2D & context )
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

void CChunk::Init()
{
	if( m_bInited )
		return;
	m_bInited = true;
	m_nHp += m_fHpPerTile * ( m_nTileX * m_nTileY - 1 );
	if( !m_bNoHit )
		AddRect( CRectangle( m_ofs.x + m_hitSize.x, m_ofs.y + m_hitSize.y, m_tileSize.x * m_nTileX + m_hitSize.width, m_tileSize.y * m_nTileY + m_hitSize.height ) );
	UpdateImages();
}

void CChunk::UpdateImages()
{
	if( !GetRenderObject() )
		return;
	auto pImage = static_cast<CImage2D*>( GetRenderObject() );
	auto tex = pImage->GetElem().texRect;
	m_origTexRect = CRectangle( tex.x, tex.y, tex.width / m_nTexX, tex.height / m_nTexY );
	m_nParamCount = Min<int32>( 2, pImage->GetParamCount() );
	if( m_nParamCount )
	{
		for( int i = 0; i < m_nParamCount; i++ )
			m_params[i] = pImage->GetParam()[0];
	}
	SetRenderObject( NULL );
	auto pDrawable = static_cast<CDrawableGroup*>( GetResource() );
	if( !pDrawable )
		return;

	auto size = m_tileSize * CVector2( m_nTileX, m_nTileY );
	auto ofs = m_ofs;
	SetLocalBound( CRectangle( ofs.x, ofs.y, size.x, size.y ) );

	m_vecElems.resize( m_nTileX * m_nTileY );
	int32 iElem = 0;
	int32 nTexX1, nTexY1;
	if( m_nTex1Type )
	{
		nTexX1 = SRand::Inst<eRand_Render>().Rand( 0, m_nTexX1 );
		nTexY1 = SRand::Inst<eRand_Render>().Rand( 0, m_nTexY1 );
	}
	for( int i = 0; i < m_nTileX; i++ )
	{
		for( int j = 0; j < m_nTileY; j++ )
		{
			auto& elem = m_vecElems[iElem++];
			auto p = static_cast<CImage2D*>( pDrawable->CreateInstance() );
			elem.rect = CRectangle( m_tileSize.x * i + m_ofs.x, m_tileSize.y * j + m_ofs.y, m_tileSize.x, m_tileSize.y );
			int32 tX, tY;
			if( m_nTypeX == 0 )
				tX = SRand::Inst<eRand_Render>().Rand( 0, m_nTexX );
			else
			{
				if( i == 0 )
					tX = 0;
				else if( i == m_nTileX - 1 )
					tX = m_nTexX - 1;
				else
					tX = SRand::Inst<eRand_Render>().Rand( 1, m_nTexX - 1 );
			}
			if( m_nTypeY == 0 )
				tY = SRand::Inst<eRand_Render>().Rand( 0, m_nTexY );
			else
			{
				if( j == 0 )
					tY = m_nTexY - 1;
				else if( j == m_nTileY - 1 )
					tY = 0;
				else
					tY = SRand::Inst<eRand_Render>().Rand( 1, m_nTexY - 1 );
			}
			if( !m_nTex1Type )
			{
				nTexX1 = SRand::Inst<eRand_Render>().Rand( 0, m_nTexX1 );
				nTexY1 = SRand::Inst<eRand_Render>().Rand( 0, m_nTexY1 );
			}
			tX += nTexX1 * m_nTexX;
			tY += nTexY1 * m_nTexY;

			auto texRect = m_origTexRect;
			texRect.x += texRect.width * tX;
			texRect.y += texRect.height * tY;
			elem.texRect = texRect;
			elem.nInstDataSize = m_nParamCount * sizeof( CVector4 );
			if( m_nParamCount )
				elem.pInstData = m_params;
		}
	}
}

bool CChunkPortal::CheckTeleport( CPlayer* pPlayer )
{
	auto pPlayerHits = pPlayer->GetAllHits();
	static vector<CHitProxy*> vecResult;
	for( int i = 0; i < 3; i++ )
	{
		auto mat = pPlayerHits[i]->GetGlobalTransform();
		vecResult.resize( 0 );
		auto pHitProxy = Get_HitProxy();
		auto pHitProxy1 = pPlayerHits[i]->Get_HitProxy();
		if( !SHitProxy::Contain( pHitProxy, pHitProxy1, GetGlobalTransform(), mat ) )
			return false;
	}
	return true;
}

void CChunkPortal::OnTickAfterHitTest()
{
	auto pPlayer = GetLevel()->GetPlayer();
	if( pPlayer )
	{
		if( CMasterLevel::GetInst()->GetTestState() || CMasterLevel::GetInst()->IsAlert() )
			m_params[0] = CVector4( 0, 0, 0, 0 );
		else if( CheckTeleport( pPlayer ) )
		{
			if( CGame::Inst().IsKey( ' ' ) )
				CMasterLevel::GetInst()->TryTeleport( m_bUp );
			if( CMasterLevel::GetInst()->CheckTeleport( m_bUp ) )
				m_params[0] = m_colorValid;
			else
				m_params[0] = m_colorInvalid;
		}
		else
			m_params[0] = m_colorDefault;
	}
}

void CChunk1::OnAddedToStage()
{
	Init();
	CCharacter::OnAddedToStage();
}

void CChunk1::Render( CRenderContext2D & context )
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

void CChunk1::Resize( const TRectangle<int32>& size )
{
	m_ofs = m_ofs + CVector2( size.x, size.y ) * m_tileSize;
	m_nTileX = size.width;
	m_nTileY = size.height;
	m_arrCols.Resize( m_nTileX );
	m_arrRows.Resize( m_nTileY );
	m_arrData.Resize( m_nTileX * m_nTileY * 2 );
	auto n = m_nEditGroup;
	m_nEditGroup = -1;
	SetEditGroup( n );
}

void CChunk1::SetEditGroup( int32 n )
{
	if( n == m_nEditGroup )
		return;
	m_nEditGroup = n;
	auto& editGroup = m_arrEditGroup[n];
	for( int i = 0; i < m_nTileX; i++ )
	{
		if( editGroup.nAutoTypeCol )
		{
			if( i == 0 )
				m_arrCols[i] = 0;
			else if( i == m_nTileX - 1 )
				m_arrCols[i] = editGroup.nTexX - 1;
			else
				m_arrCols[i] = Min( editGroup.nTexX - 1, 1 );
		}
		else if( editGroup.nAutoTypeCol == 2 )
			m_arrCols[i] = 0;
		else
			m_arrCols[i] = -1;
	}
	for( int i = 0; i < m_nTileY; i++ )
	{
		if( editGroup.nAutoTypeRow == 1 )
		{
			if( i == 0 )
				m_arrRows[i] = editGroup.nTexY - 1;
			else if( i == m_nTileY - 1 )
				m_arrRows[i] = 0;
			else
				m_arrRows[i] = Min( editGroup.nTexY - 1, 1 );
		}
		else if( editGroup.nAutoTypeRow == 2 )
			m_arrRows[i] = 0;
		else
			m_arrRows[i] = -1;
	}
	for( int i = 0; i < m_nTileX; i++ )
	{
		auto nCol = m_arrCols[i];
		for( int j = 0; j < m_nTileY; j++ )
		{
			auto nRow = m_arrRows[j];
			m_arrData[( i + j * m_nTileX ) * 2] = nCol >= 0 ? nCol : SRand::Inst().Rand( 0, editGroup.nTexX - 1 );
			m_arrData[( i + j * m_nTileX ) * 2 + 1] = nRow >= 0 ? nRow : SRand::Inst().Rand( 0, editGroup.nTexY - 1 );
		}
	}
}

void CChunk1::SetColType( int32 x, int32 nCol )
{
	m_arrCols[x] = nCol;
	auto& editGroup = m_arrEditGroup[m_nEditGroup];
	for( int y = 0; y < m_nTileY; y++ )
		m_arrData[( x + y * m_nTileX ) * 2] = nCol >= 0 ? nCol : SRand::Inst().Rand( 0, editGroup.nTexX - 1 );
}

void CChunk1::SetRowType( int32 y, int32 nRow )
{
	m_arrRows[y] = nRow;
	auto& editGroup = m_arrEditGroup[m_nEditGroup];
	for( int x = 0; x < m_nTileX; x++ )
		m_arrData[( x + y * m_nTileX ) * 2 + 1] = nRow >= 0 ? nRow : SRand::Inst().Rand( 0, editGroup.nTexY - 1 );
}

void CChunk1::Init()
{
	if( m_bInited )
		return;
	m_bInited = true;
	m_nHp += m_fHpPerTile * ( m_nTileX * m_nTileY - 1 );
	if( !m_bNoHit )
		AddRect( CRectangle( m_ofs.x + m_hitSize.x, m_ofs.y + m_hitSize.y, m_tileSize.x * m_nTileX + m_hitSize.width, m_tileSize.y * m_nTileY + m_hitSize.height ) );
	UpdateImages();
}

void CChunk1::UpdateImages()
{
	if( !GetRenderObject() )
		return;
	if( !m_arrData.Size() )
		Resize( TRectangle<int32>( 0, 0, m_nTileX, m_nTileY ) );
	auto tex = static_cast<CImage2D*>( GetRenderObject() )->GetElem().texRect;
	SetRenderObject( NULL );
	auto pDrawable = static_cast<CDrawableGroup*>( GetResource() );
	if( !pDrawable )
		return;

	auto size = m_tileSize * CVector2( m_nTileX, m_nTileY );
	auto ofs = m_ofs;
	SetLocalBound( CRectangle( ofs.x, ofs.y, size.x, size.y ) );

	m_vecElems.resize( m_nTileX * m_nTileY );
	int32 iElem = 0;
	auto texRect0 = m_arrEditGroup[m_nEditGroup].texRect;
	texRect0.width /= m_arrEditGroup[m_nEditGroup].nTexX;
	texRect0.height /= m_arrEditGroup[m_nEditGroup].nTexY;
	for( int i = 0; i < m_nTileX; i++ )
	{
		for( int j = 0; j < m_nTileY; j++ )
		{
			auto& elem = m_vecElems[iElem++];
			auto p = static_cast<CImage2D*>( pDrawable->CreateInstance() );
			elem.rect = CRectangle( m_tileSize.x * i + m_ofs.x, m_tileSize.y * j + m_ofs.y, m_tileSize.x, m_tileSize.y );
			int32 tX = m_arrData[( i + j * m_nTileX ) * 2];
			int32 tY = m_arrData[( i + j * m_nTileX ) * 2 + 1];

			auto texRect = texRect0;
			texRect.x += texRect.width * tX;
			texRect.y += texRect.height * tY;
			elem.texRect = texRect;
		}
	}
}

bool CAlertTrigger::IsTriggered()
{
	auto pMasterLevel = CMasterLevel::GetInst();
	if( pMasterLevel->GetCurLevel() != GetLevel() )
		return false;
	if( !pMasterLevel->GetTestState() )
		return false;
	if( !pMasterLevel->GetTestRect().Contains( GetGlobalTransform().GetPosition() ) )
		return false;
	return true;
}

void CAlertTrigger::OnTickAfterHitTest()
{
	CCharacter::OnTickAfterHitTest();
	bool bTriggered = IsTriggered();
	if( bTriggered )
	{
		if( !m_bTriggered && !CMasterLevel::GetInst()->IsAlert() )
		{
			if( !m_bDetected )
				m_bDetected = true;
			CMasterLevel::GetInst()->BeginAlert( m_alertRect.Offset( globalTransform.GetPosition() ), m_alertVel );
			m_bTriggered = true;
		}
	}
	else
		m_bTriggered = false;

	bVisible = m_bDetected;
	auto pImage = static_cast<CImage2D*>( GetRenderObject() );
	CVector4& param = pImage->GetParam()[0];
	param = bTriggered ? m_colorTriggered : m_color0;
}

void CCharacterTriggerSpawn::OnAddedToStage()
{
	auto p = CMyLevel::GetEntityCharacterRootInLevel( this );
	p->RegisterCharacterEvent( m_eEvent, &m_onTrigger );
}

void CCharacterTriggerSpawn::OnRemovedFromStage()
{
	if( m_onTrigger.IsRegistered() )
		m_onTrigger.Unregister();
}

void CCharacterTriggerSpawn::Trigger()
{
	auto pOwner = CMyLevel::GetEntityCharacterRootInLevel( this );
	auto trans = GetGlobalTransform();
	auto p = SafeCast<CEntity>( m_pPrefab->GetRoot()->CreateInstance() );

	p->SetPosition( trans.GetPosition() );
	p->SetRotation( atan2( trans.m10, trans.m00 ) );
	auto pCharacter = SafeCast<CCharacter>( p );
	if( pCharacter )
		pCharacter->SetOwner( pOwner );
	p->SetParentBeforeEntity( pOwner );
}

void CTurret::UpdateModule( bool bActivated )
{
	int8 nStaticEft = 0;
	if( bActivated )
	{
		bool bDetect = false;
		if( m_nDetectActivateTime )
			m_nDetectActivateTime--;
		else
			bDetect = Detect();

		if( m_nActivateTimeLeft )
		{
			nStaticEft = 2;
			m_nActivateTimeLeft--;
		}
		else
		{
			if( m_nFireCD )
				m_nFireCD--;
			if( !m_nFireCD )
			{
				if( !m_nFireCountLeft && bDetect )
					m_nFireCountLeft = m_nFireCount;
				if( m_nFireCountLeft )
				{
					Fire();
					m_nFireCountLeft--;
					m_nFireCD = m_nFireCountLeft ? m_nFireInterval : m_nReloadTime;
				}
			}
			nStaticEft = 1;
		}
	}
	else
	{
		m_nFireCountLeft = 0;
		m_nFireCD = 0;
		m_nActivateTimeLeft = m_nActivateTime;
		m_nDetectActivateTimeLeft = m_nDetectActivateTime;
	}

	if( nStaticEft != m_nStaticEft )
	{
		if( m_pStaticEft )
		{
			m_pStaticEft->SetParentEntity( NULL );
			m_pStaticEft = NULL;
		}
		m_nStaticEft = nStaticEft;
		if( nStaticEft == 1 )
		{
			if( m_pStaticEftPrefab )
			{
				if( !m_pStaticEft )
				{
					auto p = SafeCast<CCharacter>( m_pStaticEftPrefab->GetRoot()->CreateInstance() );
					p->SetOwner( CMyLevel::GetEntityCharacterRootInLevel( this ) );
					p->SetParentEntity( this );
					m_pStaticEft = p;
				}
			}
		}
		else
		{
			if( m_pStaticEftPreActivePrefab )
			{
				if( !m_pStaticEft )
				{
					auto p = SafeCast<CCharacter>( m_pStaticEftPreActivePrefab->GetRoot()->CreateInstance() );
					p->SetOwner( CMyLevel::GetEntityCharacterRootInLevel( this ) );
					p->SetParentEntity( this );
					m_pStaticEft = p;
				}
			}
		}
	}
	if( GetRenderObject() )
		GetRenderObject()->SetRotation( m_fCurRot );
	if( m_pStaticEft )
	{
		CMatrix2D mat;
		mat.Rotate( m_fCurRot );
		m_pStaticEft->SetPosition( mat.MulVector2Pos( m_staticEftOfs ) );
		m_pStaticEft->SetRotation( m_fCurRot );
	}
}

bool CTurret::Detect()
{
	auto pPlayer = CMasterLevel::GetInst()->GetPlayer();
	auto trans = GetGlobalTransform();
	auto playerPos = trans.MulTVector2Pos( pPlayer->GetPosition() );
	bool bSeePlayer = false;
	float fRotAngle = m_fRotAngle / 180 * PI;
	if( m_nScanType == 1 )
	{
		float dRot = m_fRotSpeed * pPlayer->GetLevel()->GetElapsedTimePerTick();
		if( fRotAngle > 0 )
		{
			m_fCurRot1 += dRot;
			while( m_fCurRot1 >= fRotAngle * 2 )
				m_fCurRot1 -= fRotAngle * 4;
			m_fCurRot = abs( m_fCurRot1 )- fRotAngle;
		}
		else
			m_fCurRot = NormalizeAngle( m_fCurRot + dRot );
	}

	if( m_fSightRange > 0 )
	{
		if( m_fSightRange * m_fSightRange < playerPos.Length2() )
			return false;
	}
	else if( Get_HitProxy() )
	{
		auto pLevel = pPlayer->GetLevel();
		if( !SHitProxy::HitTest( Get_HitProxy(), pPlayer->Get_HitProxy(), trans, pPlayer->GetGlobalTransform() ) )
			return false;
	}
	if( m_fRotSpeed <= 0 && m_fSightRange <= 0 )
		return true;
	float fRot = atan2( playerPos.y, playerPos.x );
	if( m_fRotSpeed > 0 )
	{
		if( !m_nScanType )
		{
			if( fRotAngle > 0 )
			{
				float dRot = m_fRotSpeed * pPlayer->GetLevel()->GetElapsedTimePerTick();
				m_fCurRot = Min( m_fCurRot + dRot, Max( m_fCurRot - dRot, fRot ) );
				m_fCurRot = Max( -fRotAngle, Min( fRotAngle, m_fCurRot ) );
			}
			else
				m_fCurRot = CEntity::CommonTurn1( m_fCurRot, pPlayer->GetLevel()->GetElapsedTimePerTick(), m_fRotSpeed, fRot );
		}
	}
	if( m_fSightRange > 0 )
	{
		auto dRot = NormalizeAngle( m_fCurRot - fRot );
		if( m_fSightAngle >= 0 && abs( dRot ) > m_fSightAngle / 180 * PI )
			return false;
	}
	return true;
}

void CTurret::Fire()
{
	auto trans = GetGlobalTransform();
	CMatrix2D trans1;
	trans1.Rotate( m_fCurRot );
	for( int i = 0; i < m_nBulletCount; i++ )
	{
		float fAngle = m_fCurRot;
		if( m_nBulletCount > 1 )
			fAngle += m_fBulletAngle * ( ( i + ( m_nBulletCount - 1 ) * 0.5f ) / ( m_nBulletCount - 1 ) );
		CVector2 vel0( cos( fAngle ), sin( fAngle ) );
		vel0 = trans.MulVector2Dir( vel0 );
		for( int32 iDesc = 0; iDesc < m_arrBulletDesc.Size(); iDesc++ )
		{
			auto& item = m_arrBulletDesc[iDesc];
			auto p = SafeCast<CCharacter>( item.pPrefab->GetRoot()->CreateInstance() );
			p->SetPosition( trans.MulVector2Pos( trans1.MulVector2Pos( item.ofs ) ) );
			CVector2 vel( vel0.x * item.vel.x - vel0.y * item.vel.y, vel0.x * item.vel.y + vel0.y * item.vel.x );
			p->SetRotation( atan2( vel.y, vel.x ) );
			p->SetVelocity( vel );
			p->SetOwner( CMyLevel::GetEntityCharacterRootInLevel( this ) );
			if( SafeCast<CBullet>( p ) )
				SafeCast<CBullet>( p )->SetBulletVelocity( vel );
			p->SetParentEntity( CMasterLevel::GetInst()->GetCurLevel() );
		}
	}
}

void CGate::OnAddedToStage()
{
	Init();
	CCharacter::OnAddedToStage();
}

void CGate::OnTickBeforeHitTest()
{
	if( m_nTriggerType == 0 )
	{
		auto pMasterLevel = CMasterLevel::GetInst();
		if( pMasterLevel->GetCurLevel() == GetLevel() && pMasterLevel->GetTestState() && pMasterLevel->GetTestRect().Contains( GetGlobalTransform().GetPosition() ) )
			m_nTriggerTimeLeft = m_nTriggerTime;
	}
	else if( m_nTriggerType == 1 )
	{
		if( IsAlerted() )
			m_nTriggerTimeLeft = m_nTriggerTime;
	}

	bool bClose = m_nTriggerTimeLeft == 0;
	if( m_bStartOpen )
		bClose = !bClose;
	if( bClose )
	{
		float dLen = Min( m_bStartOpen ? m_fMaxLength - m_fCurLength : m_fCurLength, m_fCloseLenPerTick );
		if( dLen > 0 )
		{
			CVector2 dirs[4] = { { 1, 0 }, { 0, 1 }, { -1, 0 }, { 0, -1 } };
			dLen = GetLevel()->Push( this, dirs[m_nDir], dLen );
		}
	}
	else
	{
		float dLen = Min( m_bStartOpen ? m_fCurLength : m_fMaxLength - m_fCurLength, m_fOpenLenPerTick );
		if( dLen > 0 )
		{
			auto& hitTestMgr = GetLevel()->GetHitTestMgr();
			hitTestMgr.Remove( this );
			m_fCurLength += m_bStartOpen ? -dLen : dLen;
			UpdateHit();
			hitTestMgr.Add( this );
			hitTestMgr.Update( this );
		}
	}
	UpdateImages();
	if( m_nTriggerTimeLeft )
		m_nTriggerTimeLeft--;
}

void CGate::Render( CRenderContext2D & context )
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

	for( int i = 0; i < ELEM_COUNT( m_elems ); i++ )
	{
		auto& elem = m_elems[i];
		elem.worldMat = globalTransform;
		elem.SetDrawable( pDrawables[nPass] );
		context.AddElement( &elem, nGroup );
	}
}

void CGate::HandlePush( const CVector2& dir, float fDist, int8 nStep )
{
	auto& hitTestMgr = GetLevel()->GetHitTestMgr();
	if( nStep == 0 )
	{
		if( fDist > 0 )
		{
			hitTestMgr.Remove( this );
			m_fCurLength += m_bStartOpen ? fDist : -fDist;
			UpdateHit();
		}
	}
	else if( nStep == 1 )
	{
		if( fDist > 0 )
		{
			hitTestMgr.Add( this );
			hitTestMgr.Update( this );
		}
	}
}

bool CGate::Damage( SDamageContext& context )
{
	if( m_nTriggerType == 2 && context.nType <= eDamageHitType_Alert )
	{
		m_nTriggerTimeLeft = m_nTriggerTime;
		return true;
	}
	return false;
}

void CGate::Init()
{
	if( m_bInited )
		return;
	m_bInited = true;
	InitImages();
	UpdateHit();
}

void CGate::InitImages()
{
	auto pImg = static_cast<CImage2D*>( GetRenderObject() );
	m_rect0 = pImg->GetElem().rect;
	m_rect0.SetSize( m_rect0.GetSize() * 0.5f );
	m_texRect0 = m_texRect1 = pImg->GetElem().texRect;
	m_texRect0.SetSize( m_texRect0.GetSize() * 0.5f );
	pImg->SetRect( m_rect0 );
	pImg->SetTexRect( m_texRect0 );
	pImg->SetBoundDirty();
	CMatrix2D mat;
	mat.Identity();
	static_cast<SHitProxyPolygon*>( Get_HitProxy() )->CalcBound( mat, m_initHit );

	auto rect = m_rect0;
	switch( m_nDir )
	{
	case 0:
		m_elems[0].rect = CRectangle( m_rect0.x + m_rect0.width * 0.5f, m_rect0.y, m_rect0.width * 0.5f, m_rect0.height * 0.5f );
		m_elems[1].rect = CRectangle( m_rect0.x + m_rect0.width * 0.5f, m_rect0.y + m_rect0.height * 0.5f, m_rect0.width * 0.5f, m_rect0.height * 0.5f );
		m_elems[2].rect = CRectangle( m_rect0.x + m_rect0.width * 0.5f, m_rect0.y, 0, m_rect0.height * 0.5f );
		m_elems[3].rect = CRectangle( m_rect0.x + m_rect0.width * 0.5f, m_rect0.y + m_rect0.height * 0.5f, 0, m_rect0.height * 0.5f );
		m_elems[0].texRect = CRectangle( m_texRect0.GetRight(), m_texRect0.GetBottom(), m_texRect1.GetRight() - m_texRect0.GetRight(), m_texRect1.GetBottom() - m_texRect0.GetBottom() );
		m_elems[1].texRect = CRectangle( m_texRect0.GetRight(), m_texRect1.y, m_texRect1.GetRight() - m_texRect0.GetRight(), m_texRect0.y - m_texRect1.y );
		m_elems[2].texRect = CRectangle( m_texRect0.x + m_texRect0.width / 2, m_texRect0.GetBottom(), m_texRect0.width / 2, m_texRect1.GetBottom() - m_texRect0.GetBottom() );
		m_elems[3].texRect = CRectangle( m_texRect0.x + m_texRect0.width / 2, m_texRect1.y, m_texRect0.width / 2, m_texRect0.y - m_texRect1.y );
		break;
	case 1:
		m_elems[0].rect = CRectangle( m_rect0.x, m_rect0.y + m_rect0.height * 0.5f, m_rect0.width * 0.5f, m_rect0.height * 0.5f );
		m_elems[1].rect = CRectangle( m_rect0.x + m_rect0.width * 0.5f, m_rect0.y + m_rect0.height * 0.5f, m_rect0.width * 0.5f, m_rect0.height * 0.5f );
		m_elems[2].rect = CRectangle( m_rect0.x, m_rect0.y + m_rect0.height * 0.5f, m_rect0.width * 0.5f, 0 );
		m_elems[3].rect = CRectangle( m_rect0.x + m_rect0.width * 0.5f, m_rect0.y + m_rect0.height * 0.5f, m_rect0.width * 0.5f, 0 );
		m_elems[0].texRect = CRectangle( m_texRect1.x, m_texRect1.y, m_texRect0.x - m_texRect1.x, m_texRect0.y - m_texRect1.y );
		m_elems[1].texRect = CRectangle( m_texRect0.GetRight(), m_texRect1.y, m_texRect1.GetRight() - m_texRect0.GetRight(), m_texRect0.y - m_texRect1.y );
		m_elems[2].texRect = CRectangle( m_texRect1.x, m_texRect0.y + m_texRect0.height / 2, m_texRect0.x - m_texRect1.x, m_texRect0.height / 2 );
		m_elems[3].texRect = CRectangle( m_texRect0.GetRight(), m_texRect0.y + m_texRect0.height / 2, m_texRect1.GetRight() - m_texRect0.GetRight(), m_texRect0.height / 2 );
		break;
	case 2:
		m_elems[0].rect = CRectangle( m_rect0.x, m_rect0.y, m_rect0.width * 0.5f, m_rect0.height * 0.5f );
		m_elems[1].rect = CRectangle( m_rect0.x, m_rect0.y + m_rect0.height * 0.5f, m_rect0.width * 0.5f, m_rect0.height * 0.5f );
		m_elems[2].rect = CRectangle( m_rect0.x + m_rect0.width * 0.5f, m_rect0.y, 0, m_rect0.height * 0.5f );
		m_elems[3].rect = CRectangle( m_rect0.x + m_rect0.width * 0.5f, m_rect0.y + m_rect0.height * 0.5f, 0, m_rect0.height * 0.5f );
		m_elems[0].texRect = CRectangle( m_texRect1.x, m_texRect0.GetBottom(), m_texRect0.x - m_texRect1.x, m_texRect1.GetBottom() - m_texRect0.GetBottom() );
		m_elems[1].texRect = CRectangle( m_texRect1.x, m_texRect1.y, m_texRect0.x - m_texRect1.x, m_texRect0.y - m_texRect1.y );
		m_elems[2].texRect = CRectangle( m_texRect0.x + m_texRect0.width / 2, m_texRect0.GetBottom(), m_texRect0.width / 2, m_texRect1.GetBottom() - m_texRect0.GetBottom() );
		m_elems[3].texRect = CRectangle( m_texRect0.x + m_texRect0.width / 2, m_texRect1.y, m_texRect0.width / 2, m_texRect0.y - m_texRect1.y );
		break;
	case 3:
		m_elems[0].rect = CRectangle( m_rect0.x, m_rect0.y, m_rect0.width * 0.5f, m_rect0.height * 0.5f );
		m_elems[1].rect = CRectangle( m_rect0.x + m_rect0.width * 0.5f, m_rect0.y, m_rect0.width * 0.5f, m_rect0.height * 0.5f );
		m_elems[2].rect = CRectangle( m_rect0.x, m_rect0.y + m_rect0.height * 0.5f, m_rect0.width * 0.5f, 0 );
		m_elems[3].rect = CRectangle( m_rect0.x + m_rect0.width * 0.5f, m_rect0.y + m_rect0.height * 0.5f, m_rect0.width * 0.5f, 0 );
		m_elems[0].texRect = CRectangle( m_texRect1.x, m_texRect0.GetBottom(), m_texRect0.x - m_texRect1.x, m_texRect1.GetBottom() - m_texRect0.GetBottom() );
		m_elems[1].texRect = CRectangle( m_texRect0.GetRight(), m_texRect0.GetBottom(), m_texRect1.GetRight() - m_texRect0.GetRight(), m_texRect1.GetBottom() - m_texRect0.GetBottom() );
		m_elems[2].texRect = CRectangle( m_texRect1.x, m_texRect0.y + m_texRect0.height / 2, m_texRect0.x - m_texRect1.x, m_texRect0.height / 2 );
		m_elems[3].texRect = CRectangle( m_texRect0.GetRight(), m_texRect0.y + m_texRect0.height / 2, m_texRect1.GetRight() - m_texRect0.GetRight(), m_texRect0.height / 2 );
		break;
	default:
		break;
	}
	for( int i = 0; i < 4; i++ )
	{
		m_elems[i].nInstDataSize = static_cast<CImage2D*>( GetRenderObject() )->GetParamCount() * sizeof( CVector4 );
		m_elems[i].pInstData = static_cast<CImage2D*>( GetRenderObject() )->GetParam();
	}
	UpdateImages();
}

void CGate::UpdateImages()
{
	auto fLen = m_bStartOpen ? m_fCurLength : m_fMaxLength - m_fCurLength;

	auto rect = m_rect0;
	switch( m_nDir )
	{
	case 0:
		rect.width += fLen;
		m_elems[0].rect.x = m_elems[1].rect.x = m_rect0.x + m_rect0.width * 0.5f + fLen;
		m_elems[2].rect.width = m_elems[3].rect.width = fLen;
		break;
	case 1:
		rect.height += fLen;
		m_elems[0].rect.y = m_elems[1].rect.y = m_rect0.y + m_rect0.height * 0.5f + fLen;
		m_elems[2].rect.height = m_elems[3].rect.height = fLen;
		break;
	case 2:
		rect.SetLeft( rect.x - fLen );
		m_elems[0].rect.x = m_elems[1].rect.x = m_rect0.x - fLen;
		m_elems[2].rect.x = m_elems[3].rect.x = m_rect0.x + m_rect0.width * 0.5f - fLen;
		m_elems[2].rect.width = m_elems[3].rect.width = fLen;
		break;
	case 3:
		rect.SetTop( rect.y - fLen );
		m_elems[0].rect.y = m_elems[1].rect.y = m_rect0.y - fLen;
		m_elems[2].rect.y = m_elems[3].rect.y = m_rect0.y + m_rect0.height * 0.5f - fLen;
		m_elems[2].rect.height = m_elems[3].rect.height = fLen;
		break;
	default:
		break;
	}
	SetLocalBound( rect );
}

void CGate::UpdateHit()
{
	auto fLen = m_bStartOpen ? m_fCurLength : m_fMaxLength - m_fCurLength;
	auto rect = m_initHit;
	switch( m_nDir )
	{
	case 0:
		rect.width += fLen;
		break;
	case 1:
		rect.height += fLen;
		break;
	case 2:
		rect.SetLeft( rect.x - fLen );
		break;
	case 3:
		rect.SetTop( rect.y - fLen );
		break;
	default:
		break;
	}
	RemoveProxy( Get_HitProxy() );
	AddRect( rect );
}

void CAlertBeam::OnTickAfterHitTest()
{
	__super::OnTickAfterHitTest();
	if( m_nAlertTimeLeft )
		m_nAlertTimeLeft--;
}

void CAlertBeam::HandleHit( CEntity* pEntity, const CVector2& hitPoint )
{
	CCharacter* pCharacter = SafeCast<CCharacter>( pEntity );
	if( !pCharacter )
		return;
	if( m_nAlertTimeLeft && pCharacter->IsEnemy() )
	{
		SDamageContext context;
		context.nSourceType = 3;
		context.hitPos = hitPoint;
		context.hitDir = globalTransform.MulVector2Dir( CVector2( m_fHitForce, 0 ) );
		context.nHitType = -1;
		context.nType = eDamageHitType_Alert;
		pCharacter->Damage( context );
		return;
	}
	if( SafeCast<CPlayer>( pEntity ) )
	{
		OnHit( pEntity );
		m_nAlertTimeLeft = m_nAlertTime;
	}
}

void CAlertBeam::UpdateImages()
{
	__super::UpdateImages();
	if( m_nAlertTimeLeft )
	{
		for( int i = 0; i < 3; i++ )
		{
			auto texRect = static_cast<CImage2D*>( m_pBeamImg[i].GetPtr() )->GetElem().texRect;
			texRect.x += m_alertTexOfs.x;
			texRect.y += m_alertTexOfs.y;
			static_cast<CImage2D*>( m_pBeamImg[i].GetPtr() )->SetTexRect( texRect );
		}
	}
}

void CEnemy1::OnAddedToStage()
{
	CCharacter::OnAddedToStage();
	m_initDir = GetGlobalTransform().MulVector2Dir( CVector2( 1, 0 ) );
	m_initVel = GetVelocity();
	if( m_nVelType == 2 )
	{
		m_velFactor.y += SRand::Inst().Rand( -PI, PI ) * m_velFactor.z;
	}
}

void CEnemy1::OnTickAfterHitTest()
{
	CVector2 vel = m_initVel;
	float t = m_nTick * 1.0f / m_nVelPeriod;
	m_nTick++;
	if( m_nTick >= m_nVelPeriod )
		m_nTick = 0;
	switch( m_nVelType )
	{
	case 1:
	{
		float y = ( ( m_velFactor.w * t + m_velFactor.z ) * t + m_velFactor.y ) * t + m_velFactor.x;
		vel = vel + CVector2( -m_initDir.y, m_initDir.x ) * y;
	}
	break;
	case 2:
	{
		float y = sin( t * PI * 2 + m_velFactor.y ) * m_velFactor.x;
		vel = vel + CVector2( -m_initDir.y, m_initDir.x ) * y;
		break;
	}
	break;
	}
	auto d = ( GetVelocity() + vel ) * GetLevel()->GetElapsedTimePerTick() * 0.5f;
	SetVelocity( vel );
	SCharacterMovementData data;
	SRaycastResult result[3];
	data.TryMove( this, d, result );
	if( result[0].pHitProxy )
		Kill();
}

void RegisterGameClasses_EffectObject();
void RegisterGameClasses_Bullet();
void RegisterGameClasses_Explosion();
void RegisterGameClasses_Beam();
void RegisterGameClasses_PlayerMisc();
void RegisterGameClasses_EffectMisc();
void RegisterGameClasses_Bot();
void RegisterGameClasses_CharacterMisc()
{
	REGISTER_CLASS_BEGIN( CDamageArea )
		REGISTER_BASE_CLASS( CEntity )
		REGISTER_MEMBER( m_bIgnoreDamageSource )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CCommonMoveableObject )
		REGISTER_BASE_CLASS( CCharacter )
		REGISTER_MEMBER( m_fGravity )
		REGISTER_MEMBER( m_fFrac )
		REGISTER_MEMBER( m_fMaxFallSpeed )
		REGISTER_MEMBER( m_fAirborneFrac )
		REGISTER_MEMBER( m_nBlockImpactLevel )
		REGISTER_MEMBER( m_nImpactHitTime )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CCommonGrabbable )
		REGISTER_BASE_CLASS( CCharacter )
		REGISTER_BASE_CLASS( IGrabbable )
		REGISTER_MEMBER( m_grabOfs )
		REGISTER_MEMBER( m_fDropThreshold )
		REGISTER_MEMBER( m_nGrabDir )
		REGISTER_MEMBER( m_nDetachType )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CLever )
		REGISTER_BASE_CLASS( CCommonGrabbable )
		REGISTER_MEMBER( m_arrState )
		REGISTER_MEMBER( m_arrTransfer )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CChunk )
		REGISTER_BASE_CLASS( CCharacter )
		REGISTER_BASE_CLASS( IEditorTiled )
		REGISTER_MEMBER( m_nTypeX )
		REGISTER_MEMBER( m_nTypeY )
		REGISTER_MEMBER( m_nTex1Type )
		REGISTER_MEMBER( m_nTexX )
		REGISTER_MEMBER( m_nTexY )
		REGISTER_MEMBER( m_nTexX1 )
		REGISTER_MEMBER( m_nTexY1 )
		REGISTER_MEMBER( m_tileSize )
		REGISTER_MEMBER( m_nTileX )
		REGISTER_MEMBER( m_nTileY )
		REGISTER_MEMBER( m_ofs )
		REGISTER_MEMBER( m_hitSize )
		REGISTER_MEMBER( m_fHpPerTile )
		REGISTER_MEMBER( m_bNoHit )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CChunkPortal )
		REGISTER_BASE_CLASS( CChunk )
		REGISTER_MEMBER( m_bUp )
		REGISTER_MEMBER( m_colorDefault )
		REGISTER_MEMBER( m_colorInvalid )
		REGISTER_MEMBER( m_colorValid )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( SChunk1EditGroup )
		REGISTER_MEMBER( nAutoTypeCol )
		REGISTER_MEMBER( nAutoTypeRow )
		REGISTER_MEMBER( nTexX )
		REGISTER_MEMBER( nTexY )
		REGISTER_MEMBER( texRect )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CChunk1 )
		REGISTER_BASE_CLASS( CCharacter )
		REGISTER_BASE_CLASS( IEditorTiled )
		REGISTER_MEMBER( m_arrEditGroup )
		REGISTER_MEMBER( m_nEditGroup )
		REGISTER_MEMBER_BEGIN( m_arrCols )
			MEMBER_ARG( editor_hide, 1 )
		REGISTER_MEMBER_END()
		REGISTER_MEMBER_BEGIN( m_arrRows )
			MEMBER_ARG( editor_hide, 1 )
		REGISTER_MEMBER_END()
		REGISTER_MEMBER_BEGIN( m_arrData )
			MEMBER_ARG( editor_hide, 1 )
		REGISTER_MEMBER_END()
		REGISTER_MEMBER( m_tileSize )
		REGISTER_MEMBER( m_nTileX )
		REGISTER_MEMBER( m_nTileY )
		REGISTER_MEMBER( m_ofs )
		REGISTER_MEMBER( m_hitSize )
		REGISTER_MEMBER( m_fHpPerTile )
		REGISTER_MEMBER( m_bNoHit )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CAlertTrigger )
		REGISTER_BASE_CLASS( CCharacter )
		REGISTER_MEMBER( m_alertRect )
		REGISTER_MEMBER( m_alertVel )
		REGISTER_MEMBER( m_color0 )
		REGISTER_MEMBER( m_colorTriggered )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CCharacterTriggerSpawn )
		REGISTER_BASE_CLASS( CEntity )
		REGISTER_MEMBER( m_eEvent )
		REGISTER_MEMBER( m_pPrefab )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( SBulletDesc )
		REGISTER_MEMBER( pPrefab )
		REGISTER_MEMBER( ofs )
		REGISTER_MEMBER( vel )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CTurret )
		REGISTER_BASE_CLASS( CEntity )
		REGISTER_BASE_CLASS( IBotModule )
		REGISTER_MEMBER( m_fSightRange )
		REGISTER_MEMBER( m_fSightAngle )
		REGISTER_MEMBER( m_fRotAngle )
		REGISTER_MEMBER( m_fRotSpeed )
		REGISTER_MEMBER( m_nScanType )
		REGISTER_MEMBER( m_pBullet )//
		REGISTER_MEMBER( m_arrBulletOfs )//
		REGISTER_MEMBER( m_fBulletVel )//
		REGISTER_MEMBER( m_arrBulletDesc )
		REGISTER_MEMBER( m_nBulletCount )
		REGISTER_MEMBER( m_fBulletAngle )
		REGISTER_MEMBER( m_nFireCount )
		REGISTER_MEMBER( m_nFireInterval )
		REGISTER_MEMBER( m_nReloadTime )
		REGISTER_MEMBER( m_nActivateTime )
		REGISTER_MEMBER( m_nDetectActivateTime )
		REGISTER_MEMBER( m_pStaticEftPreActivePrefab )
		REGISTER_MEMBER( m_pStaticEftPrefab )
		REGISTER_MEMBER( m_staticEftOfs )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CGate )
		REGISTER_BASE_CLASS( CCharacter )
		REGISTER_MEMBER( m_nDir )
		REGISTER_MEMBER( m_bStartOpen )
		REGISTER_MEMBER( m_nTriggerType )
		REGISTER_MEMBER( m_fMaxLength )
		REGISTER_MEMBER( m_fOpenLenPerTick )
		REGISTER_MEMBER( m_fCloseLenPerTick )
		REGISTER_MEMBER( m_nTriggerTime )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CAlertBeam )
		REGISTER_BASE_CLASS( CBeam )
		REGISTER_MEMBER( m_nAlertTime )
		REGISTER_MEMBER( m_alertTexOfs )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CEnemy1 )
		REGISTER_BASE_CLASS( CCharacter )
		REGISTER_MEMBER( m_nVelType )
		REGISTER_MEMBER( m_nVelPeriod )
		REGISTER_MEMBER( m_velFactor )
	REGISTER_CLASS_END()

	RegisterGameClasses_EffectObject();
	RegisterGameClasses_Bullet();
	RegisterGameClasses_Explosion();
	RegisterGameClasses_Beam();
	RegisterGameClasses_PlayerMisc();
	RegisterGameClasses_EffectMisc();
	RegisterGameClasses_Bot();
}