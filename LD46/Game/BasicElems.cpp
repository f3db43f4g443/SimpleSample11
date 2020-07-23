#include "stdafx.h"
#include "BasicElems.h"
#include "Stage.h"
#include "MyLevel.h"
#include "Render/Image2D.h"
#include "MyGame.h"
#include "Common/Rand.h"
#include "GlobalCfg.h"
#include "CommonUtils.h"

void CLevelSpawnHelper::OnPreview()
{
	auto pPawn = SafeCast<CPawn>( GetRenderObject() );
	pPawn->m_pos = pPawn->m_moveTo = TVector2<int32>( 0, 0 );
	pPawn->m_nCurDir = pPawn->m_nInitDir;
	pPawn->OnPreview();
}

void CPawn::OnPreview()
{
	if( m_arrSubStates.Size() )
	{
		m_nCurDir = m_nInitDir;
		int32 nInitState;
		if( m_bUseInitState )
		{
			nInitState = m_nInitState;
			if( nInitState < 0 || nInitState >= m_arrSubStates.Size() || m_arrSubStates[nInitState].nForm != m_nCurForm )
				nInitState = GetDefaultState();
		}
		else
			nInitState = GetDefaultState();
		ChangeState( nInitState, true );
		Update0();
	}
	for( auto pChild = Get_TransformChild(); pChild; pChild = pChild->NextTransformChild() )
	{
		auto pMount = SafeCast<CPlayerMount>( pChild );
		if( pMount )
			pMount->OnPreview();
	}
}

void CPawn::Init()
{
	if( m_pHpBar && m_hpBarOrigRect.width == 0 )
		m_hpBarOrigRect = static_cast<CImage2D*>( m_pHpBar.GetPtr() )->GetElem().rect;
	InitState();
	if( m_pUsage )
		m_pUsage->Init();

	for( auto pChild = Get_ChildEntity(); pChild; pChild = pChild->NextChildEntity() )
	{
		auto pMount = SafeCast<CPlayerMount>( pChild );
		if( pMount )
			pMount->Init();
	}
}

void CPawn::Update()
{
	bVisible = !m_bMountHide && !m_bForceHide;
	if( m_arrSubStates.Size() )
	{
		if( !m_bMounted )
		{
			while( 1 )
			{
				bool bInterrupted = IsCurStateInterrupted();
				bool bCheckAction = false;
				bool bCanCheckAction = true;
				bool bJumpTo = false;
				if( m_pAI )
					bCanCheckAction = m_pAI->CanCheckAction( GetLevel()->IsScenario() );
				else
					bCanCheckAction = !GetLevel()->IsScenario();

				auto& curState = GetCurState();
				if( !bInterrupted )
				{
					for( int i = 0; i < curState.arrEvts.Size(); i++ )
					{
						auto& evt = curState.arrEvts[i];
						if( evt.nTick != m_nCurStateTick )
							continue;
						if( evt.eType == ePawnStateEventType_CheckAction )
						{
							if( bCanCheckAction )
								bCheckAction = true;
						}
						else if( evt.eType == ePawnStateEventType_MoveBegin )
						{
							bool bSuccess = GetLevel()->PawnMoveTo( this, TVector2<int32>( evt.nParams[0] * ( m_nCurDir ? -1 : 1 ), evt.nParams[1] ), evt.nParams[2] );
							if( !bSuccess )
							{
								bInterrupted = true;
								break;
							}
						}
						else if( evt.eType == ePawnStateEventType_MoveEnd )
							GetLevel()->PawnMoveEnd( this );
						else if( evt.eType == ePawnStateEventType_Hit )
						{
							auto hitOfs = OnHit( evt );
							auto pHitDesc = GetHitSpawn( evt.nParams[3] );
							if( pHitDesc )
							{
								auto pHit = SafeCast<CPawn>( pHitDesc->pHit->GetRoot()->CreateInstance() );
								TVector2<int32> ofs( pHitDesc->nOfsX, pHitDesc->nOfsY );
								ofs = ofs + hitOfs;
								if( m_nCurDir )
								{
									ofs.x = m_nWidth - ( ofs.x + pHit->m_nWidth );
								}
								auto pPawnHit = SafeCast<CPawnHit>( pHit );
								if( pPawnHit )
									pPawnHit->SetHitOfs( ofs );
								auto pos = m_moveTo + ofs;
								auto dir = pHitDesc->nDir ? 1 - m_nCurDir : m_nCurDir;
								if( GetLevel()->AddPawn( pHit, pos, dir, this ) )
									pHit->Signal( evt.nParams[2] );
							}
						}
						else if( evt.eType == ePawnStateEventType_Death )
						{
							GetLevel()->PawnDeath( this );
							return;
						}
						else if( evt.eType == ePawnStateEventType_Transform )
						{
							bool bSuccess = GetLevel()->PawnTransform( this, evt.nParams[0], TVector2<int32>( evt.nParams[1], evt.nParams[2] ) );
							if( !bSuccess )
							{
								bInterrupted = true;
								break;
							}
						}
						else if( evt.eType == ePawnStateEventType_PickUp )
							SafeCast<CPlayer>( this )->TryPickUp( evt.nParams[0] );
						else if( evt.eType == ePawnStateEventType_Drop )
							SafeCast<CPlayer>( this )->TryDrop( evt.nParams[0], evt.nParams[1] );
						else if( evt.eType == ePawnStateEventType_Cost )
						{
							if( !StateCost( evt.nParams[1], evt.nParams[0] ) )
							{
								bInterrupted = true;
								break;
							}
						}
						else if( evt.eType == ePawnStateEventType_UnMount )
						{
							SafeCast<CPlayer>( this )->UnMount();
							bInterrupted = true;
							break;
						}
						else if( evt.eType == ePawnStateEventType_SetZ )
							m_nRenderOrder = evt.nParams[0];
						else if( evt.eType == ePawnStateEventType_Sound )
							PlaySoundEffect( evt.strParam );
						else if( evt.eType == ePawnStateEventType_JumpTo )
						{
							bJumpTo = true;
							TransitTo( evt.strParam, evt.nParams[0], ePawnStateTransitReason_JumpTo );
						}
						else if( evt.eType == ePawnStateEventType_Script )
						{
							CLuaMgr::Inst().Load( evt.strParam );
							CLuaMgr::Inst().PushLua( this );
							CLuaMgr::Inst().Call( 1, 1 );
							if( CLuaMgr::Inst().PopLuaValue<bool>() )
							{
								bInterrupted = true;
								break;
							}
						}
					}
				}

				if( bJumpTo )
					continue;
				if( bInterrupted )
					CheckStateTransits( GetDefaultState() );
				else if( bCheckAction )
				{
					if( GetLevel()->GetPlayer()->GetControllingPawn() == this )
					{
						if( !GetLevel()->GetPlayer()->ControllingPawnCheckAction() )
							break;
					}
					else
					{
						if( !CheckAction() )
							break;
					}
				}
				else
					break;
			}
		}
		if( !m_bMountHide )
			Update0();
	}
	if( m_pUsage )
		m_pUsage->Update();
}

void CPawn::Update1()
{
	if( !m_arrSubStates.Size() || m_bMounted )
		return;
	m_nCurStateTick++;
	auto& curState = GetCurState();
	bool bFinished = false;
	if( curState.nTotalTicks )
	{
		if( m_nCurStateTick >= curState.nTotalTicks )
			bFinished = true;
	}
	else if( m_nCurStateTick >= curState.nImgTexCount * curState.nTicksPerFrame )
		m_nCurStateTick = 0;
	int32 nNewState = -1;
	bool bCanFinish = CheckCanFinish();
	if( bFinished && bCanFinish )
		nNewState = GetDefaultState();
	CheckStateTransits1( nNewState, ( bFinished || !curState.nTotalTicks ) && bCanFinish );
	m_bDamaged = false;
}

void CPawn::UpdateAnimOnly()
{
	if( m_arrSubStates.Size() )
	{
		auto& curState = GetCurState();
		for( int i = 0; i < curState.arrEvts.Size(); i++ )
		{
			auto& evt = curState.arrEvts[i];
			if( evt.nTick != m_nCurStateTick )
				continue;
			if( evt.eType == ePawnStateEventType_Sound )
				PlaySoundEffect( evt.strParam );
		}
		Update0();
		bool bFinished = curState.nTotalTicks && m_nCurStateTick >= curState.nTotalTicks;
		if( !bFinished )
			m_nCurStateTick++;
	}
}

int32 CPawn::Damage( int32 nDamage, int8 nDamageType, TVector2<int32> damageOfs )
{
	if( m_nArmorType > nDamageType )
		return 0;
	m_nHp = Max( 0, m_nHp - nDamage );
	if( !m_bDamaged || m_nDamageType < nDamageType )
	{
		m_nDamageType = nDamageType;
		m_damageOfs = damageOfs;
	}
	m_bDamaged = true;
	if( m_nHp <= 0 )
		OnKilled();
	return nDamage;
}

CMyLevel* CPawn::GetLevel()
{
	if( !GetParentEntity() )
		return NULL;
	return SafeCast<CMyLevel>( GetParentEntity()->GetParentEntity() );
}

int32 CPawn::GetStateIndexByName( const char* szName ) const
{
	for( int i = 0; i < m_arrSubStates.Size(); i++ )
	{
		if( m_arrSubStates[i].strName == szName )
			return i;
	}
	return -1;
}

bool CPawn::CanBeHit()
{
	if( m_bForceHit )
		return true;
	if( m_nHp <= 0 && !m_bIsEnemy )
		return false;
	return true;
}

int8 CPawn::GetDamageOfsDir()
{
	if( !m_damageOfs.x && !m_damageOfs.y )
		return -1;
	int32 x1 = abs( m_damageOfs.x );
	int8 n = 0;
	if( m_damageOfs.y >= x1 )
		n = 1;
	else if( m_damageOfs.y <= -x1 )
		n = 2;
	int8 nDir = m_nCurDir;
	if( m_damageOfs.x > 0 )
		nDir = 0;
	else if( m_damageOfs.x < 0 )
		nDir = 1;
	return n * 2 + nDir;
}

bool CPawn::PlayState( const char* sz )
{
	if( TransitTo( sz, -1, -1 ) )
	{
		if( !GetLevel()->IsBegin() )
			Update0();
		return true;
	}
	return false;
}

bool CPawn::PlayStateTurnBack( const char* sz )
{
	auto nDir0 = m_nCurDir;
	m_nCurDir = !m_nCurDir;
	if( !TransitTo( sz, -1, -1 ) )
	{
		m_nCurDir = nDir0;
		return false;
	}
	return true;
}

bool CPawn::PlayStateSetDir( const char* sz, int8 nDir )
{
	auto nDir0 = m_nCurDir;
	m_nCurDir = nDir;
	if( !TransitTo( sz, -1, -1 ) )
	{
		m_nCurDir = nDir0;
		return false;
	}
	return true;
}

CPawnAI* CPawn::ChangeAI( const char* sz )
{
	if( m_pAI )
	{
		m_pAI->SetParentEntity( NULL );
		m_pAI = NULL;
	}
	if( !sz || !sz[0] )
		return NULL;
	auto pPrefab = CResourceManager::Inst()->CreateResource<CPrefab>( sz );
	if( !pPrefab || !pPrefab->GetRoot()->GetStaticDataSafe<CPawnAI>() )
		return NULL;
	m_pAI = SafeCast<CPawnAI>( pPrefab->GetRoot()->CreateInstance() );
	m_pAI->SetParentEntity( this );
	return m_pAI;
}

const char* CPawn::GetCurStateName()
{
	return GetCurState().strName;
}

void CPawn::RegisterSignalScript()
{
	auto pTrigger = CLuaTrigger::CreateAuto( CLuaTrigger::eParam_Int );
	RegisterSignal( pTrigger );
}

void CPawn::RegisterKilledScript()
{
	auto pTrigger = CLuaTrigger::CreateAuto();
	RegisterKilled( pTrigger );
}

void CPawn::InitState()
{
	int32 nInitState;
	if( m_pSpawnHelper && m_pSpawnHelper->m_bSpawnDeath )
	{
		nInitState = m_pSpawnHelper->m_nDeathState;
		m_nHp = 0;
	}
	else if( m_bUseInitState )
	{
		nInitState = m_nInitState;
		if( nInitState < 0 || nInitState >= m_arrSubStates.Size() || m_arrSubStates[nInitState].nForm != m_nCurForm )
			nInitState = GetDefaultState();
	}
	else
		nInitState = GetDefaultState();
	if( m_arrSubStates.Size() )
	{
		ChangeState( nInitState, true );
		Update0();
	}
}

bool CPawn::CheckTransitCondition( EPawnStateTransitCondition eCondition, const char* strCondition )
{
	if( !strCondition || !strCondition[0] )
		return true;
	auto pLuaState = CLuaMgr::GetCurLuaState();
	pLuaState->Load( strCondition );
	pLuaState->PushLua( this );
	pLuaState->Call( 1, 1 );
	return pLuaState->PopLuaValue<bool>();
}

bool CPawn::TransitTo( const char* szToName, int32 nTo, int32 nReason )
{
	if( GetLevel()->GetPlayer()->GetControllingPawn() == this )
	{
		if( GetLevel()->GetPlayer()->ControllingPawnCheckStateInput( nReason ) )
			return true;
	}

	if( szToName && szToName[0] )
	{
		for( int i = 0; i < m_arrSubStates.Size(); i++ )
		{
			if( m_arrSubStates[i].strName == szToName )
			{
				ChangeState( i );
				return true;
			}
		}
	}
	if( nTo >= m_arrSubStates.Size() || nTo < 0 )
		return false;
	ChangeState( nTo );
	return true;
}

bool CPawn::EnumAllCommonTransits( function<bool( SPawnStateTransit1&, int32 )> Func )
{
	for( int i = 0; i < m_arrCommonStateTransits.Size(); i++ )
	{
		if( Func( m_arrCommonStateTransits[i], 0 ) )
			return true;
	}
	return false;
}

bool CPawn::FilterCommonTransit( SPawnStateTransit1& transit, int32 nSource )
{
	if( m_arrForms.Size() && m_nCurForm != m_arrSubStates[transit.nTo].nForm )
		return false;
	for( int i = 0; i < transit.arrStrExclude.Size(); i++ )
	{
		if( transit.arrStrExclude[i] == GetCurState().strName )
			return false;
	}
	if( nSource == m_nCurStateSource )
	{
		for( int i = 0; i < transit.arrExclude.Size(); i++ )
		{
			if( transit.arrExclude[i] == m_nCurState )
				return false;
		}
	}
	return true;
}

int32 CPawn::GetDefaultState()
{
	if( m_bUseDefaultState )
	{
		if( m_nDefaultState >= 0 && m_nDefaultState < m_arrSubStates.Size() && ( !m_arrForms.Size() || m_arrSubStates[m_nDefaultState].nForm == m_nCurForm ) )
			return m_nDefaultState;
	}
	if( !m_arrForms.Size() )
		return 0;
	return m_arrForms[m_nCurForm].nDefaultState;
}

bool CPawn::ChangeState( int32 nNewState, bool bInit )
{
	if( !IsValidStateIndex( nNewState ) )
		return false;
	m_nCurState = nNewState;
	ChangeState( m_arrSubStates[nNewState], 0, bInit );
	return true;
}

void CPawn::ChangeState( SPawnState& state, int32 nStateSource, bool bInit )
{
	ASSERT( !m_arrForms.Size() || state.nForm == m_nCurForm );
	if( !bInit )
		GetLevel()->PawnMoveBreak( this );
	m_nCurStateTick = 0;
	m_curStateBeginPos = CVector2( m_pos.x, m_pos.y ) * LEVEL_GRID_SIZE;
	CRectangle rect( m_origRect.x - state.nImgExtLeft * m_origRect.width, m_origRect.y - state.nImgExtTop * m_origRect.height,
		m_origRect.width * ( 1 + state.nImgExtLeft + state.nImgExtRight ), m_origRect.height * ( 1 + state.nImgExtTop + state.nImgExtBottom ) );
	if( m_nCurDir )
		rect = CRectangle( LEVEL_GRID_SIZE_X * m_nWidth - rect.GetRight(), rect.y, rect.width, rect.height );
	m_curStateRect = rect;
	m_curStateOrigTexRect = m_origTexRect;
	m_trigger.Trigger( 2, NULL );
}

void CPawn::Update0()
{
	auto& curState = GetCurState();
	CRectangle texRect( m_curStateOrigTexRect.x + curState.nImgTexBeginX * m_curStateOrigTexRect.width, m_curStateOrigTexRect.y + curState.nImgTexBeginY * m_curStateOrigTexRect.height,
		m_curStateOrigTexRect.width * ( 1 + curState.nImgExtLeft + curState.nImgExtRight ), m_curStateOrigTexRect.height * ( 1 + curState.nImgExtTop + curState.nImgExtBottom ) );
	int32 nFrame = Min( curState.nImgTexCount - 1, m_nCurStateTick / curState.nTicksPerFrame );
	texRect = texRect.Offset( CVector2( texRect.width * ( nFrame % curState.nImgTexCols ), texRect.height * ( nFrame / curState.nImgTexCols ) ) );
	if( m_nCurDir )
		texRect = CRectangle( 2 - texRect.GetRight(), texRect.y, texRect.width, texRect.height );
	auto pImage = static_cast<CImage2D*>( GetRenderObject() );
	pImage->SetRect( m_curStateRect );
	pImage->SetBoundDirty();
	pImage->SetTexRect( texRect );
	if( GetLevel() )
	{
		SetPosition( m_curStateBeginPos );
		if( m_pHpBar )
		{
			auto rect = m_hpBarOrigRect;
			rect.width = m_nHp * rect.width / m_nMaxHp;
			static_cast<CImage2D*>( m_pHpBar.GetPtr() )->SetRect( rect );
			m_pHpBar->SetBoundDirty();
			m_pHpBar->SetPosition( CVector2( m_moveTo.x, m_moveTo.y ) * LEVEL_GRID_SIZE - m_curStateBeginPos );
		}
	}
	else
	{
		if( m_pHpBar )
		{
			m_pHpBar->RemoveThis();
			m_pHpBar = NULL;
		}
	}
}

bool CPawn::CheckAction()
{
	auto nNewState = -1;
	if( m_pAI )
		nNewState = m_pAI->CheckAction( m_nCurDir );
	if( nNewState >= 0 )
	{
		ChangeState( nNewState );
		return true;
	}
	return false;
}

bool CPawn::CheckStateTransits( int32 nDefaultState )
{
	if( m_pAI )
	{
		auto n = m_pAI->CheckStateTransits( m_nCurDir );
		if( n >= 0 )
		{
			ChangeState( n );
			return true;
		}
	}
	auto& transits = GetCurState().arrTransits;
	auto& commonTransits = m_arrCommonStateTransits;
	for( int i = 0; i < transits.Size(); i++ )
	{
		auto& transit = transits[i];
		if( ( transit.eCondition == ePawnStateTransitCondition_Break || transit.eCondition == ePawnStateTransitCondition_Finish )
			&& CheckTransitCondition( transit.eCondition, transit.strCondition ) )
		{
			if( transit.bInverse )
				m_nCurDir = -m_nCurDir;
			TransitTo( transit.strToName, transit.nTo, transit.eCondition );
			return true;
		}
	}
	auto Func = [this] ( SPawnStateTransit1& transit, int32 nSource ) {
		if( !FilterCommonTransit( transit, nSource ) )
			return false;
		if( ( transit.eCondition == ePawnStateTransitCondition_Break || transit.eCondition == ePawnStateTransitCondition_Finish )
			&& CheckTransitCondition( transit.eCondition, transit.strCondition ) )
		{
			if( transit.bInverse )
				m_nCurDir = -m_nCurDir;
			TransitTo( transit.strToName, transit.nTo, transit.eCondition );
			return true;
		}
		return false;
	};
	if( !EnumAllCommonTransits( Func ) )
		ChangeState( nDefaultState );
	return true;
}

bool CPawn::CheckStateTransits1( int32 nDefaultState, bool bFinished )
{
	if( m_pAI )
	{
		auto n = m_pAI->CheckStateTransits1( m_nCurDir, bFinished );
		if( n >= 0 )
		{
			ChangeState( n );
			return true;
		}
	}
	auto& transits = GetCurState().arrTransits;
	auto& commonTransits = m_arrCommonStateTransits;
	for( int i = 0; i < transits.Size(); i++ )
	{
		auto& transit = transits[i];
		if( m_bDamaged && transit.eCondition == ePawnStateTransitCondition_Hit && CheckTransitCondition( transit.eCondition, transit.strCondition ) )
		{
			if( transit.bInverse )
				m_nCurDir = -m_nCurDir;
			TransitTo( transit.strToName, transit.nTo, transit.eCondition );
			return true;
		}
		if( m_nHp <= 0 && bFinished && transit.eCondition == ePawnStateTransitCondition_Killed && CheckTransitCondition( transit.eCondition, transit.strCondition ) )
		{
			if( transit.bInverse )
				m_nCurDir = -m_nCurDir;
			TransitTo( transit.strToName, transit.nTo, transit.eCondition );
			return true;
		}
		if( bFinished && transit.eCondition == ePawnStateTransitCondition_Finish && CheckTransitCondition( transit.eCondition, transit.strCondition ) )
		{
			if( transit.bInverse )
				m_nCurDir = -m_nCurDir;
			TransitTo( transit.strToName, transit.nTo, transit.eCondition );
			return true;
		}
	}
	auto Func = [this, bFinished] ( SPawnStateTransit1& transit, int32 nSource ) {
		if( !FilterCommonTransit( transit, nSource ) )
			return false;
		if( m_bDamaged && transit.eCondition == ePawnStateTransitCondition_Hit && CheckTransitCondition( transit.eCondition, transit.strCondition ) )
		{
			if( transit.bInverse )
				m_nCurDir = -m_nCurDir;
			TransitTo( transit.strToName, transit.nTo, transit.eCondition );
			return true;
		}
		if( m_nHp <= 0 && bFinished && transit.eCondition == ePawnStateTransitCondition_Killed && CheckTransitCondition( transit.eCondition, transit.strCondition ) )
		{
			if( transit.bInverse )
				m_nCurDir = -m_nCurDir;
			TransitTo( transit.strToName, transit.nTo, transit.eCondition );
			return true;
		}
		if( bFinished && transit.eCondition == ePawnStateTransitCondition_Finish && CheckTransitCondition( transit.eCondition, transit.strCondition ) )
		{
			if( transit.bInverse )
				m_nCurDir = -m_nCurDir;
			TransitTo( transit.strToName, transit.nTo, transit.eCondition );
			return true;
		}
		return false;
	};
	if( EnumAllCommonTransits( Func ) )
		return true;
	if( nDefaultState >= 0 )
	{
		ChangeState( nDefaultState );
		return true;
	}
	return false;
}

void CPawn::OnKilled()
{
	if( m_pSpawnHelper )
	{
		if( m_pSpawnHelper->m_nDataType == 1 )
		{
			auto& mapDeadPawn = GetStage()->GetMasterLevel()->GetCurLevelData().mapDataDeadPawn;
			auto& data = mapDeadPawn[m_pSpawnHelper->GetName().c_str()];
			data.p = GetMoveTo();
			data.nDir = m_nCurDir;
			data.nSpawnIndex = m_pSpawnHelper->m_nSpawnIndex;
		}
		else
		{
			if( m_pSpawnHelper->m_strDeathKey.length() )
				GetStage()->GetMasterLevel()->SetKeyInt( m_pSpawnHelper->m_strDeathKey, 1 );
		}
	}
	m_trigger.Trigger( 1, this );
	if( m_strKillScript.length() )
		CLuaMgr::Inst().Run( m_strKillScript );
}


void CPlayerEquipment::OnPreview()
{
	SetRenderObject( NULL );
	for( int i = 0; i < 2; i++ )
	{
		if( m_pEft[i] )
			m_pEft[i]->RemoveThis();
	}
}

void CPlayerEquipment::Init()
{
	if( m_pOrigRenderObject )
		return;
	m_pOrigRenderObject = GetRenderObject();
	SetRenderObject( NULL );
	for( int i = 0; i < 2; i++ )
	{
		if( m_pEft[i] )
			m_pEft[i]->RemoveThis();
	}
}

void CPlayerEquipment::Drop( class CPlayer* pPlayer, const TVector2<int32>& pos, int8 nDir, int32 nPickupState )
{
	if( !m_pPickUp || m_nEquipType == ePlayerEquipment_Ability )
		return;
	SafeCast<CPickUp>( m_pPickUp.GetPtr() )->PreDrop( this, nPickupState );
	pPlayer->GetLevel()->AddPawn( m_pPickUp, pos, nDir );
	m_pPickUp = NULL;
}

void CPlayerEquipment::PrePickedUp( CPawn* pPickUp )
{
	m_pPickUp = pPickUp;
}

void CPlayerEquipment::LoadData( IBufReader& buf )
{
	buf.Read( m_nAmmo );
}

void CPlayerEquipment::SaveData( CBufFile& buf )
{
	buf.Write( m_nAmmo );
}

void CPlayerMount::OnPreview()
{
	if( m_pEquipment )
		m_pEquipment->OnPreview();
}

void CPlayerMount::Init()
{
	m_levelPos = TVector2<int32>( -1, -1 );
	m_pEquipment->Init();
}

bool CPlayerMount::IsEnabled()
{
	if( m_bDisabled )
		return false;
	auto pPawn = GetPawn();
	if( pPawn->GetCurStateIndex() != m_nNeedStateIndex )
		return false;
	return true;
}

bool CPlayerMount::CheckMount( CPlayer* pPlayer )
{
	if( !pPlayer->IsReadyForMount( this ) )
		return false;
	auto pPawn = GetPawn();
	if( m_nEnterDir == 0 || m_nEnterDir == 1 )
	{
		if( ( pPawn->GetCurDir() ^ m_nEnterDir ) != pPlayer->GetCurDir() )
			return false;
	}
	return true;
}

void CPlayerMount::Mount( CPlayer* pPlayer )
{
	pPlayer->Mount( GetPawn(), m_pEquipment, m_strEntryState, m_bAnimPlayerOriented, !m_bShowPawnOnMount );
}

CPawn* CPlayerMount::GetPawn()
{
	for( auto p = GetParentEntity(); p; p = p->GetParentEntity() )
	{
		auto pPawn = SafeCast<CPawn>( p );
		if( pPawn )
			return pPawn;
	}
	return NULL;
}

enum
{
	ePlayerInput_Right_Down,
	ePlayerInput_Up_Down,
	ePlayerInput_Left_Down,
	ePlayerInput_Down_Down,
	ePlayerInput_Right_Up,
	ePlayerInput_Up_Up,
	ePlayerInput_Left_Up,
	ePlayerInput_Down_Up,
	ePlayerInput_A_Down,
	ePlayerInput_B_Down,
	ePlayerInput_C_Down,
	ePlayerInput_D_Down,
	ePlayerInput_A_Up,
	ePlayerInput_B_Up,
	ePlayerInput_C_Up,
	ePlayerInput_D_Up,
};

void CPlayer::Reset()
{
	for( int i = 0; i < ePlayerEquipment_Count; i++ )
		m_pCurEquipment[i] = NULL;
	m_nHp = m_nMaxHp;
	m_bEnableDefaultEquipment = false;
}

void CPlayer::LoadData( IBufReader& buf )
{
	for( int i = 0; i < ePlayerEquipment_Count; i++ )
		m_pCurEquipment[i] = NULL;

	int32 nVersionData;
	buf.Read( nVersionData );
	buf.Read( m_nHp );
	for( int i = 0; i < ePlayerEquipment_Count; i++ )
	{
		CBufReader tempBuf( buf );
		string strName;
		tempBuf.Read( strName );
		if( !strName.length() )
			continue;
		auto pPrefab = CResourceManager::Inst()->CreateResource<CPrefab>( strName.c_str() );
		if( !pPrefab || !pPrefab->GetRoot()->GetStaticDataSafe<CPickUp>() )
			continue;
		auto pPickUp = SafeCast<CPickUp>( pPrefab->GetRoot()->CreateInstance() );
		pPickUp->strCreatedFrom = strName.c_str();
		pPickUp->PickUp( this );
		ASSERT( m_pCurEquipment[i] && m_pCurEquipment[i] != m_pDefaultEquipment.GetPtr() );
		m_pCurEquipment[i]->LoadData( tempBuf );
		m_pCurEquipment[i]->Init();
	}
	buf.Read( m_bEnableDefaultEquipment );
}

void CPlayer::SaveData( CBufFile& buf )
{
	int32 nVersionData = 0;
	buf.Write( nVersionData );
	buf.Write( m_nHp );
	for( int i = 0; i < ePlayerEquipment_Count; i++ )
	{
		if( m_pCurEquipment[i] && m_pCurEquipment[i] != m_pDefaultEquipment.GetPtr() )
		{
			CBufFile tempBuf;
			tempBuf.Write( m_pCurEquipment[i]->m_pPickUp->strCreatedFrom );
			m_pCurEquipment[i]->SaveData( tempBuf );
			buf.Write( tempBuf );
		}
		else
			buf.Write( (int32)0 );
	}
	buf.Write( m_bEnableDefaultEquipment );
}

void CPlayer::Init()
{
	m_pDefaultEquipment->SetParentEntity( NULL );
	m_pCurUsingPawn = NULL;
	m_pCurMount = NULL;
	m_pCurMountingPawn = NULL;
	m_pControllingPawn = NULL;
	m_bForceUnMount = false;
	ChangeAI( NULL );
	m_vecInputQueues.resize( 0 );
	m_nActionEftFrame = 0;
	/*m_nMoveXInput = 0;
	m_nMoveYInput = 0;
	m_nAttackInput = 0;*/
	if( !m_pOrigRenderObject )
		m_pOrigRenderObject = GetRenderObject();
	for( int i = 0; i < 2; i++ )
	{
		if( !m_pCurEft[i] )
			m_pCurEft[i] = m_pEft[i];
	}
	CPawn::Init();
	m_pDefaultEquipment->Init();
	if( !m_pCurEquipment[0] && m_bEnableDefaultEquipment )
		Equip( m_pDefaultEquipment );
}

void CPlayer::Update()
{
	uint8 nKeyUp = 0;
	if( !GetLevel()->IsScenario() )
	{
		bool bInput = false;
		bool bDown = false;
		if( CGame::Inst().IsKeyUp( 'D' ) || CGame::Inst().IsKeyUp( 'd' ) ) { m_vecInputQueues.push_back( ePlayerInput_Right_Up ); bInput = true; nKeyUp |= 1; }
		if( CGame::Inst().IsKeyUp( 'W' ) || CGame::Inst().IsKeyUp( 'w' ) ) { m_vecInputQueues.push_back( ePlayerInput_Up_Up ); bInput = true; nKeyUp |= 2; }
		if( CGame::Inst().IsKeyUp( 'A' ) || CGame::Inst().IsKeyUp( 'a' ) ) { m_vecInputQueues.push_back( ePlayerInput_Left_Up ); bInput = true; nKeyUp |= 4; }
		if( CGame::Inst().IsKeyUp( 'S' ) || CGame::Inst().IsKeyUp( 's' ) ) { m_vecInputQueues.push_back( ePlayerInput_Down_Up ); bInput = true; nKeyUp |= 8; }
		if( CGame::Inst().IsKeyDown( 'D' ) || CGame::Inst().IsKeyDown( 'd' ) ) { m_vecInputQueues.push_back( ePlayerInput_Right_Down ); bInput = true; bDown = true; }
		if( CGame::Inst().IsKeyDown( 'W' ) || CGame::Inst().IsKeyDown( 'w' ) ) { m_vecInputQueues.push_back( ePlayerInput_Up_Down ); bInput = true; bDown = true; }
		if( CGame::Inst().IsKeyDown( 'A' ) || CGame::Inst().IsKeyDown( 'a' ) ) { m_vecInputQueues.push_back( ePlayerInput_Left_Down ); bInput = true; bDown = true; }
		if( CGame::Inst().IsKeyDown( 'S' ) || CGame::Inst().IsKeyDown( 's' ) ) { m_vecInputQueues.push_back( ePlayerInput_Down_Down ); bInput = true; bDown = true; }
		if( CGame::Inst().IsKeyUp( 'J' ) || CGame::Inst().IsKeyUp( 'j' ) ) { m_vecInputQueues.push_back( ePlayerInput_A_Up ); bInput = true; nKeyUp |= 16; }
		if( CGame::Inst().IsKeyUp( 'K' ) || CGame::Inst().IsKeyUp( 'k' ) ) { m_vecInputQueues.push_back( ePlayerInput_B_Up ); bInput = true; nKeyUp |= 32; }
		if( CGame::Inst().IsKeyUp( 'U' ) || CGame::Inst().IsKeyUp( 'u' ) ) { m_vecInputQueues.push_back( ePlayerInput_C_Up ); bInput = true; nKeyUp |= 64; }
		if( CGame::Inst().IsKeyUp( 'I' ) || CGame::Inst().IsKeyUp( 'i' ) ) { m_vecInputQueues.push_back( ePlayerInput_D_Up ); bInput = true; nKeyUp |= 128; }
		if( CGame::Inst().IsKeyDown( 'J' ) || CGame::Inst().IsKeyDown( 'j' ) ) { m_vecInputQueues.push_back( ePlayerInput_A_Down ); bInput = true; bDown = true; }
		if( CGame::Inst().IsKeyDown( 'K' ) || CGame::Inst().IsKeyDown( 'k' ) ) { m_vecInputQueues.push_back( ePlayerInput_B_Down ); bInput = true; bDown = true; }
		if( CGame::Inst().IsKeyDown( 'U' ) || CGame::Inst().IsKeyDown( 'u' ) ) { m_vecInputQueues.push_back( ePlayerInput_C_Down ); bInput = true; bDown = true; }
		if( CGame::Inst().IsKeyDown( 'I' ) || CGame::Inst().IsKeyDown( 'i' ) ) { m_vecInputQueues.push_back( ePlayerInput_D_Down ); bInput = true; bDown = true; }
		if( bDown && m_bActionStop )
			m_nTickInputOnActionStop = 10;
		if( bInput )
			GetStage()->GetMasterLevel()->GetMainUI()->RefreshPlayerInput( ParseInputSequence(), -1, m_nChargeKeyDown, 0 );
	}
	if( m_nTickInputOnActionStop )
	{
		m_nTickInputOnActionStop--;
		if( !m_nTickInputOnActionStop )
		{
			m_bActionStop = false;
			CheckAction();
		}
	}
	if( m_bForceUnMount )
		UnMount();
	if( m_pControllingPawn && ( m_pControllingPawn->IsKilled() || !m_pControllingPawn->GetLevel() ) )
		EndControl();
	CPawn::Update();

	if( !!( nKeyUp & m_nChargeKeyDown ) )
		m_nChargeKeyDown = 0;
	if( m_nActionEftFrame )
	{
		m_nActionEftFrame--;
		m_pCurEft[0]->bVisible = m_pCurEft[1]->bVisible = true;
		auto pImg = static_cast<CImage2D*>( GetRenderObject() );
		for( int i = 0; i < 2; i++ )
		{
			auto pImg1 = static_cast<CImage2D*>( m_pCurEft[i].GetPtr() );
			pImg1->SetRect( pImg->GetElem().rect );
			pImg1->SetTexRect( pImg->GetElem().texRect );
			pImg1->SetBoundDirty();
			pImg1->SetPosition( m_actionEftOfs[i * ACTION_EFT_FRAMES + m_nActionEftFrame] );
			pImg1->GetParam()[0] = m_actionEftParam[i * ACTION_EFT_FRAMES + m_nActionEftFrame];
		}
	}
	else
		m_pCurEft[0]->bVisible = m_pCurEft[1]->bVisible = false;
}

int32 CPlayer::Damage( int32 nDamage, int8 nDamageType, TVector2<int32> damageOfs )
{
	GetStage()->GetMasterLevel()->OnPlayerDamaged();
	return CPawn::Damage( nDamage, nDamageType, damageOfs );
}

bool CPlayer::TryPickUp( int32 nParam )
{
	if( m_pCurMountingPawn )
	{
		m_pCurMountingPawn->Signal( nParam );
		return true;
	}
	if( m_pCurUsingPawn )
	{
		if( m_pCurUsingPawn->GetStage() )
			m_pCurUsingPawn->GetUsage()->UseHit( this );
		return true;
	}

	auto pPickUp = GetLevel()->FindPickUp( GetMoveTo(), m_nWidth, m_nHeight );
	if( !pPickUp )
		return false;
	pPickUp->PickUp( this );
	return true;
}

bool CPlayer::TryDrop( int8 nType, int32 nPickupState )
{
	if( nType == 2 )
	{
		if( m_nCurStateSource > 0 && m_nCurStateSource <= ePlayerEquipment_Count )
			return TryDropIndex( m_nCurStateSource - 1, nPickupState );
		return false;
	}
	int32 nDrop[] = { ePlayerEquipment_Large, ePlayerEquipment_Melee, ePlayerEquipment_Ranged };
	bool bDrop = false;
	for( int i = 0; i < ELEM_COUNT( nDrop ); i++ )
	{
		auto iDrop = nDrop[i];
		if( TryDropIndex( iDrop, nPickupState ) )
		{
			bDrop = true;
			if( !nType )
				break;
		}
	}
	return bDrop;
}

bool CPlayer::TryDropIndex( int32 nIndex, int32 nPickupState )
{
	if( m_pCurEquipment[nIndex] == m_pDefaultEquipment.GetPtr() )
		return false;
	if( m_pCurEquipment[nIndex] )
	{
		UnEquip( m_pCurEquipment[nIndex], nPickupState );
		if( nIndex == ePlayerEquipment_Melee && m_bEnableDefaultEquipment )
			m_pCurEquipment[ePlayerEquipment_Melee] = m_pDefaultEquipment;
		return true;
	}
	return false;
}

void CPlayer::Equip( CPlayerEquipment* pEquipment )
{
	if( m_pCurEquipment[pEquipment->m_nEquipType] )
		UnEquip( m_pCurEquipment[pEquipment->m_nEquipType] );
	m_pCurEquipment[pEquipment->m_nEquipType] = pEquipment;
}

void CPlayer::UnEquip( CPlayerEquipment* pEquipment, int32 nPickupState )
{
	m_pCurEquipment[pEquipment->m_nEquipType]->Drop( this, m_moveTo, m_nCurDir, nPickupState );
	m_pCurEquipment[pEquipment->m_nEquipType] = NULL;
}

void CPlayer::Mount( CPawn* pPawn, CPlayerEquipment* pMount, const char* szState, bool bAnimPlayerOriented, bool bMountHide )
{
	m_pCurMountingPawn = pPawn;
	m_pCurMountingPawn->SetMounted( true, bMountHide );
	m_pCurMount = pMount;
	m_bMountAnimPlayerOriented = bAnimPlayerOriented;
	m_nDirBeforeMounting = m_nCurDir;
	if( !bAnimPlayerOriented )
		m_nCurDir = pPawn->GetCurDir();
	VERIFY( TransitTo( szState, -1, -1 ) );
}

void CPlayer::UnMount()
{
	m_pCurMountingPawn->SetMounted( false, false );
	m_pCurMount = NULL;
	m_pCurMountingPawn = NULL;
	m_bForceUnMount = false;
	m_nCurDir = m_nDirBeforeMounting;
}

void CPlayer::ForceUnMount()
{
	if( !m_pCurMount )
		return;
	m_bForceUnMount = true;
}

bool CPlayer::IsReadyForMount( CPlayerMount* pMount )
{
	if( m_pCurMount )
		return false;
	auto sz = pMount->GetCostEquipment();
	if( sz.length() )
	{
		if( !m_pCurEquipment[ePlayerEquipment_Large] || sz != m_pCurEquipment[ePlayerEquipment_Large]->GetEquipmentName() )
			return false;
	}
	else
	{
		if( m_pCurEquipment[ePlayerEquipment_Large] )
			return false;
	}
	return true;
}

SPawnState& CPlayer::GetCurState()
{
	if( m_pCurStateSource )
		return m_pCurStateSource->m_arrSubStates[m_nCurState];
	return CPawn::GetCurState();
}

SPawnHitSpawnDesc* CPlayer::GetHitSpawn( int32 nHit )
{
	if( m_pCurStateSource )
	{
		if( nHit >= 0 && nHit < m_pCurStateSource->m_arrHitSpawnDesc.Size() )
			return &m_pCurStateSource->m_arrHitSpawnDesc[nHit];
		return NULL;
	}
	return CPawn::GetHitSpawn( nHit );
}

SInputTableItem* CPlayer::GetCurInputResult()
{
	ParseInputSequence();
	if( m_pCurMount )
	{
		auto& inputTable = m_pCurMount->m_inputTable;
		for( int j = inputTable.Size() - 1; j >= 0; j-- )
		{
			auto& item = inputTable[j];
			if( CheckInputTableItem( item ) )
				return &item;
		}
		return NULL;
	}
	if( m_pCurEquipment[ePlayerEquipment_Large] )
	{
		auto& inputTable = m_pCurEquipment[ePlayerEquipment_Large]->m_inputTable;
		for( int j = inputTable.Size() - 1; j >= 0; j-- )
		{
			auto& item = inputTable[j];
			if( CheckInputTableItem( item ) )
				return &item;
		}
		return NULL;
	}
	for( int i = 0; i < ePlayerEquipment_Large; i++ )
	{
		if( m_pCurEquipment[i] )
		{
			auto& inputTable = m_pCurEquipment[i]->m_inputTable;
			for( int j = inputTable.Size() - 1; j >= 0; j-- )
			{
				auto& item = inputTable[j];
				if( CheckInputTableItem( item ) )
					return &item;
			}
		}
	}
	for( int i = m_inputTable.Size() - 1; i >= 0; i-- )
	{
		auto& item = m_inputTable[i];
		if( CheckInputTableItem( item ) )
			return &item;
	}
	return NULL;
}

void CPlayer::EnableDefaultEquipment()
{
	if( m_bEnableDefaultEquipment )
		return;
	m_bEnableDefaultEquipment = true;
	if( !m_pCurEquipment[0] )
		Equip( m_pDefaultEquipment );
}

void CPlayer::RestoreAmmo()
{
	auto pEquipment = GetEquipment( ePlayerEquipment_Ranged );
	if( pEquipment )
		pEquipment->SetAmmo( pEquipment->GetMaxAmmo() );
}

void CPlayer::BeginControl( CPawn* pPawn )
{
	m_pControllingPawn = pPawn;
}

void CPlayer::EndControl()
{
	m_pControllingPawn = NULL;
	m_bActionStop = false;
	FlushInput( 0, 0, 0 );
}

bool CPlayer::ControllingPawnCheckStateInput( int32 nReason )
{
	if( nReason == ePawnStateTransitCondition_Finish || nReason == ePawnStateTransitReason_JumpTo )
	{
		auto pStateSource = GetStateSource( m_nCurStateSource );
		if( pStateSource )
		{
			bool bChecked = false;
			ParseInputSequence();
			auto& inputTable = *m_pControllingPawn->GetControllingStateInputTable();
			for( int i = inputTable.Size() - 1; i >= 0; i-- )
			{
				auto& arrStates = inputTable[i].arrStates;
				for( int j = 0; j < arrStates.Size(); j++ )
				{
					if( arrStates[j] == m_nCurState )
					{
						bChecked = true;
						auto& item = inputTable[i].input;
						if( CheckInputTableItem( item ) )
						{
							int32 nMatchLen = item.strInput.length();
							int32 nChargeKey = 0;
							if( nMatchLen && item.strInput[nMatchLen - 1] == '#' )
								nChargeKey = m_nChargeKeyDown;
							if( ExecuteInputtableItem( item, m_nCurStateSource ) )
							{
								FlushInput( nMatchLen, nChargeKey, 1 );
								return true;
							}
							else
								return false;
						}
					}
				}
			}
			if( bChecked )
				GetStage()->GetMasterLevel()->GetMainUI()->InsertDefaultFinishAction();
		}
	}
	else if( nReason > ePawnStateTransitCondition_Finish )
		FlushInput( 0, 0, 2 );
	return false;
}

void CPlayer::SetInputSequence( const char* szInput )
{
	auto l = strlen( szInput );
	m_parsedInputSequence.resize( l );
	for( int i = 0; i < l; i++ )
	{
		auto chInput = szInput[i];
		auto nInput = 0;
		if( chInput >= '1' && chInput <= '9' )
		{
			if( chInput == '3' || chInput == '6' || chInput == '9' )
				nInput |= 1;
			if( chInput == '1' || chInput == '4' || chInput == '7' )
				nInput |= 4;
			if( chInput == '7' || chInput == '8' || chInput == '9' )
				nInput |= 2;
			if( chInput == '1' || chInput == '2' || chInput == '3' )
				nInput |= 8;
		}
		else if( chInput == 'A' )
			nInput = -1;
		else if( chInput == 'B' )
			nInput = -2;
		else if( chInput == 'C' )
			nInput = -3;
		else if( chInput == 'D' )
			nInput = -4;
		m_parsedInputSequence[i] = nInput;
	}
}

vector<int8>& CPlayer::ParseInputSequence()
{
	int8 nKeyDown = 0;
	bool bFinish = false;
	bool bActionKeyDown[4] = { 0, 0, 0, 0 };
	m_parsedInputSequence.resize( 0 );
	for( auto n : m_vecInputQueues )
	{
		auto k0 = nKeyDown;
		if( n <= ePlayerInput_Down_Down )
		{
			if( bFinish )
			{
				bFinish = false;
				m_parsedInputSequence.resize( 0 );
			}
			nKeyDown |= 1 << n;
			auto n2 = n ^ 2;
			nKeyDown &= ~( 1 << n2 );
		}
		else if( n <= ePlayerInput_Down_Up )
		{
			nKeyDown &= ~( 1 << ( n - 4 ) );
		}
		else if( n <= ePlayerInput_D_Down )
		{
			bActionKeyDown[n - ePlayerInput_A_Down] = true;
			continue;
		}
		else
		{
			if( !bActionKeyDown[n - ePlayerInput_A_Up] )
				continue;
			bActionKeyDown[n - ePlayerInput_A_Up] = false;
			if( bFinish )
				m_parsedInputSequence.back() = -1 - n + ePlayerInput_A_Up;
			else
				m_parsedInputSequence.push_back( -1 - n + ePlayerInput_A_Up );
			nKeyDown = 0;
			bFinish = true;
			continue;
		}
		if( nKeyDown && k0 != nKeyDown )
			m_parsedInputSequence.push_back( nKeyDown );
	}
	for( int i = 0; i < 4; i++ )
	{
		if( !bActionKeyDown[i] )
			continue;
		if( bFinish )
			m_parsedInputSequence.back() = -1 - i;
		else
			m_parsedInputSequence.push_back( -1 - i );
		bFinish = true;
	}
	return m_parsedInputSequence;
}

void CPlayer::InitState()
{
	if( m_pCurEquipment[ePlayerEquipment_Large] )
	{
		m_nCurState = 0;
		ChangeState( m_pCurEquipment[ePlayerEquipment_Large]->m_arrSubStates[0], ePlayerEquipment_Large + 1, true );
		Update0();
		return;
	}
	CPawn::InitState();
}

bool CPlayer::TransitTo( const char* szToName, int32 nTo, int32 nReason )
{
	if( nReason == ePawnStateTransitCondition_Finish || nReason == ePawnStateTransitReason_JumpTo )
	{
		auto pStateSource = GetStateSource( m_nCurStateSource );
		if( pStateSource )
		{
			bool bChecked = false;
			ParseInputSequence();
			auto& inputTable = m_nCurStateSource ? pStateSource->m_stateInputTable : m_stateInputTable;
			for( int i = inputTable.Size() - 1; i >= 0; i-- )
			{
				auto& arrStates = inputTable[i].arrStates;
				for( int j = 0; j < arrStates.Size(); j++ )
				{
					if( arrStates[j] == m_nCurState )
					{
						bChecked = true;
						auto& item = inputTable[i].input;
						if( CheckInputTableItem( item ) )
						{
							int32 nMatchLen = item.strInput.length();
							int32 nChargeKey = 0;
							if( nMatchLen && item.strInput[nMatchLen - 1] == '#' )
								nChargeKey = m_nChargeKeyDown;
							if( ExecuteInputtableItem( item, m_nCurStateSource ) )
							{
								m_nActionEftFrame = ACTION_EFT_FRAMES;
								FlushInput( nMatchLen, nChargeKey, 1 );
								return true;
							}
							else
								goto forcebreak;
						}
					}
				}
			}
			if( bChecked )
			{
				m_nActionEftFrame = ACTION_EFT_FRAMES;
				GetStage()->GetMasterLevel()->GetMainUI()->InsertDefaultFinishAction();
			}
		}
	}
	else if( nReason > ePawnStateTransitCondition_Finish )
		FlushInput( 0, 0, 2 );
forcebreak:
	if( szToName && szToName[0] )
	{
		if( m_pCurMount )
		{
			auto& arrStates = m_pCurMount->m_arrSubStates;
			for( int j = 0; j < arrStates.Size(); j++ )
			{
				if( arrStates[j].strName == szToName )
				{
					m_nCurState = j;
					ChangeState( arrStates[j], ePlayerStateSource_Mount, false );
					return true;
				}
			}
		}
		else
		{
			for( int i = 0; i < ePlayerEquipment_Count; i++ )
			{
				if( m_nCurStateSource && i != m_nCurStateSource - 1 )
					continue;
				if( m_pCurEquipment[i] )
				{
					auto& arrStates = m_pCurEquipment[i]->m_arrSubStates;
					for( int j = 0; j < arrStates.Size(); j++ )
					{
						if( arrStates[j].strName == szToName )
						{
							m_nCurState = j;
							ChangeState( arrStates[j], i + 1, false );
							return true;
						}
					}
				}
			}
		}
	}
	return CPawn::TransitTo( szToName, nTo, nReason );
}

bool CPlayer::EnumAllCommonTransits( function<bool( SPawnStateTransit1&, int32 )> Func )
{
	if( m_pCurMount )
	{
		auto& transits = m_pCurMount->m_arrCommonStateTransits;
		for( int i = 0; i < transits.Size(); i++ )
		{
			if( Func( transits[i], m_nCurStateSource ) )
				return true;
		}
		return false;
	}
	if( m_nCurStateSource )
	{
		auto pStateSource = GetStateSource( m_nCurStateSource );
		if( pStateSource )
		{
			auto& transits = pStateSource->m_arrCommonStateTransits;
			for( int i = 0; i < transits.Size(); i++ )
			{
				if( Func( transits[i], m_nCurStateSource ) )
					return true;
			}
		}
		if( m_nCurStateSource == ePlayerEquipment_Large + 1 )
			return false;
	}
	return CPawn::EnumAllCommonTransits( Func );
}

void CPlayer::ChangeState( SPawnState& state, int32 nStateSource, bool bInit )
{
	ASSERT( !m_arrForms.Size() || state.nForm == m_nCurForm );
	if( !bInit )
		GetLevel()->PawnMoveBreak( this );
	if( m_pCurUsingPawn )
	{
		if( m_pCurUsingPawn->GetStage() )
			m_pCurUsingPawn->GetUsage()->EndUse( this );
		m_pCurUsingPawn = NULL;
	}
	m_bActionStop = false;
	m_nChargeKeyDown = 0;
	m_nTickInputOnActionStop = 0;
	m_nCurStateTick = 0;
	auto origRect = m_origRect;
	auto origTexRect = m_origTexRect;
	m_pCurStateSource = GetStateSource( nStateSource );
	if( m_pCurStateSource )
	{
		origRect = m_pCurStateSource->m_origRect;
		origTexRect = m_pCurStateSource->m_origTexRect;
		m_pNewRenderObject = m_pCurStateSource->m_pOrigRenderObject;
		for( int i = 0; i < 2; i++ )
			m_pNewEft[i] = m_pCurStateSource->m_pEft[i];
	}
	else
	{
		m_pNewRenderObject = m_pOrigRenderObject;
		for( int i = 0; i < 2; i++ )
			m_pNewEft[i] = m_pEft[i];
	}
	m_nCurStateSource = nStateSource;

	m_curStateBeginPos = ( nStateSource == ePlayerStateSource_Mount && !m_bMountAnimPlayerOriented ?
		CVector2( m_pCurMountingPawn->GetPos().x, m_pCurMountingPawn->GetPos().y ) : CVector2( m_pos.x, m_pos.y ) ) * LEVEL_GRID_SIZE;
	CRectangle rect( origRect.x - state.nImgExtLeft * origRect.width, origRect.y - state.nImgExtTop * origRect.height,
		origRect.width * ( 1 + state.nImgExtLeft + state.nImgExtRight ), origRect.height * ( 1 + state.nImgExtTop + state.nImgExtBottom ) );
	if( m_nCurDir )
		rect = CRectangle( LEVEL_GRID_SIZE_X * m_nWidth - rect.GetRight(), rect.y, rect.width, rect.height );
	m_curStateRect = rect;
	m_curStateOrigTexRect = origTexRect;
	GetLevel()->OnPlayerChangeState( state, nStateSource, m_nCurDir );
	m_trigger.Trigger( 2, NULL );
}

void CPlayer::Update0()
{
	if( m_pNewRenderObject )
	{
		if( m_pNewRenderObject != GetRenderObject() )
			SetRenderObject( m_pNewRenderObject );
		m_pNewRenderObject = NULL;
	}
	for( int i = 0; i < 2; i++ )
	{
		if( m_pNewEft[i] )
		{
			if( m_pNewEft[i] != m_pCurEft[i].GetPtr() )
			{
				AddChildBefore( m_pNewEft[i], m_pCurEft[i] );
				m_pCurEft[i]->RemoveThis();
				m_pNewEft[i]->bVisible = false;
				m_pCurEft[i] = m_pNewEft[i];
			}
			m_pNewEft[i] = NULL;
		}
	}
	CPawn::Update0();
}

bool CPlayer::IsCurStateInterrupted()
{
	if( !m_pCurMount && m_nCurStateSource == ePlayerStateSource_Mount )
		return true;
	return false;
}

bool CPlayer::CheckAction()
{
	if( !m_bActionStop )
	{
		ParseInputSequence();
		bool b = false;
		if( CPawn::CheckAction() )
			b = true;
		else
			b = HandleInput();
		m_bActionStop = !b;
		return b;
	}
	return false;
}

bool CPlayer::CheckCanFinish()
{
	return !m_nChargeKeyDown && !m_pControllingPawn;
}

bool CPlayer::HandleInput()
{
	if( m_pControllingPawn )
	{
		auto& inputTable = *m_pControllingPawn->GetControllingInputTable();
		for( int j = inputTable.Size() - 1; j >= 0; j-- )
		{
			auto& item = inputTable[j];
			if( CheckInputTableItem( item ) )
			{
				bool b = ExecuteInputtableItem( item, -1 );
				ASSERT( b );
				FlushInput( item.strInput.length(), 0, 0 );
				return true;
			}
		}
		FlushInput( 0, 0, 0 );
		return false;
	}
	if( m_pCurMount )
	{
		auto& inputTable = m_pCurMount->m_inputTable;
		for( int j = inputTable.Size() - 1; j >= 0; j-- )
		{
			auto& item = inputTable[j];
			if( CheckInputTableItem( item ) )
			{
				bool b = ExecuteInputtableItem( item, ePlayerStateSource_Mount );
				ASSERT( b );
				FlushInput( item.strInput.length(), 0, 0 );
				return true;
			}
		}
		m_nCurState = 0;
		ChangeState( m_pCurMount->m_arrSubStates[0], ePlayerStateSource_Mount, false );
		FlushInput( 0, 0, 0 );
		return true;
	}
	if( m_pCurEquipment[ePlayerEquipment_Large] )
	{
		auto& inputTable = m_pCurEquipment[ePlayerEquipment_Large]->m_inputTable;
		for( int j = inputTable.Size() - 1; j >= 0; j-- )
		{
			auto& item = inputTable[j];
			if( CheckInputTableItem( item ) )
			{
				bool b = ExecuteInputtableItem( item, ePlayerEquipment_Large + 1 );
				ASSERT( b );
				FlushInput( item.strInput.length(), 0, 0 );
				return true;
			}
		}
		if( m_parsedInputSequence.size() && m_parsedInputSequence.back() == -4 )
		{
			FlushInput( 1, 0, 0 );
			auto pMount = GetLevel()->FindMount();
			if( pMount )
			{
				if( pMount->GetCostEquipment().length() )
				{
					ASSERT( m_pCurEquipment[ePlayerEquipment_Large]->GetEquipmentName() == pMount->GetCostEquipment() );
					m_pCurEquipment[ePlayerEquipment_Large] = NULL;
				}
				pMount->Mount( this );
				return true;
			}
			TransitTo( "break", 0, -1 );
			return true;
		}
		m_nCurState = 0;
		ChangeState( m_pCurEquipment[ePlayerEquipment_Large]->m_arrSubStates[0], ePlayerEquipment_Large + 1, false );
		FlushInput( 0, 0, 0 );
		return false;
	}

	for( int i = 0; i < ePlayerEquipment_Large; i++ )
	{
		if( m_pCurEquipment[i] )
		{
			auto& inputTable = m_pCurEquipment[i]->m_inputTable;
			for( int j = inputTable.Size() - 1; j >= 0; j-- )
			{
				auto& item = inputTable[j];
				if( CheckInputTableItem( item ) )
				{
					bool b = ExecuteInputtableItem( item, i + 1 );
					ASSERT( b );
					FlushInput( item.strInput.length(), 0, 0 );
					return true;
				}
			}
		}
	}
	for( int i = m_inputTable.Size() - 1; i >= 0; i-- )
	{
		auto& item = m_inputTable[i];
		if( CheckInputTableItem( item ) )
		{
			bool b = ExecuteInputtableItem( item, 0 );
			ASSERT( b );
			FlushInput( item.strInput.length(), 0, 0 );
			return true;
		}
	}

	if( m_parsedInputSequence.size() )
	{
		if( m_parsedInputSequence.back() == -4 )
		{
			FlushInput( 1, 0, 0 );
			if( GetLevel()->FindPickUp( GetMoveTo(), m_nWidth, m_nHeight ) )
			{
				TransitTo( "pick", 0, -1 );
				return true;
			}
			auto pMount = GetLevel()->FindMount();
			if( pMount )
			{
				pMount->Mount( this );
				return true;
			}
			auto pUseable = GetLevel()->FindUseablePawn( GetMoveTo(), m_nCurDir, m_nWidth, m_nHeight );
			if( pUseable )
			{
				auto pUsage = pUseable->GetUsage();
				const char* szUseAction = pUsage->GetUseAction();
				if( szUseAction[0] )
				{
					TransitTo( pUsage->GetUseAction(), 0, -1 );
					m_pCurUsingPawn = pUseable;
					pUsage->BeginUse( this );
				}
				else
				{
					TransitTo( "", 0, -1 );
					pUsage->UseHit( this );
				}
				return true;
			}
			TransitTo( "break", 0, -1 );
			return true;
		}
	}
	FlushInput( 0, 0, 0 );
	return false;
}

bool CPlayer::CheckInputTableItem( SInputTableItem& item )
{
	int32 l = item.strInput.length();
	if( l && item.strInput[l - 1] == '#' )
	{
		if( !m_nChargeKeyDown )
			return false;
		l--;
	}
	if( m_parsedInputSequence.size() < l )
		return false;
	auto nCurDir = m_pControllingPawn ? m_pControllingPawn->GetCurDir() : m_nCurDir;
	for( int j = 0; j < l; j++ )
	{
		auto nInput = m_parsedInputSequence[j + m_parsedInputSequence.size() - l];
		auto chInput = item.strInput[j];
		auto nInput1 = 0;
		if( chInput >= '1' && chInput <= '9' )
		{
			int8 nForward = nCurDir == 0 ? 1 : 4;
			int8 nBack = nCurDir == 0 ? 4 : 1;
			if( chInput == '3' || chInput == '6' || chInput == '9' )
				nInput1 |= nForward;
			if( chInput == '1' || chInput == '4' || chInput == '7' )
				nInput1 |= nBack;
			if( chInput == '7' || chInput == '8' || chInput == '9' )
				nInput1 |= 2;
			if( chInput == '1' || chInput == '2' || chInput == '3' )
				nInput1 |= 8;
		}
		else if( chInput == 'A' )
			nInput1 = -1;
		else if( chInput == 'B' )
			nInput1 = -2;
		else if( chInput == 'C' )
			nInput1 = -3;
		else if( chInput == 'D' )
			nInput1 = -4;
		if( nInput != nInput1 )
			return false;
	}
	return true;
}

bool CPlayer::ExecuteInputtableItem( SInputTableItem& item, int32 nStateSource )
{
	if( item.nStateIndex < 0 )
		return false;

	int8 nChargeKeyDown = 0;
	if( item.strCharge.length() )
	{
		uint8 nKeyDown = m_nChargeKeyDown;
		for( auto n : m_vecInputQueues )
		{
			if( n <= ePlayerInput_Down_Down )
				nKeyDown |= 1 << n;
			else if( n <= ePlayerInput_Down_Up )
				nKeyDown &= ~( 1 << ( n - 4 ) );
			else if( n <= ePlayerInput_D_Down )
				nKeyDown |= 1 << ( n - ePlayerInput_A_Down + 4 );
			else
				nKeyDown &= ~( 1 << ( n - ePlayerInput_A_Up + 4 ) );
		}
		int8 nChargeKeys[] = { '6', '8', '4', '2', 'A', 'B', 'C', 'D' };
		bool bCharge = true;
		for( int i = 0; i < item.strCharge.length() && bCharge; i++ )
		{
			auto c = item.strCharge[i];
			for( int k = 0; k < ELEM_COUNT( nChargeKeys ); k++ )
			{
				if( c == nChargeKeys[k] )
				{
					if( !( nKeyDown & ( 1 << k ) ) )
						break;
					nChargeKeyDown |= 1 << k;
				}
			}
		}
		if( !bCharge )
			nChargeKeyDown = 0;
	}

	if( m_pControllingPawn )
	{
		auto nDir = m_pControllingPawn->GetCurDir();
		if( item.bInverse )
			nDir = 1 - nDir;
		m_pControllingPawn->StateTransit( item.strStateName, item.nStateIndex, nDir );
	}
	else
	{
		if( item.bInverse )
			m_nCurDir = 1 - m_nCurDir;
		if( nStateSource )
		{
			auto pStateSource = GetStateSource( nStateSource );
			if( item.strStateName.length() )
			{
				for( int k = 0; k < pStateSource->m_arrSubStates.Size(); k++ )
				{
					auto& state = pStateSource->m_arrSubStates[k];
					if( state.strName == item.strStateName )
					{
						m_nCurState = k;
						ChangeState( state, nStateSource, false );
						m_nChargeKeyDown = nChargeKeyDown;
						return true;
					}
				}
				if( nStateSource <= ePlayerEquipment_Large )
				{
					for( int k = 0; k < m_arrSubStates.Size(); k++ )
					{
						auto& state = m_arrSubStates[k];
						if( state.strName == item.strStateName )
						{
							CPawn::ChangeState( k );
							m_nChargeKeyDown = nChargeKeyDown;
							return true;
						}
					}
				}
			}
			auto& state = pStateSource->m_arrSubStates[item.nStateIndex];
			m_nCurState = item.nStateIndex;
			ChangeState( state, nStateSource, false );
		}
		else
			TransitTo( item.strStateName, item.nStateIndex, -1 );
	}
	m_nChargeKeyDown = nChargeKeyDown;
	return true;
}

void CPlayer::FlushInput( int32 nMatchLen, int8 nChargeKey, int8 nType )
{
	m_vecInputQueues.resize( 0 );
	if( nType == 2 )
		m_parsedInputSequence.resize( 0 );
	else if( !m_pControllingPawn )
		m_nActionEftFrame = ACTION_EFT_FRAMES;
	GetStage()->GetMasterLevel()->GetMainUI()->OnPlayerAction( m_parsedInputSequence, nMatchLen, nChargeKey, nType );
	GetLevel()->OnPlayerAction( nMatchLen, nType );
}

bool CPlayer::StateCost( int8 nType, int32 nCount )
{
	if( nType == 0 )
	{
		auto pStateSource = GetStateSource( m_nCurStateSource );
		if( !pStateSource )
			return false;
		auto nAmmo = pStateSource->GetAmmo() - nCount;
		if( nAmmo < 0 )
			return false;
		pStateSource->SetAmmo( nAmmo );
		return true;
	}
	else if( nType == 1 )
	{
		if( m_nHp <= nCount )
			return false;
		SetHp( m_nHp - nCount );
		return true;
	}
	return false;
}

CPlayerEquipment* CPlayer::GetStateSource( int8 nType )
{
	if( nType == ePlayerStateSource_Mount )
		return m_pCurMount;
	else if( nType > 0 && nType <= ePlayerEquipment_Count )
		return m_pCurEquipment[nType - 1];
	return NULL;
}

TVector2<int32> CPawnHit::OnHit( SPawnStateEvent& evt )
{
	auto pLevel = GetLevel();
	TVector2<int32> hitOfs( 0, 0 );

	if( m_nHitType == 1 )
	{
		TVector2<int32> hitPoint( 0, 0 );
		for( int i = 0; i < arrGridDesc.Size(); i++ )
		{
			auto& desc = arrGridDesc[i];
			if( desc.nFlag )
			{
				hitPoint = TVector2<int32>( desc.nOfsX, desc.nOfsY );
				break;
			}
		}
		int l = 0;
		TVector2<int32> ofs[3] = { { 2, 0 }, { 1, 1 }, { 1, -1 } };
		for( ; ; l++ )
		{
			auto p = hitPoint + ofs[m_nHitParam[0]] * l;
			if( m_nCurDir )
				p.x = m_nWidth - p.x - 1;
			p = p + m_moveTo;
			auto pGrid = pLevel->GetGrid( p );
			if( !pGrid || !pGrid->bCanEnter )
				break;
			CPawn* pPawn = pGrid->pPawn;
			if( pPawn && pPawn != m_pCreator && !pPawn->IsIgnoreBullet() )
				break;
		}
		hitOfs = ofs[m_nHitParam[0]] * l;
		if( l >= 1 )
		{
			for( int i = 0; i <= l; i++ )
			{
				auto pHit = SafeCast<CPawn>( m_pBeamPrefab[i == 0 ? 0 : ( i == l ? 2 : 1 )]->GetRoot()->CreateInstance() );
				auto p = hitPoint + ofs[m_nHitParam[0]] * i;
				if( m_nCurDir )
					p.x = m_nWidth - p.x - pHit->GetWidth();

				auto pPawnHit = SafeCast<CPawnHit>( pHit );
				if( pPawnHit )
					pPawnHit->SetHitOfs( p );
				auto pos = m_moveTo + p;
				auto dir = m_nCurDir;
				GetLevel()->AddPawn( pHit, pos, dir, !m_pCreator ? this : m_pCreator );
			}
		}
	}

	for( int i = 0; i < arrGridDesc.Size(); i++ )
	{
		auto& desc = arrGridDesc[i];
		if( desc.nHitIndex >= 0 && desc.nHitIndex != evt.nParams[0] )
			continue;
		TVector2<int32> ofs( desc.nOfsX, desc.nOfsY );
		ofs = ofs + hitOfs;
		if( m_nCurDir )
			ofs.x = m_nWidth - ofs.x - 1;
		auto p = ofs + m_moveTo;
		auto pGrid = pLevel->GetGrid( p );
		if( !pGrid )
			continue;
		auto pPawn = pGrid->pPawn;
		if( !pPawn || pPawn.GetPtr() == m_pCreator || !pPawn->CanBeHit() )
		{
			if( desc.nDamageType )
				pGrid->nMissBashEft = CGlobalCfg::Inst().lvIndicatorData.vecMissParams.size();
			else
				pGrid->nMissEft = CGlobalCfg::Inst().lvIndicatorData.vecMissParams.size();
			continue;
		}
		if( desc.nDamage )
		{
			auto n = pPawn->Damage( desc.nDamage, desc.nDamageType, hitOfs + m_hitOfs );
			if( !n )
				pGrid->nHitBlockedEft = CGlobalCfg::Inst().lvIndicatorData.vecHitBlockedParams.size();
			else if( desc.nDamageType )
				pGrid->nHitBashEft = CGlobalCfg::Inst().lvIndicatorData.vecHitParams.size();
			else
				pGrid->nHitEft = CGlobalCfg::Inst().lvIndicatorData.vecHitParams.size();
			if( pPawn->GetDamageEft() )
			{
				auto pEft = SafeCast<CPawn>( pPawn->GetDamageEft()->GetRoot()->CreateInstance() );
				CPawn* pCreator = m_pCreator;
				if( !pCreator )
					pCreator = this;
				auto pos = p;
				if( m_nCurDir )
					pos.x -= pEft->GetWidth() - 1;
				pLevel->AddPawn( pEft, pos, m_nCurDir, pCreator );
			}
		}
	}
	return hitOfs;
}

void CPickUp::OnPreview()
{
	CPawnHit::OnPreview();
	if( m_pEquipment )
		m_pEquipment->OnPreview();
}

void CPickUp::Init()
{
	CPawnHit::Init();
	if( m_pEquipment && m_pEquipment->GetParentEntity() == this )
	{
		m_pEquipment->SetParentEntity( NULL );
		m_pEquipment->Init();
	}
}

bool CPickUp::IsPickUpReady()
{
	return GetCurState().strName == "stand";
}

void CPickUp::PickUp( CPlayer* pPlayer )
{
	if( m_pEquipment )
	{
		m_pEquipment->PrePickedUp( this );
		pPlayer->Equip( m_pEquipment );
		m_pEquipment = NULL;
	}
	if( GetLevel() )
	{
		if( m_strScript.length() )
			CLuaMgr::Inst().Run( m_strScript );
		OnKilled();
		m_strKillScript = "";
		GetLevel()->RemovePawn( this );
	}
}

int32 CPickUp::GetDefaultState()
{
	if( !m_bDropped )
		return GetStateIndexByName( "stand" );
	if( m_nDropState >= 0 && m_nDropState < m_arrSubStates.Size() )
		return m_nDropState;
	return CPawnHit::GetDefaultState();
}


void RegisterGameClasses_BasicElems()
{
	REGISTER_ENUM_BEGIN( EPawnStateEventType )
		REGISTER_ENUM_ITEM( ePawnStateEventType_CheckAction )
		REGISTER_ENUM_ITEM( ePawnStateEventType_MoveBegin )
		REGISTER_ENUM_ITEM( ePawnStateEventType_MoveEnd )
		REGISTER_ENUM_ITEM( ePawnStateEventType_Hit )
		REGISTER_ENUM_ITEM( ePawnStateEventType_Death )
		REGISTER_ENUM_ITEM( ePawnStateEventType_Transform )
		REGISTER_ENUM_ITEM( ePawnStateEventType_PickUp )
		REGISTER_ENUM_ITEM( ePawnStateEventType_Drop )
		REGISTER_ENUM_ITEM( ePawnStateEventType_Cost )
		REGISTER_ENUM_ITEM( ePawnStateEventType_UnMount )
		REGISTER_ENUM_ITEM( ePawnStateEventType_SetZ )
		REGISTER_ENUM_ITEM( ePawnStateEventType_Sound )
		REGISTER_ENUM_ITEM( ePawnStateEventType_JumpTo )
		REGISTER_ENUM_ITEM( ePawnStateEventType_Script )
	REGISTER_ENUM_END()

	REGISTER_CLASS_BEGIN( SPawnStateEvent )
		REGISTER_MEMBER( eType )
		REGISTER_MEMBER( nTick )
		REGISTER_MEMBER( nParams )
		REGISTER_MEMBER_BEGIN( strParam )
			MEMBER_ARG( text, 1 )
		REGISTER_MEMBER_END()
	REGISTER_CLASS_END()
		
	REGISTER_ENUM_BEGIN( EPawnStateTransitCondition )
		REGISTER_ENUM_ITEM( ePawnStateTransitCondition_Finish )
		REGISTER_ENUM_ITEM( ePawnStateTransitCondition_Break )
		REGISTER_ENUM_ITEM( ePawnStateTransitCondition_Hit )
		REGISTER_ENUM_ITEM( ePawnStateTransitCondition_Killed )
	REGISTER_ENUM_END()

	REGISTER_CLASS_BEGIN( SPawnStateTransit )
		REGISTER_MEMBER( strToName )
		REGISTER_MEMBER( nTo )
		REGISTER_MEMBER( bInverse )
		REGISTER_MEMBER( eCondition )
		REGISTER_MEMBER( strCondition )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( SPawnStateTransit1 )
		REGISTER_MEMBER( strToName )
		REGISTER_MEMBER( nTo )
		REGISTER_MEMBER( bInverse )
		REGISTER_MEMBER( arrStrExclude )
		REGISTER_MEMBER( arrExclude )
		REGISTER_MEMBER( eCondition )
		REGISTER_MEMBER( strCondition )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( SPawnState )
		REGISTER_MEMBER( strName )
		REGISTER_MEMBER( nForm )
		REGISTER_MEMBER( nTotalTicks )
		REGISTER_MEMBER( nTicksPerFrame )
		REGISTER_MEMBER( nImgExtLeft )
		REGISTER_MEMBER( nImgExtRight )
		REGISTER_MEMBER( nImgExtTop )
		REGISTER_MEMBER( nImgExtBottom )
		REGISTER_MEMBER( nImgTexBeginX )
		REGISTER_MEMBER( nImgTexBeginY )
		REGISTER_MEMBER( nImgTexCols )
		REGISTER_MEMBER( nImgTexCount )
		REGISTER_MEMBER( arrEvts )
		REGISTER_MEMBER( arrTransits )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( SPawnHitSpawnDesc )
		REGISTER_MEMBER( pHit )
		REGISTER_MEMBER( nOfsX )
		REGISTER_MEMBER( nOfsY )
		REGISTER_MEMBER( nDir )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( SPawnForm )
		REGISTER_MEMBER( strName )
		REGISTER_MEMBER( nWidth )
		REGISTER_MEMBER( nHeight )
		REGISTER_MEMBER( nDefaultState )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CPawnAI )
		REGISTER_BASE_CLASS( CEntity )
		DEFINE_LUA_REF_OBJECT()
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CPawnUsage )
		REGISTER_BASE_CLASS( CEntity )
		REGISTER_MEMBER( m_strUseAction )
	REGISTER_CLASS_END()

	REGISTER_INTERFACE_BEGIN( ISignalObj )
	REGISTER_INTERFACE_END()

	REGISTER_CLASS_BEGIN( CLevelSpawnHelper )
		REGISTER_BASE_CLASS( CEntity )
		REGISTER_MEMBER( m_nSpawnType )
		REGISTER_MEMBER( m_nDataType )
		REGISTER_MEMBER( m_bSpawnDeath )
		REGISTER_MEMBER( m_nSpawnParam )
		REGISTER_MEMBER_BEGIN( m_strSpawnCondition )
			MEMBER_ARG( text, 1 )
		REGISTER_MEMBER_END()
		REGISTER_MEMBER( m_strDeathKey )
		REGISTER_MEMBER( m_nDeathState )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( SInputTableItem )
		REGISTER_MEMBER( strInput )
		REGISTER_MEMBER( strStateName )
		REGISTER_MEMBER( nStateIndex )
		REGISTER_MEMBER( strCharge )
		REGISTER_MEMBER( bInverse )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( SStateInputTableItem )
		REGISTER_MEMBER( arrStates )
		REGISTER_MEMBER( input )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CPawn )
		REGISTER_BASE_CLASS( CEntity )
		REGISTER_BASE_CLASS( ISignalObj )
		REGISTER_MEMBER( m_bIsEnemy )
		REGISTER_MEMBER( m_bIgnoreBullet )
		REGISTER_MEMBER( m_bForceHit )
		REGISTER_MEMBER( m_bIgnoreBlockedExit )
		REGISTER_MEMBER( m_bHideInEditor )
		REGISTER_MEMBER( m_nInitDir )
		REGISTER_MEMBER( m_nArmorType )
		REGISTER_MEMBER( m_bUseInitState )
		REGISTER_MEMBER( m_bUseDefaultState )
		REGISTER_MEMBER( m_nInitState )
		REGISTER_MEMBER( m_nDefaultState )
		REGISTER_MEMBER( m_nWidth )
		REGISTER_MEMBER( m_nHeight )
		REGISTER_MEMBER( m_nMaxHp )
		REGISTER_MEMBER( m_arrForms )
		REGISTER_MEMBER( m_arrSubStates )
		REGISTER_MEMBER( m_arrHitSpawnDesc )
		REGISTER_MEMBER( m_arrCommonStateTransits )
		REGISTER_MEMBER( m_origRect )
		REGISTER_MEMBER( m_origTexRect )
		REGISTER_MEMBER( m_nRenderOrder )
		REGISTER_MEMBER_BEGIN( m_strKillScript )
			MEMBER_ARG( text, 1 )
		REGISTER_MEMBER_END()
		REGISTER_MEMBER( m_pDamageEft )
		REGISTER_MEMBER_TAGGED_PTR( m_pAI, ai )
		REGISTER_MEMBER_TAGGED_PTR( m_pUsage, usage )
		REGISTER_MEMBER_TAGGED_PTR( m_pHpBar, hpbar )

		DEFINE_LUA_REF_OBJECT()
		REGISTER_LUA_CFUNCTION( GetLevel )
		REGISTER_LUA_CFUNCTION( GetPosX )
		REGISTER_LUA_CFUNCTION( GetPosY )
		REGISTER_LUA_CFUNCTION( GetToX )
		REGISTER_LUA_CFUNCTION( GetToY )
		REGISTER_LUA_CFUNCTION( GetCurDir )
		REGISTER_LUA_CFUNCTION( GetHp )
		REGISTER_LUA_CFUNCTION( GetMaxHp )
		REGISTER_LUA_CFUNCTION( SetHp )
		REGISTER_LUA_CFUNCTION( SetForceHide )
		REGISTER_LUA_CFUNCTION( GetDamageType )
		REGISTER_LUA_CFUNCTION( GetDamageOfsDir )
		REGISTER_LUA_CFUNCTION( PlayState )
		REGISTER_LUA_CFUNCTION( PlayStateTurnBack )
		REGISTER_LUA_CFUNCTION( PlayStateSetDir )
		REGISTER_LUA_CFUNCTION( ChangeAI )
		REGISTER_LUA_CFUNCTION( GetCurStateName )
		REGISTER_LUA_CFUNCTION( RegisterSignalScript )
		REGISTER_LUA_CFUNCTION( RegisterKilledScript )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CPlayerEquipment )
		REGISTER_BASE_CLASS( CEntity )
		REGISTER_MEMBER( m_strEquipmentName )
		REGISTER_MEMBER( m_nEquipType )
		REGISTER_MEMBER( m_nAmmo )
		REGISTER_MEMBER( m_nMaxAmmo )
		REGISTER_MEMBER( m_nIcon )
		REGISTER_MEMBER( m_nAmmoIconWidth )
		REGISTER_MEMBER( m_arrSubStates )
		REGISTER_MEMBER( m_arrHitSpawnDesc )
		REGISTER_MEMBER( m_arrCommonStateTransits )
		REGISTER_MEMBER( m_inputTable )
		REGISTER_MEMBER( m_stateInputTable )
		REGISTER_MEMBER( m_origRect )
		REGISTER_MEMBER( m_origTexRect )
		REGISTER_MEMBER_TAGGED_PTR( m_pEft[0], 0 )
		REGISTER_MEMBER_TAGGED_PTR( m_pEft[1], 1 )
		DEFINE_LUA_REF_OBJECT()
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CPlayerMount )
		REGISTER_BASE_CLASS( CEntity )
		REGISTER_MEMBER( m_bDisabled )
		REGISTER_MEMBER( m_bAnimPlayerOriented )
		REGISTER_MEMBER( m_bShowPawnOnMount )
		REGISTER_MEMBER( m_nEnterDir )
		REGISTER_MEMBER( m_nOfsX )
		REGISTER_MEMBER( m_nOfsY )
		REGISTER_MEMBER( m_strEntryState )
		REGISTER_MEMBER( m_strCostEquipment )
		REGISTER_MEMBER( m_nNeedStateIndex )
		REGISTER_MEMBER_TAGGED_PTR( m_pEquipment, equipment )
		DEFINE_LUA_REF_OBJECT()
		REGISTER_LUA_CFUNCTION( SetEnabled )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CPlayer )
		REGISTER_BASE_CLASS( CPawn )
		REGISTER_MEMBER( m_inputTable )
		REGISTER_MEMBER( m_stateInputTable )
		REGISTER_MEMBER( m_actionEftOfs )
		REGISTER_MEMBER( m_actionEftParam )
		REGISTER_MEMBER_TAGGED_PTR( m_pEft[0], 0 )
		REGISTER_MEMBER_TAGGED_PTR( m_pEft[1], 1 )
		REGISTER_MEMBER_TAGGED_PTR( m_pDefaultEquipment, default_weapon )
		DEFINE_LUA_REF_OBJECT()
		REGISTER_LUA_CFUNCTION( Reset )
		REGISTER_LUA_CFUNCTION( ForceUnMount )
		REGISTER_LUA_CFUNCTION( EnableDefaultEquipment )
		REGISTER_LUA_CFUNCTION( RestoreAmmo )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( SHitGridDesc )
		REGISTER_MEMBER( nHitIndex )
		REGISTER_MEMBER( nOfsX )
		REGISTER_MEMBER( nOfsY )
		REGISTER_MEMBER( nDamage )
		REGISTER_MEMBER( nDamageType )
		REGISTER_MEMBER( nFlag )
	REGISTER_CLASS_END()
		
	REGISTER_CLASS_BEGIN( CPawnHit )
		REGISTER_BASE_CLASS( CPawn )
		REGISTER_MEMBER( m_nHitType )
		REGISTER_MEMBER( m_nHitParam )
		REGISTER_MEMBER( arrGridDesc )
		REGISTER_MEMBER( m_pBeamPrefab )
		DEFINE_LUA_REF_OBJECT()
	REGISTER_CLASS_END()
		
	REGISTER_CLASS_BEGIN( CPickUp )
		REGISTER_BASE_CLASS( CPawnHit )
		REGISTER_MEMBER_BEGIN( m_strScript )
			MEMBER_ARG( text, 1 )
		REGISTER_MEMBER_END()
		REGISTER_MEMBER_TAGGED_PTR( m_pEquipment, equipment )
		DEFINE_LUA_REF_OBJECT()
	REGISTER_CLASS_END()
}