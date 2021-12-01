#include "stdafx.h"
#include "BasicElems.h"
#include "Stage.h"
#include "MyLevel.h"
#include "Render/Image2D.h"
#include "MyGame.h"
#include "Common/Rand.h"
#include "GlobalCfg.h"
#include "CommonUtils.h"
#include "Entities/UtilEntities.h"
#include <algorithm>

void CLevelSpawnHelper::OnPreview()
{
	auto pPawn = SafeCast<CPawn>( GetRenderObject() );
	if( pPawn )
	{
		pPawn->m_pos = pPawn->m_moveTo = TVector2<int32>( 0, 0 );
		pPawn->m_nCurDir = pPawn->m_nInitDir;
		pPawn->OnPreview();
	}
}

int32 SInputTableItem::SInputItr::Next()
{
	sz += l;
	for( bool b = true; b; )
	{
		while( *sz == '|' || *sz == '?' )
			sz++;
		b = false;
		for( int i = 0; sz[i] != '|' && sz[i]; i++ )
		{
			if( sz[i] == '?' )
			{
				szCondition = sz;
				lCondition = i;
				sz = sz + lCondition;
				b = true;
				break;
			}
		}
	}

	for( l = 0; sz[l] != '|' && sz[l]; l++ );
	return l;
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
		m_bTracerEffectDisabled = true;
	}
	Update0();
	for( auto pChild = Get_TransformChild(); pChild; pChild = pChild->NextTransformChild() )
	{
		auto pMount = SafeCast<CPlayerMount>( pChild );
		if( pMount )
			pMount->OnPreview();
	}
	auto pTracerEffect = SafeCast<CTracerEffect>( GetRenderObject() );
	if( pTracerEffect )
		pTracerEffect->OnPreview();
	if( m_pAI )
		m_pAI->OnPreview();
}

void CPawn::LoadData( IBufReader& buf )
{
	int32 nVersionData = 0;
	buf.Read( nVersionData );
	buf.Read( m_pos );
	buf.Read( m_nCurDir );
	buf.Read( m_nHp );
	buf.Read( m_bUseInitState );
	buf.Read( m_bUseDefaultState );
	buf.Read( m_nInitState );
	buf.Read( m_nDefaultState );
}

void CPawn::SaveData( CBufFile& buf )
{
	int32 nVersionData = 0;
	buf.Write( nVersionData );
	buf.Write( m_pos );
	buf.Write( m_nCurDir );
	buf.Write( m_nHp );
	buf.Write( m_bUseInitState );
	buf.Write( m_bUseDefaultState );
	buf.Write( m_nInitState );
	buf.Write( m_nDefaultState );
}

void CPawn::Init()
{
	if( m_pHpBar && GetLevel() && GetLevel()->IsSnapShot() )
	{
		m_pHpBar->RemoveThis();
		m_pHpBar = NULL;
	}
	if( m_pHpBar && m_hpBarOrigRect.width == 0 )
		m_hpBarOrigRect = static_cast<CImage2D*>( m_pHpBar.GetPtr() )->GetElem().rect;
	if( m_pAI )
		m_pAI->PreInit();
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
	bVisible = !m_bForceHide;
	if( !m_bMountHide )
	{
		if( m_bCurStateDirty )
		{
			m_bCurStateDirty = false;
			memset( m_nCurStateSpecialState, 0, sizeof( m_nCurStateSpecialState ) );
		}
		if( m_arrSubStates.Size() )
		{
			while( 1 )
			{
				bool bInterrupted = IsCurStateInterrupted();
				int8 nBreakFlag = 0;
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
								m_nCurStateCheckAction = evt.nParams[0];
						}
						else if( evt.eType == ePawnStateEventType_MoveBegin )
						{
							bool bSuccess = GetLevel()->PawnMoveTo( this, TVector2<int32>( evt.nParams[0] * ( m_nCurDir ? -1 : 1 ), evt.nParams[1] ), evt.nParams[2], evt.nParams[3] >> 1 );
							if( !bSuccess )
							{
								bInterrupted = true;
								nBreakFlag = evt.nParams[3] & 1;
								break;
							}
						}
						else if( evt.eType == ePawnStateEventType_MoveEnd )
						{
							if( evt.nParams[0] == 0 )
								GetLevel()->PawnMoveEnd( this );
							else if( evt.nParams[0] == 1 )
								GetLevel()->PawnMoveBreak( this, true );
						}
						else if( evt.eType == ePawnStateEventType_Hit )
						{
							if( !HandleHit( evt ) )
							{
								bInterrupted = true;
								break;
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
								nBreakFlag = evt.nParams[3];
								break;
							}
						}
						else if( evt.eType == ePawnStateEventType_PickUp )
						{
							if( SafeCast<CPlayer>( this )->TryPickUp( evt.nParams[0] ) == -1 )
							{
								bJumpTo = true;
								break;
							}
						}
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
							SafeCast<CPlayer>( this )->UnMount( evt.strParam, evt.nParams[0], evt.nParams[1] );
							if( !evt.nParams[2] )
							{
								bInterrupted = true;
								nBreakFlag = 1;
								break;
							}
						}
						else if( evt.eType == ePawnStateEventType_SetZ )
							m_nRenderOrder = evt.nParams[0];
						else if( evt.eType == ePawnStateEventType_Sound )
						{
							PlaySoundEffect( evt.strParam );
							if( evt.nParams[0] )
							{
								auto d = TVector2<int32>( evt.nParams[1], evt.nParams[2] );
								if( m_nCurDir )
									d.x = 2 - m_nWidth - d.x;
								GetLevel()->Alert( this, m_moveTo + d );
							}
						}
						else if( evt.eType == ePawnStateEventType_JumpTo )
						{
							if( TransitTo( evt.strParam, evt.nParams[0], ePawnStateTransitReason_JumpTo ) )
							{
								bJumpTo = true;
								break;
							}
						}
						else if( evt.eType == ePawnStateEventType_SpecialState )
						{
							ASSERT( evt.nParams[0] < ELEM_COUNT( m_nCurStateSpecialState ) );
							m_nCurStateSpecialState[evt.nParams[0]] += evt.nParams[1];
						}
						else if( evt.eType == ePawnStateEventType_Interaction )
						{
							if( !IsActionPreview() )
							{
								CPawn* pTarget = this;
								auto pPlayer = SafeCast<CPlayer>( this );
								if( pPlayer && pPlayer->GetCurMountingPawn() )
									pTarget = pPlayer->GetCurMountingPawn();
								GetStage()->GetMasterLevel()->ShowInteractionUI( pTarget, evt.strParam );
							}
						}
						else if( evt.eType == ePawnStateEventType_Script )
						{
							if( !IsActionPreview() )
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
				}

				if( bJumpTo )
				{
					if( m_bCurStateDirty )
					{
						m_bCurStateDirty = false;
						memset( m_nCurStateSpecialState, 0, sizeof( m_nCurStateSpecialState ) );
					}
					continue;
				}
				if( bInterrupted )
					CheckStateTransits( GetDefaultState(), nBreakFlag );
				else if( m_nCurStateCheckAction >= 0 )
				{
					if( GetLevel()->GetPlayer()->GetControllingPawn() == this )
					{
						if( !GetLevel()->GetPlayer()->ControllingPawnCheckAction( m_nCurStateCheckAction ) )
							break;
					}
					else
					{
						if( !CheckAction( m_nCurStateCheckAction ) )
							break;
					}
				}
				else
					break;
			}
		}
	}
	if( m_pAI )
		m_pAI->OnUpdate();
	Update0();
	if( m_pUsage )
		m_pUsage->Update();
}

void CPawn::Update1()
{
	if( !m_arrSubStates.Size() || m_bMountHide )
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
	{
		m_bCurStateDirty = true;
		m_nCurStateTick = 0;
	}
	int32 nNewState = -1;
	bool bCanFinish = CheckCanFinish();
	if( bFinished && bCanFinish )
		nNewState = GetDefaultState();
	CheckStateTransits1( nNewState, ( bFinished || !curState.nTotalTicks ) && bCanFinish );
	if( m_pAI )
		m_pAI->OnUpdate1();
	m_bDamaged = false;
}

bool CPawn::IsActionPreview()
{
	return GetLevel()->IsActionPreview();
}

void CPawn::OnRemovedFromLevel()
{
	if( m_pAI )
		m_pAI->OnRemovedFromLevel();
}

void CPawn::OnLevelEnd()
{
	if( m_pAI )
		m_pAI->OnLevelEnd();
}

void CPawn::OnLevelSave()
{
	if( m_pSpawnHelper )
	{
		if( m_pSpawnHelper->m_nDataType >= 2 )
		{
			auto& mapDeadPawn = GetStage()->GetMasterLevel()->GetCurLevelData().mapDataDeadPawn;
			auto& data = mapDeadPawn[m_pSpawnHelper->GetName().c_str()];
			data.p = GetPos();
			data.p1 = GetMoveTo();
			data.nDir = m_nCurDir;
			data.nState = m_nCurState;
			data.nStateTick = m_nCurState == m_pSpawnHelper->m_nDeathState ? 0 : m_nCurStateTick;
			data.nSpawnIndex = m_pSpawnHelper->m_nSpawnIndex;
			data.bIsAlive = !IsKilled();
		}
	}
}

bool CPawn::OnPlayerTryToLeave()
{
	if( m_pAI )
		return m_pAI->OnPlayerTryToLeave();
	return true;
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
		{
			m_nCurStateTick++;
			if( m_nCurStateTick == curState.nTotalTicks )
				CheckStateTransits1( -1, true );
		}
	}
}

int32 CPawn::Damage( int32 nDamage, int8 nDamageType, TVector2<int32> damageOfs, CPawn* pSource )
{
	if( m_bIsEnemy )
		GetLevel()->Alert1();
	if( m_pAI )
	{
		if( m_pAI->Damage( nDamage, nDamageType, damageOfs, pSource ) )
			return nDamage;
	}
	if( nDamageType < 0 )
	{
		if( m_nArmorType != nDamageType )
			return 0;
	}
	else if( m_nArmorType > nDamageType )
		return 0;
	m_nHp = Max( 0, m_nHp - nDamage );
	SetDamaged( nDamageType, damageOfs.x, damageOfs.y );
	if( nDamage && m_nHp <= 0 )
	{
		if( !m_pAI || m_pAI->PreKill() )
			OnKilled();
	}
	return nDamage;
}

void CPawn::SetDamaged( int8 nType, int32 ofsX, int32 ofsY )
{
	if( !m_bDamaged || m_nDamageType < nType )
	{
		m_nDamageType = nType;
		m_damageOfs = TVector2<int32>( ofsX, ofsY );
	}
	m_bDamaged = true;
}

void CPawn::Block( TVector2<int32> damageOfs )
{
	if( m_pAI )
		m_pAI->Block( damageOfs );
}

CMyLevel* CPawn::GetLevel()
{
	if( !GetParentEntity() )
		return NULL;
	return SafeCast<CMyLevel>( GetParentEntity()->GetParentEntity() );
}

TVector2<int32> CPawn::GetCurStateDest( int32 nTick )
{
	auto p = m_curStateOrigPos;
	auto p1 = p;
	auto& state = GetCurState();
	if( !state.nTotalTicks )
		return p;
	if( nTick <= 0 )
		nTick = state.nTotalTicks;

	struct SItem
	{
		int8 nType;
		int32 nTick;
		TVector2<int32> ofs;
	};
	static vector<SItem> vecItems;
	for( int i = 0; i < state.arrEvts.Size(); i++ )
	{
		auto& evt = state.arrEvts[i];
		if( evt.nTick >= nTick )
			continue;
		if( evt.eType == ePawnStateEventType_MoveBegin )
		{
			SItem item = { 0, evt.nTick, { evt.nParams[0] * ( m_nCurDir ? -1 : 1 ), evt.nParams[1] } };
			vecItems.push_back( item );
		}
		else if( evt.eType == ePawnStateEventType_MoveEnd )
		{
			SItem item = { evt.nParams[0] == 0 ? 1 : 2, evt.nTick, { evt.nParams[0] * ( m_nCurDir ? -1 : 1 ), evt.nParams[1] } };
			vecItems.push_back( item );
		}
	}
	if( !vecItems.size() )
		return p;
	std::stable_sort( vecItems.begin(), vecItems.end(), [] ( const SItem& a, const SItem& b )
	{
		return a.nTick < b.nTick;
	} );
	for( auto& item : vecItems )
	{
		if( item.nType == 0 )
			p1 = p1 + item.ofs;
		else if( item.nType == 1 )
			p = p1;
		else
			p1 = p;
	}
	vecItems.resize( 0 );
	return p;
}

bool CPawn::IsAutoBlockStage()
{
	if( !m_bIsEnemy )
		return false;
	if( m_pAI && m_pAI->IsIgnoreBlockStage() )
		return false;
	return true;
}

bool CPawn::HasTag( const char* sz )
{
	int32 l = strlen( sz );
	for( int i = 0; i < m_arrTags.Size(); i++ )
	{
		auto str = m_arrTags[i].c_str();
		auto c = strchr( m_arrTags[i].c_str(), '=' );
		if( c )
		{
			if( l == c - str && strncmp( str, sz, l ) == 0 )
				return true;
		}
		else
		{
			if( m_arrTags[i] == sz )
				return true;
		}
	}
	return false;
}

const char* CPawn::GetTag( const char* sz )
{
	int32 l = strlen( sz );
	for( int i = 0; i < m_arrTags.Size(); i++ )
	{
		auto str = m_arrTags[i].c_str();
		auto c = strchr( str, '=' );
		if( c )
		{
			if( l == c - str && strncmp( str, sz, l ) == 0 )
				return c + 1;
		}
	}
	return "";
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

int32 CPawn::Signal( int32 i )
{
	m_trigger.Trigger( 0, (void*)i );
	if( m_pAI )
		return m_pAI->Signal( i );
	return 0;
}

int32 CPawn::CheckHit( const TVector2<int32>& p, int8 nDamageType )
{
	if( !CanBeHit( nDamageType ) )
		return false;
	auto nHitWidth = m_nHitWidth ? m_nHitWidth : m_nWidth;
	auto nHitHeight = m_nHitHeight ? m_nHitHeight : m_nHeight;
	if( IsSpecialState( eSpecialState_Block ) )
		nHitWidth = nHitHeight = 1;
	TRectangle<int32> r[4] = { { m_pos.x, m_pos.y, m_nWidth, m_nHeight },
	{ m_moveTo.x, m_moveTo.y, m_nWidth, m_nHeight },
	{ m_pos.x, m_pos.y, nHitWidth, nHitHeight },
	{ m_moveTo.x, m_moveTo.y, nHitWidth, nHitHeight } };
	if( m_nCurDir )
	{
		r[2].x = r[0].GetRight() - r[2].width;
		r[3].x = r[1].GetRight() - r[3].width;
	}
	bool b[4];
	for( int i = 0; i < 4; i++ )
		b[i] = p.x >= r[i].x && p.y >= r[i].y && p.x < r[i].GetRight() && p.y < r[i].GetBottom();
	if( !( b[2] && b[3] || b[2] && !b[1] || b[3] && !b[0] ) )
		return -1;
	return 1;
}

bool CPawn::CanBeHit( int8 nDamageType )
{
	if( m_bForceHit )
		return true;
	if( m_nHp <= 0 && !m_bIsEnemy )
	{
		if( nDamageType >= 0 || nDamageType != m_nArmorType )
			return false;
	}
	return true;
}

void CPawn::SetMounted( bool b, bool bMountHide, int8 nMoveType )
{
	m_bMounted = b;
	m_bMountHide = b ? bMountHide : false;
	if( !b )
	{
		if( nMoveType )
			m_pos = m_moveTo;
		else
			m_moveTo = m_pos;
	}
	if( m_pAI )
		m_pAI->OnSetMounted( b );
}

int8 CPawn::GetDamageOfsDir()
{
	return GetDamageOfsDir1( m_damageOfs.x, m_damageOfs.y );
}

int8 CPawn::GetDamageOfsDir1( int32 x, int32 y )
{
	if( !x && !y )
		return -1;
	int32 x1 = abs( x );
	int8 n = 0;
	if( y >= x1 )
		n = 1;
	else if( y <= -x1 )
		n = 2;
	int8 nDir = m_nCurDir;
	if( x > 0 )
		nDir = 0;
	else if( x < 0 )
		nDir = 1;
	return n * 2 + nDir;
}

bool CPawn::HandleHit( SPawnStateEvent& evt )
{
	auto hitOfs = OnHit( evt );
	if( evt.nParams[0] == 1 )
	{
		if( !GetLevel()->SpawnPreset1( evt.strParam, m_moveTo.x, m_moveTo.y, m_nCurDir ) )
			return false;
	}
	else
	{
		auto pHitDesc = GetHitSpawn( evt.nParams[3] );
		if( pHitDesc )
		{
			CReference<CPawn> pHit = SafeCast<CPawn>( pHitDesc->pHit->GetRoot()->CreateInstance() );
			pHit->strCreatedFrom = pHitDesc->pHit;
			if( pHitDesc->strInitState.length() )
			{
				auto nIndex = pHit->GetStateIndexByName( pHitDesc->strInitState );
				if( nIndex >= 0 )
					pHit->SetInitState( nIndex );
			}
			TVector2<int32> ofs( pHitDesc->nOfsX, pHitDesc->nOfsY );
			ofs = ofs + hitOfs;
			if( m_nCurDir )
			{
				ofs.x = m_nWidth - ( ofs.x + pHit->m_nWidth );
			}
			auto pPawnHit = SafeCast<CPawnHit>( pHit.GetPtr() );
			if( pPawnHit )
				pPawnHit->SetHitOfs( ofs );
			auto pos = m_moveTo + ofs;
			auto dir = pHitDesc->nDir ? 1 - m_nCurDir : m_nCurDir;
			if( GetLevel()->AddPawn( pHit, pos, dir, this ) )
				pHit->Signal( evt.nParams[2] );
			else
				return false;
		}
	}
	return true;
}

TVector2<int32> CPawn::HandleStealthDetect()
{
	if( m_pAI )
		return m_pAI->HandleStealthDetect();
	return TVector2<int32>( -1, -1 );
}

void CPawn::HandleAlert( CPawn* pTrigger, const TVector2<int32>& p )
{
	if( m_pAI )
		m_pAI->HandleAlert( pTrigger, p );
}

bool CPawn::HasStateTag( const char* sz )
{
	if( !m_arrSubStates.Size() )
		return false;
	auto state = GetCurState();
	for( int i = 0; i < state.arrTags.Size(); i++ )
	{
		if( state.arrTags[i] == sz )
			return true;
	}
	return false;
}

CVector2 CPawn::GetHpBarOfs()
{
	return CVector2( m_moveTo.x, m_moveTo.y ) * LEVEL_GRID_SIZE - m_curStateBeginPos;
}

void CPawn::ResetState()
{
	ChangeState( GetCurState(), 0, false );
}

bool CPawn::PlayState( const char* sz, int8 nType )
{
	if( TransitTo( sz, -1, -1 ) )
	{
		if( !GetLevel()->IsBegin() )
			Update0();
		else if( nType )
			Update();
		return true;
	}
	return false;
}

bool CPawn::PlayStateTurnBack( const char* sz, int8 nType )
{
	auto nDir0 = m_nCurDir;
	m_nCurDir = !m_nCurDir;
	if( !TransitTo( sz, -1, -1 ) )
	{
		m_nCurDir = nDir0;
		return false;
	}
	if( !GetLevel()->IsBegin() )
		Update0();
	else if( nType )
		Update();
	return true;
}

bool CPawn::PlayStateSetDir( const char* sz, int8 nDir, int8 nType )
{
	auto nDir0 = m_nCurDir;
	m_nCurDir = nDir;
	if( !TransitTo( sz, -1, -1 ) )
	{
		m_nCurDir = nDir0;
		return false;
	}
	if( !GetLevel()->IsBegin() )
		Update0();
	else if( nType )
		Update();
	return true;
}

bool CPawn::PlayStateForceMove( const char* sz, int32 x, int32 y, int8 nDir, int8 nType )
{
	GetLevel()->PawnMoveBreak( this );
	if( !GetLevel()->PawnMoveTo( this, TVector2<int32>( x, y ) - GetPos() ) )
		return false;
	GetLevel()->PawnMoveEnd( this );
	m_nCurDir = nDir;
	if( !TransitTo( sz, -1, -1 ) )
	{
		m_curStateBeginPos = CVector2( m_pos.x, m_pos.y ) * LEVEL_GRID_SIZE;
		m_curStateOrigPos = m_pos;
	}
	if( !GetLevel()->IsBegin() )
		Update0();
	else if( nType )
		Update();
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

void CPawn::CreateIconData( CPrefabNode* pNode, const char* szCondition0, TArray<SMapIconData>& arrData ) const
{
	if( !m_nMapIconType )
		return;
	if( m_nMapIconType >= 1 )
	{
		int8 nDir = pNode->GetPatchedNode()->GetStaticDataSafe<CPawn>()->GetInitDir();
		CVector2 p( floor( pNode->x / LEVEL_GRID_SIZE_X + 0.5f ), floor( pNode->y / LEVEL_GRID_SIZE_Y + 0.5f ) );
		if( nDir )
			p.x += m_nWidth - 2;
		arrData.Resize( arrData.Size() + 1 );
		auto& item = arrData[arrData.Size() - 1];
		item.ofs = p;
		item.nDir = nDir;
		item.nTexX = m_nMapIconTexX[0];
		item.nTexY = m_nMapIconTexY[0];
		item.bKeepSize = m_bMapIconKeepSize[0];
		item.strTag = m_strMapIconTag[0];
		auto pSpawnData = pNode->GetStaticDataSafe<CLevelSpawnHelper>();
		if( szCondition0[0] )
		{
			item.arrFilter.Resize( item.arrFilter.Size() + 1 );
			item.arrFilter[item.arrFilter.Size() - 1] = szCondition0;
		}
		if( pSpawnData && pSpawnData->m_strSpawnCondition.length() )
		{
			item.arrFilter.Resize( item.arrFilter.Size() + 1 );
			item.arrFilter[item.arrFilter.Size() - 1] = pSpawnData->m_strSpawnCondition;
		}
		if( pSpawnData &&( pSpawnData->m_strDeathKey.length() || pSpawnData->m_nDataType ) )
		{
			if( pSpawnData->m_nDataType )
			{
				string str = "&dead&";
				str += pNode->GetName().c_str();
				item.strCondition = str.c_str();
			}
			else
				item.strCondition = pSpawnData->m_strDeathKey;
			item.nConditionValue = 0;
			if( m_nMapIconType >= 2 )
			{
				arrData.Resize( arrData.Size() + 1 );
				auto& item0 = arrData[arrData.Size() - 2];
				auto& item1 = arrData[arrData.Size() - 1];
				item1.ofs = p;
				item1.nDir = nDir;
				item1.nTexX = m_nMapIconTexX[1];
				item1.nTexY = m_nMapIconTexY[1];
				item1.bKeepSize = m_bMapIconKeepSize[1];
				item1.strTag = m_strMapIconTag[1];
				item1.arrFilter = item0.arrFilter;
				item1.strCondition = item0.strCondition;
				item1.nConditionValue = 1;
			}
		}
	}
}

bool CPawn::ChangeState( int32 nNewState, bool bInit )
{
	if( !IsValidStateIndex( nNewState ) )
		return false;
	m_nCurState = nNewState;
	return ChangeState( m_arrSubStates[nNewState], 0, bInit );
}

void CPawn::AutoCreateSpawnHelper()
{
	if( m_nLevelDataType )
	{
		auto pSpawnHelper = new CLevelSpawnHelper( -1, "", GetStateIndexByName( "death" ), m_nLevelDataType );
		pSpawnHelper->SetName( GetName() );
		m_pSpawnHelper = pSpawnHelper;
	}
}

void CPawn::SetTracerEffectDisabled( bool bDisabled )
{
	m_bTracerEffectDisabled = bDisabled;
}

void CPawn::InitState()
{
	m_bCurStateDirty = true;
	memset( m_nCurStateSpecialState, 0, sizeof( m_nCurStateSpecialState ) );
	if( m_pSpawnHelper && m_pSpawnHelper->m_bInitState )
	{
		if( m_pSpawnHelper->m_bSpawnDeath )
			m_nHp = 0;
		m_nCurForm = m_arrSubStates[m_pSpawnHelper->m_nInitState].nForm;
		ChangeState( m_pSpawnHelper->m_nInitState, true );
		m_nCurStateTick = m_pSpawnHelper->m_nInitStateTick;
		if( m_pAI )
			m_pAI->OnInit();
		Update0();
		return;
	}

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
		ChangeState( nInitState, true );
	if( m_pAI )
		m_pAI->OnInit();
	Update0();
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
	if( GetLevel() && GetLevel()->GetPlayer() && GetLevel()->GetPlayer()->GetControllingPawn() == this )
	{
		if( GetLevel()->GetPlayer()->ControllingPawnCheckStateInput( nReason ) )
			return true;
	}

	if( szToName && szToName[0] )
	{
		for( int i = 0; i < m_arrSubStates.Size(); i++ )
		{
			if( m_arrSubStates[i].strName == szToName )
				return ChangeState( i );
		}
	}
	if( nTo >= m_arrSubStates.Size() || nTo < 0 )
		return false;
	return ChangeState( nTo );
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

bool CPawn::ChangeState( SPawnState& state, int32 nStateSource, bool bInit )
{
	if( !bInit && GetLevel() )
	{
		if( m_arrForms.Size() && state.nForm != m_nCurForm )
		{
			if( !GetLevel()->PawnTransform( this, state.nForm, TVector2<int32>( 0, 0 ), false ) )
				return false;
		}
	}
	else
		ASSERT( !m_arrForms.Size() || state.nForm == m_nCurForm );
	if( !bInit && GetLevel() )
		GetLevel()->PawnMoveBreak( this );
	m_nCurStateTick = 0;
	m_curStateBeginPos = CVector2( m_pos.x, m_pos.y ) * LEVEL_GRID_SIZE;
	CRectangle rect( m_origRect.x - state.nImgExtLeft * m_origRect.width, m_origRect.y - state.nImgExtTop * m_origRect.height,
		m_origRect.width * ( 1 + state.nImgExtLeft + state.nImgExtRight ), m_origRect.height * ( 1 + state.nImgExtTop + state.nImgExtBottom ) );
	if( m_nCurDir )
		rect = CRectangle( LEVEL_GRID_SIZE_X * m_nWidth - rect.GetRight(), rect.y, rect.width, rect.height );
	m_curStateOrigPos = m_pos;
	m_curStateRect = rect;
	m_curStateOrigTexRect = m_origTexRect;
	m_nCurStateCheckAction = -1;
	m_bCurStateDirty = true;
	if( m_pAI )
		m_pAI->OnChangeState();
	m_trigger.Trigger( 2, NULL );
	return true;
}

void CPawn::Update0()
{
	m_nCurStateRenderOrder = ( m_pos.y + m_moveTo.y ) * 64 - m_nRenderOrder;
	if( GetRenderObject() )
		GetRenderObject()->bVisible = true;
	if( m_bMountHide )
	{
		if( GetRenderObject() )
			GetRenderObject()->bVisible = false;
	}
	else if( m_arrSubStates.Size() )
	{
		auto& curState = GetCurState();
		CRectangle texRect( m_curStateOrigTexRect.x + curState.nImgTexBeginX * m_curStateOrigTexRect.width, m_curStateOrigTexRect.y + curState.nImgTexBeginY * m_curStateOrigTexRect.height,
			m_curStateOrigTexRect.width * ( 1 + curState.nImgExtLeft + curState.nImgExtRight ), m_curStateOrigTexRect.height * ( 1 + curState.nImgExtTop + curState.nImgExtBottom ) );
		int32 nFrame = Min( curState.nImgTexCount - 1, m_nCurStateTick / curState.nTicksPerFrame );
		texRect = texRect.Offset( CVector2( texRect.width * ( nFrame % curState.nImgTexCols ), texRect.height * ( nFrame / curState.nImgTexCols ) ) );
		if( m_nCurDir )
			texRect = CRectangle( 2 - texRect.GetRight(), texRect.y, texRect.width, texRect.height );
		auto pTracerEffect = SafeCast<CTracerEffect>( GetRenderObject() );
		if( pTracerEffect )
			pTracerEffect->SetDisabled( m_bTracerEffectDisabled );
		auto pEft = SafeCastToInterface<IImageRect>( GetRenderObject() );
		if( pEft )
		{
			pEft->SetRect( m_curStateRect );
			pEft->SetTexRect( texRect );
		}
		else
		{
			auto pImage = SafeCast<CImage2D>( GetRenderObject() );
			if( pImage )
			{
				pImage->SetRect( m_curStateRect );
				pImage->SetBoundDirty();
				pImage->SetTexRect( texRect );
			}
		}
	}
	else
		m_curStateBeginPos = CVector2( m_pos.x, m_pos.y ) * LEVEL_GRID_SIZE;
	if( GetLevel() )
	{
		auto p = m_curStateBeginPos;
		if( IsSpecialState( eSpecialState_Effect_Shake ) )
		{
			int32 x = ( m_nCurStateTick * 4 + SRand::Inst<eRand_Render>().Rand( 0, 4 ) ) % 12;
			int32 y = ( m_nCurStateTick * 4 + SRand::Inst<eRand_Render>().Rand( 0, 4 ) ) % 8;
			p.x += abs( x - 6 ) - 3;
			p.y += abs( y - 4 ) - 2;
		}
		SetPosition( p );
		if( m_pHpBar )
		{
			m_pHpBar->bVisible = true;
			auto rect = m_hpBarOrigRect;
			rect.width = m_nHp * rect.width / m_nMaxHp;
			static_cast<CImage2D*>( m_pHpBar.GetPtr() )->SetRect( rect );
			m_pHpBar->SetBoundDirty();
			m_pHpBar->SetPosition( GetHpBarOfs() );
		}
	}
	else
	{
		if( m_pHpBar )
			m_pHpBar->bVisible = false;
	}
	if( m_pAI )
		m_pAI->OnUpdate0();
}

bool CPawn::CheckAction( int32 nGroup )
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

bool CPawn::CheckStateTransits( int32 nDefaultState, int8 nBreakFlag )
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
				m_nCurDir = 1 - m_nCurDir;
			TransitTo( transit.strToName, transit.nTo, nBreakFlag ? ePawnStateTransitReason_JumpTo : ePawnStateTransitCondition_Break );
			return true;
		}
	}
	auto Func = [this, nBreakFlag] ( SPawnStateTransit1& transit, int32 nSource ) {
		if( !FilterCommonTransit( transit, nSource ) )
			return false;
		if( ( transit.eCondition == ePawnStateTransitCondition_Break || transit.eCondition == ePawnStateTransitCondition_Finish )
			&& CheckTransitCondition( transit.eCondition, transit.strCondition ) )
		{
			if( transit.bInverse )
				m_nCurDir = 1 - m_nCurDir;
			TransitTo( transit.strToName, transit.nTo, nBreakFlag ? ePawnStateTransitReason_JumpTo : ePawnStateTransitCondition_Break );
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
				m_nCurDir = 1 - m_nCurDir;
			TransitTo( transit.strToName, transit.nTo, transit.eCondition );
			return true;
		}
		if( m_nHp <= 0 && bFinished && transit.eCondition == ePawnStateTransitCondition_Killed && CheckTransitCondition( transit.eCondition, transit.strCondition ) )
		{
			if( transit.bInverse )
				m_nCurDir = 1 - m_nCurDir;
			TransitTo( transit.strToName, transit.nTo, transit.eCondition );
			return true;
		}
		if( bFinished && transit.eCondition == ePawnStateTransitCondition_Finish && CheckTransitCondition( transit.eCondition, transit.strCondition ) )
		{
			if( transit.bInverse )
				m_nCurDir = 1 - m_nCurDir;
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
				m_nCurDir = 1 - m_nCurDir;
			TransitTo( transit.strToName, transit.nTo, transit.eCondition );
			return true;
		}
		if( m_nHp <= 0 && bFinished && transit.eCondition == ePawnStateTransitCondition_Killed && CheckTransitCondition( transit.eCondition, transit.strCondition ) )
		{
			if( transit.bInverse )
				m_nCurDir = 1 - m_nCurDir;
			TransitTo( transit.strToName, transit.nTo, transit.eCondition );
			return true;
		}
		if( bFinished && transit.eCondition == ePawnStateTransitCondition_Finish && CheckTransitCondition( transit.eCondition, transit.strCondition ) )
		{
			if( transit.bInverse )
				m_nCurDir = 1 - m_nCurDir;
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
	if( m_pSpecialRenderObject )
		m_pSpecialRenderObject->RemoveThis();
}

void CPlayerEquipment::Init()
{
	if( m_pOrigRenderObject )
		return;
	m_pOrigRenderObject = GetRenderObject();
	SetRenderObject( NULL );
	if( m_pSpecialRenderObject )
	{
		if( SafeCast<CEntity>( m_pSpecialRenderObject.GetPtr() ) )
			SafeCast<CEntity>( m_pSpecialRenderObject.GetPtr() )->SetParentEntity( NULL );
		else
			m_pSpecialRenderObject->RemoveThis();
	}
	for( int i = 0; i < 2; i++ )
	{
		if( m_pEft[i] )
			m_pEft[i]->RemoveThis();
	}
	if( m_pSpecialRenderObject )
		m_pSpecialRenderObject->RemoveThis();
}

void CPlayerEquipment::Drop( class CPlayer* pPlayer, const TVector2<int32>& pos, int8 nDir, int32 nPickupState )
{
	if( !m_pPickUp )
		return;
	SafeCast<CPickUp>( m_pPickUp.GetPtr() )->PreDrop( this, nPickupState );
	pPlayer->GetLevel()->AddPawn( m_pPickUp, pos, nDir );
	m_pPickUp = NULL;
}

void CPlayerEquipment::PrePickedUp( CPawn* pPickUp )
{
	m_pPickUp = pPickUp;
}

CPickUp* CPlayerEquipment::GetPickUp()
{
	return SafeCast<CPickUp>( m_pPickUp.GetPtr() );
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
	if( m_pEquipment )
		m_pEquipment->Init();
}

bool CPlayerMount::IsEnabled()
{
	if( m_bDisabled )
		return false;
	auto pPawn = GetPawn();
	if( pPawn->IsLocked() )
		return false;
	if( pPawn->GetCurStateIndex() != m_nNeedStateIndex || pPawn->GetCurStateSource() != 0 )
		return false;
	if( m_bNeedLevelComplete && !pPawn->GetLevel()->IsComplete() )
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
	pPlayer->Mount( GetPawn(), GetEquipment(), m_strEntryState, m_bAnimPlayerOriented, !m_bShowPawnOnMount, m_bUseMountRenderOrder, m_bEnablePreview );
}

CPlayerEquipment* CPlayerMount::GetEquipment()
{
	CPlayerMount* pMount = this;
	for( ;; )
	{
		auto pMount1 = SafeCast<CPlayerMount>( pMount->GetParentEntity() );
		if( !pMount1 )
			break;
		pMount = pMount1;
	}
	return pMount->m_pEquipment;
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

void CPlayer::OnRemovedFromStage()
{
	m_bForceHide = false;
}

void CPlayer::OnPreview()
{
	CPawn::OnPreview();
	if( m_pDefaultEquipment )
		m_pDefaultEquipment->RemoveThis();
}

void CPlayer::Reset( int8 nFlag )
{
	for( int i = 0; i < ePlayerEquipment_Count; i++ )
	{
		if( nFlag && i == ePlayerEquipment_Ability )
			continue;
		m_pCurEquipment[i] = NULL;
	}

	m_nHp = m_nMaxHp;
	m_nStealthValue = m_nMaxStealthValue = 0;
}

void CPlayer::LoadData( IBufReader& buf )
{
	for( int i = 0; i < ePlayerEquipment_Count; i++ )
		m_pCurEquipment[i] = NULL;

	int32 nVersionData;
	buf.Read( nVersionData );
	buf.Read( m_nHp );
	buf.Read( m_nMaxStealthValue );
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
		pPickUp->AutoCreateSpawnHelper();
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
	buf.Write( m_nMaxStealthValue );
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
	if( m_pDefaultEquipment )
		m_pDefaultEquipment->SetParentEntity( NULL );
	m_pCurUsingPawn = NULL;
	m_pCurMount = NULL;
	m_pCurMountingPawn = NULL;
	m_pControllingPawn = NULL;
	m_bForceUnMount = false;
	m_nStealthValue = m_nMaxStealthValue;
	if( m_bIsRealPlayer )
		ChangeAI( NULL );
	m_vecInputQueues.resize( 0 );
	m_nActionEftFrame = 0;
	/*m_nMoveXInput = 0;
	m_nMoveYInput = 0;
	m_nAttackInput = 0;*/
	if( !m_pOrigRenderObject )
		m_pOrigRenderObject = GetRenderObject();
	if( m_bIsRealPlayer )
	{
		for( int i = 0; i < 2; i++ )
		{
			m_pEft[i]->bVisible = false;
			if( !m_pCurEft[i] )
				m_pCurEft[i] = m_pEft[i];
		}
	}
	CPawn::Init();
	if( m_pDefaultEquipment )
	{
		m_pDefaultEquipment->Init();
		if( !m_pCurEquipment[0] && ( !m_bIsRealPlayer || m_bEnableDefaultEquipment ) )
			Equip( m_pDefaultEquipment );
	}
	m_bActionStop = true;
}

void CPlayer::Update()
{
	if( m_bCurStateDirty )
	{
		m_bCurStateDirty = false;
		memset( m_nCurStateSpecialState, 0, sizeof( m_nCurStateSpecialState ) );
	}
	uint8 nKeyUp = 0;
	if( m_bIsRealPlayer )
	{
		if( !GetLevel()->IsActionPreview() )
		{
			if( !GetLevel()->IsScenario() )
			{
				bool bInput = false;
				bool bDown = false;
				if( CGame::Inst().IsInputUp( eInput_Up ) || CGame::Inst().IsKeyUp( 'Q' ) || CGame::Inst().IsKeyUp( 'E' ) )
				{
					m_vecInputQueues.push_back( ePlayerInput_Up_Up ); bInput = true; nKeyUp |= 2;
				}
				if( CGame::Inst().IsInputUp( eInput_Down ) || CGame::Inst().IsKeyUp( 'Z' ) || CGame::Inst().IsKeyUp( 'C' ) )
				{
					m_vecInputQueues.push_back( ePlayerInput_Down_Up ); bInput = true; nKeyUp |= 8;
				}
				if( CGame::Inst().IsInputUp( eInput_Right ) || CGame::Inst().IsKeyUp( 'E' ) || CGame::Inst().IsKeyUp( 'C' ) )
				{
					m_vecInputQueues.push_back( ePlayerInput_Right_Up ); bInput = true; nKeyUp |= 1;
				}
				if( CGame::Inst().IsInputUp( eInput_Left ) || CGame::Inst().IsKeyUp( 'Q' ) || CGame::Inst().IsKeyUp( 'Z' ) )
				{
					m_vecInputQueues.push_back( ePlayerInput_Left_Up ); bInput = true; nKeyUp |= 4;
				}
				if( CGame::Inst().IsInputDown( eInput_Up ) || CGame::Inst().IsKeyDown( 'Q' ) || CGame::Inst().IsKeyDown( 'E' ) )
				{
					m_vecInputQueues.push_back( ePlayerInput_Up_Down ); bInput = true; bDown = true;
				}
				if( CGame::Inst().IsInputDown( eInput_Down ) || CGame::Inst().IsKeyDown( 'Z' ) || CGame::Inst().IsKeyDown( 'C' ) )
				{
					m_vecInputQueues.push_back( ePlayerInput_Down_Down ); bInput = true; bDown = true;
				}
				if( CGame::Inst().IsInputDown( eInput_Right ) || CGame::Inst().IsKeyDown( 'E' ) || CGame::Inst().IsKeyDown( 'C' ) )
				{
					m_vecInputQueues.push_back( ePlayerInput_Right_Down ); bInput = true; bDown = true;
				}
				if( CGame::Inst().IsInputDown( eInput_Left ) || CGame::Inst().IsKeyDown( 'Q' ) || CGame::Inst().IsKeyDown( 'Z' ) )
				{
					m_vecInputQueues.push_back( ePlayerInput_Left_Down ); bInput = true; bDown = true;
				}
				if( CGame::Inst().IsInputUp( eInput_A ) ) { m_vecInputQueues.push_back( ePlayerInput_A_Up ); bInput = true; nKeyUp |= 16; }
				if( CGame::Inst().IsInputUp( eInput_B ) ) { m_vecInputQueues.push_back( ePlayerInput_B_Up ); bInput = true; nKeyUp |= 32; }
				if( CGame::Inst().IsInputUp( eInput_C ) ) { m_vecInputQueues.push_back( ePlayerInput_C_Up ); bInput = true; nKeyUp |= 64; }
				if( CGame::Inst().IsInputUp( eInput_D ) ) { m_vecInputQueues.push_back( ePlayerInput_D_Up ); bInput = true; nKeyUp |= 128; }
				if( CGame::Inst().IsInputDown( eInput_A ) ) { m_vecInputQueues.push_back( ePlayerInput_A_Down ); bInput = true; bDown = true; }
				if( CGame::Inst().IsInputDown( eInput_B ) ) { m_vecInputQueues.push_back( ePlayerInput_B_Down ); bInput = true; bDown = true; }
				if( CGame::Inst().IsInputDown( eInput_C ) ) { m_vecInputQueues.push_back( ePlayerInput_C_Down ); bInput = true; bDown = true; }
				if( CGame::Inst().IsInputDown( eInput_D ) ) { m_vecInputQueues.push_back( ePlayerInput_D_Down ); bInput = true; bDown = true; }
				if( bDown && m_bActionStop && !m_nTickInputOnActionStop )
					m_nTickInputOnActionStop = 10;
				if( bInput )
					GetStage()->GetMasterLevel()->GetMainUI()->RefreshPlayerInput( ParseInputSequence(), -1, m_nChargeKeyDown, 0 );
			}
			else if( m_pAI && m_bActionStop && !m_nTickInputOnActionStop )
				m_nTickInputOnActionStop = 10;
			if( m_nTickInputOnActionStop )
			{
				m_nTickInputOnActionStop--;
				if( !m_nTickInputOnActionStop )
				{
					m_bActionStop = false;
					CheckAction( m_nCurActionGroup );
				}
			}
		}
	}
	if( m_bForceUnMount )
	{
		UnMount();
		PlayState( "stand" );
	}
	if( m_pControllingPawn && ( m_pControllingPawn->IsKilled() || !m_pControllingPawn->GetLevel() ) )
		EndControl();
	CPawn::Update();

	if( m_bIsRealPlayer )
	{
		if( !!( nKeyUp & m_nChargeKeyDown ) )
			m_nChargeKeyDown = 0;
		/*if( m_nActionEftFrame )
		{
			m_nActionEftFrame--;
			m_pCurEft[0]->bVisible = m_pCurEft[1]->bVisible = true;
			CRectangle rect, texRect;
			auto pEft = SafeCastToInterface<IImageRect>( GetRenderObject() );
			if( pEft )
			{
				rect = pEft->GetRect();
				texRect = pEft->GetTexRect();
			}
			else
			{
				auto pImg = static_cast<CImage2D*>( GetRenderObject() );

				rect = pImg->GetElem().rect;
				texRect = pImg->GetElem().texRect;
			}
			for( int i = 0; i < 2; i++ )
			{
				auto pImg1 = static_cast<CImage2D*>( m_pCurEft[i].GetPtr() );
				pImg1->SetRect( rect );
				pImg1->SetTexRect( texRect );
				pImg1->SetBoundDirty();
				pImg1->SetPosition( m_actionEftOfs[i * ACTION_EFT_FRAMES + m_nActionEftFrame] );
				pImg1->GetParam()[0] = m_actionEftParam[i * ACTION_EFT_FRAMES + m_nActionEftFrame];
			}
		}
		else*/
			m_pCurEft[0]->bVisible = m_pCurEft[1]->bVisible = false;
	}
}

void CPlayer::UpdateInputOnly()
{
	uint8 nKeyUp = 0;
	if( !GetLevel()->IsActionPreview() )
	{
		if( !GetLevel()->IsScenario() )
		{
			bool bInput = false;
			bool bDown = false;
			if( CGame::Inst().IsInputUp( eInput_Up ) || CGame::Inst().IsKeyUp( 'Q' ) || CGame::Inst().IsKeyUp( 'E' ) )
			{
				m_vecInputQueues.push_back( ePlayerInput_Up_Up ); bInput = true; nKeyUp |= 2;
			}
			if( CGame::Inst().IsInputUp( eInput_Down ) || CGame::Inst().IsKeyUp( 'Z' ) || CGame::Inst().IsKeyUp( 'C' ) )
			{
				m_vecInputQueues.push_back( ePlayerInput_Down_Up ); bInput = true; nKeyUp |= 8;
			}
			if( CGame::Inst().IsInputUp( eInput_Right ) || CGame::Inst().IsKeyUp( 'E' ) || CGame::Inst().IsKeyUp( 'C' ) )
			{
				m_vecInputQueues.push_back( ePlayerInput_Right_Up ); bInput = true; nKeyUp |= 1;
			}
			if( CGame::Inst().IsInputUp( eInput_Left ) || CGame::Inst().IsKeyUp( 'Q' ) || CGame::Inst().IsKeyUp( 'Z' ) )
			{
				m_vecInputQueues.push_back( ePlayerInput_Left_Up ); bInput = true; nKeyUp |= 4;
			}
			if( CGame::Inst().IsInputDown( eInput_Up ) || CGame::Inst().IsKeyDown( 'Q' ) || CGame::Inst().IsKeyDown( 'E' ) )
			{
				m_vecInputQueues.push_back( ePlayerInput_Up_Down ); bInput = true; bDown = true;
			}
			if( CGame::Inst().IsInputDown( eInput_Down ) || CGame::Inst().IsKeyDown( 'Z' ) || CGame::Inst().IsKeyDown( 'C' ) )
			{
				m_vecInputQueues.push_back( ePlayerInput_Down_Down ); bInput = true; bDown = true;
			}
			if( CGame::Inst().IsInputDown( eInput_Right ) || CGame::Inst().IsKeyDown( 'E' ) || CGame::Inst().IsKeyDown( 'C' ) )
			{
				m_vecInputQueues.push_back( ePlayerInput_Right_Down ); bInput = true; bDown = true;
			}
			if( CGame::Inst().IsInputDown( eInput_Left ) || CGame::Inst().IsKeyDown( 'Q' ) || CGame::Inst().IsKeyDown( 'Z' ) )
			{
				m_vecInputQueues.push_back( ePlayerInput_Left_Down ); bInput = true; bDown = true;
			}
			if( CGame::Inst().IsInputUp( eInput_A ) ) { m_vecInputQueues.push_back( ePlayerInput_A_Up ); bInput = true; nKeyUp |= 16; }
			if( CGame::Inst().IsInputUp( eInput_B ) ) { m_vecInputQueues.push_back( ePlayerInput_B_Up ); bInput = true; nKeyUp |= 32; }
			if( CGame::Inst().IsInputUp( eInput_C ) ) { m_vecInputQueues.push_back( ePlayerInput_C_Up ); bInput = true; nKeyUp |= 64; }
			if( CGame::Inst().IsInputUp( eInput_D ) ) { m_vecInputQueues.push_back( ePlayerInput_D_Up ); bInput = true; nKeyUp |= 128; }
			if( CGame::Inst().IsInputDown( eInput_A ) ) { m_vecInputQueues.push_back( ePlayerInput_A_Down ); bInput = true; bDown = true; }
			if( CGame::Inst().IsInputDown( eInput_B ) ) { m_vecInputQueues.push_back( ePlayerInput_B_Down ); bInput = true; bDown = true; }
			if( CGame::Inst().IsInputDown( eInput_C ) ) { m_vecInputQueues.push_back( ePlayerInput_C_Down ); bInput = true; bDown = true; }
			if( CGame::Inst().IsInputDown( eInput_D ) ) { m_vecInputQueues.push_back( ePlayerInput_D_Down ); bInput = true; bDown = true; }
			if( bDown && m_bActionStop && !m_nTickInputOnActionStop )
				m_nTickInputOnActionStop = 10;
			if( bInput )
				GetStage()->GetMasterLevel()->GetMainUI()->RefreshPlayerInput( ParseInputSequence(), -1, m_nChargeKeyDown, 0 );
		}
	}
	if( !!( nKeyUp & m_nChargeKeyDown ) )
		m_nChargeKeyDown = 0;
}

CPlayer* CPlayer::InitActionPreviewLevel( CMyLevel* pLevel, const TVector2<int32>& pos )
{
	auto pPlayer = SafeCast<CPlayer>( GetInstanceOwnerNode()->CreateInstance() );
	for( int i = 0; i < ePlayerEquipment_Count; i++ )
	{
		if( m_pCurEquipment[i] && m_pCurEquipment[i] != m_pDefaultEquipment.GetPtr() )
		{
			auto pPrefab = CResourceManager::Inst()->CreateResource<CPrefab>( m_pCurEquipment[i]->m_pPickUp->strCreatedFrom );
			auto pPickUp = SafeCast<CPickUp>( pPrefab->GetRoot()->CreateInstance() );
			pPickUp->strCreatedFrom = m_pCurEquipment[i]->m_pPickUp->strCreatedFrom;
			pPickUp->PickUp( pPlayer );
			auto pEquip = pPlayer->m_pCurEquipment[i];
			pEquip->Init();
			pEquip->SetAmmo( pEquip->GetMaxAmmo() );
		}
	}
	pPlayer->m_bEnableDefaultEquipment = m_bEnableDefaultEquipment;
	pLevel->AddPawn( pPlayer, pos, 0 );
	if( m_pCurMountingPawn && m_bMountEnablePreview )
	{
		auto pPawn1 = SafeCast<CPawn>( m_pCurMountingPawn->GetInstanceOwnerNode()->CreateInstance() );
		auto ofs = m_pCurMountingPawn->GetPos() - GetPos();
		auto dir = m_pCurMountingPawn->GetCurDir() == m_nDirBeforeMounting ? 0 : 1;
		if( m_nDirBeforeMounting == 1 )
			ofs.x = GetWidth() - ofs.x - pPawn1->GetWidth();
		pLevel->AddPawn( pPawn1, pos + ofs, dir );
	}
	pLevel->Begin();
	auto pMount = pLevel->FindMount( TVector2<int32>( 0, 0 ) );
	if( pMount )
		pMount->Mount( pPlayer );
	return pPlayer;
}

int32 CPlayer::Damage( int32 nDamage, int8 nDamageType, TVector2<int32> damageOfs, CPawn* pSource )
{
	if( m_bIsRealPlayer && !IsActionPreview() )
	{
		GetStage()->GetMasterLevel()->OnPlayerDamaged();
		if( m_strScriptDamaged )
		{
			auto pLuaState = CLuaMgr::GetCurLuaState();
			pLuaState->Load( m_strScriptDamaged );
			pLuaState->PushLua( this );
			pLuaState->Call( 1, 0 );
		}
	}
	return CPawn::Damage( nDamage, nDamageType, damageOfs, pSource );
}

void CPlayer::Block( TVector2<int32> damageOfs )
{
	if( m_pCurStateSource->m_strOnBlock.length() )
	{
		auto pLuaState = CLuaMgr::GetCurLuaState();
		pLuaState->Load( m_pCurStateSource->m_strOnBlock );
		pLuaState->PushLua( this );
		pLuaState->Call( 1, 0 );
	}
	CPawn::Block( damageOfs );
}

int8 CPlayer::TryPickUp( int32 nParam )
{
	if( !m_bIsRealPlayer )
	{
		if( m_pAI )
			m_pAI->TryPickUp();
		return 1;
	}
	if( m_pCurMountingPawn )
	{
		if( !IsActionPreview() )
			m_pCurMountingPawn->Signal( nParam );
		return 1;
	}
	if( m_pCurUsingPawn )
	{
		if( m_pCurUsingPawn->GetStage() )
			m_pCurUsingPawn->GetUsage()->UseHit( this );
		return 1;
	}
	if( nParam )
	{
		TVector2<int32> mountOfs[] = { { 2, 0 }, { 1, 1 }, { 1, -1 } };
		auto ofs = mountOfs[nParam - 1];
		if( m_nCurDir )
			ofs.x = -ofs.x;
		auto pMount = GetLevel()->FindMount( ofs );
		if( pMount )
		{
			if( pMount->GetCostEquipment().length() )
			{
				ASSERT( m_pCurEquipment[ePlayerEquipment_Large]->GetEquipmentName() == pMount->GetCostEquipment() );
				if( pMount->GetCostEquipmentType() == 0 )
					m_pCurEquipment[ePlayerEquipment_Large] = NULL;
			}
			pMount->Mount( this );
			return -1;
		}
		return 0;
	}

	auto pPickUp = GetLevel()->FindPickUp( GetMoveTo(), m_nWidth, m_nHeight );
	if( !pPickUp )
		return 0;
	pPickUp->PickUp( this );
	return 1;
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
		if( nIndex == ePlayerEquipment_Melee && ( !m_bIsRealPlayer || m_bEnableDefaultEquipment ) )
			m_pCurEquipment[ePlayerEquipment_Melee] = m_pDefaultEquipment;
		return true;
	}
	return false;
}

void CPlayer::Drop( int32 n )
{
	if( m_pCurEquipment[n] )
		TryDropIndex( n, 0 );
}

void CPlayer::DropAll()
{
	for( int i = 0; i < ePlayerEquipment_Count; i++ )
	{
		if( m_pCurEquipment[i] )
			TryDropIndex( i, 0 );
	}
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

const char* CPlayer::GetEquipmentName( int8 n )
{
	if( !m_pCurEquipment[n] )
		return "";
	return m_pCurEquipment[n]->GetEquipmentName();
}

void CPlayer::Mount( CPawn* pPawn, CPlayerEquipment* pMount, const char* szState, bool bAnimPlayerOriented, bool bMountHide, bool bUseMountRenderOrder, bool bMountEnablePreview )
{
	m_pCurMountingPawn = pPawn;
	m_pCurMountingPawn->SetMounted( true, bMountHide, 0 );
	m_pCurMount = pMount;
	m_bMountAnimPlayerOriented = bAnimPlayerOriented;
	m_bUseMountRenderOrder = bUseMountRenderOrder;
	m_bMountEnablePreview = bMountEnablePreview;
	m_nDirBeforeMounting = m_nCurDir;
	if( !bAnimPlayerOriented )
		m_nCurDir = pPawn->GetCurDir();
	VERIFY( TransitTo( szState, -1, -1 ) );
}

void CPlayer::UnMount( const char* szAction, int8 nActionDirType, int8 nMoveType )
{
	CReference<CPawn> pMount = m_pCurMountingPawn;
	bool bMountHide = pMount->IsMountHide();
	m_pCurMountingPawn->SetMounted( false, false, nMoveType );
	m_pCurMount = NULL;
	m_pCurMountingPawn = NULL;
	m_bForceUnMount = false;
	m_nCurDir = m_nDirBeforeMounting;
	if( szAction && szAction[0] )
	{
		if( nActionDirType == 2 )
			pMount->PlayStateSetDir( szAction, m_nDirBeforeMounting );
		else if( nActionDirType == 3 )
			pMount->PlayStateSetDir( szAction, 1 - m_nDirBeforeMounting );
		else if( nActionDirType == 1 )
			pMount->PlayStateTurnBack( szAction );
		else
			pMount->PlayState( szAction );
	}
	else if( bMountHide )
		pMount->ResetState();
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
		if( !m_pCurEquipment[ePlayerEquipment_Large] || sz != m_pCurEquipment[ePlayerEquipment_Large]->GetEquipmentName()
			|| m_pCurEquipment[ePlayerEquipment_Large]->GetAmmo() < pMount->GetNeedEquipmentCharge() )
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

void CPlayer::DisableDefaultEquipment()
{
	if( !m_bEnableDefaultEquipment )
		return;
	m_bEnableDefaultEquipment = false;
	if( m_pCurEquipment[0] == m_pDefaultEquipment.GetPtr() )
		UnEquip( m_pDefaultEquipment );
}

bool CPlayer::HasEquipment( int8 n, const char* szName )
{
	return m_pCurEquipment[n] && m_pCurEquipment[n]->GetEquipmentName() == szName;
}

void CPlayer::RemoveEquipment( int8 n )
{
	m_pCurEquipment[n] = NULL;
}

bool CPlayer::RestoreAmmo( int8 nAmmoType, int32 nMaxAmmo )
{
	auto pEquipment = GetEquipment( ePlayerEquipment_Ranged );
	if( pEquipment && ( nAmmoType < 0 || pEquipment->GetAmmoType() == nAmmoType ) )
	{
		if( !nMaxAmmo )
			nMaxAmmo = pEquipment->GetMaxAmmo();
		else
			nMaxAmmo = Min( nMaxAmmo, pEquipment->GetMaxAmmo() );
		pEquipment->SetAmmo( Max( nMaxAmmo, pEquipment->GetAmmo() ) );
		return true;
	}
	return false;
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

bool CPlayer::ControllingPawnCheckAction( int32 nActionGroup )
{
	return CheckAction( nActionGroup );
}

bool CPlayer::ControllingPawnCheckStateInput( int32 nReason )
{
	auto strDelayedChargeInput = m_strDelayedChargeInput;
	m_strDelayedChargeInput = "";
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
						if( IsActionPreview() )
						{
							if( strDelayedChargeInput.length() && nReason == ePawnStateTransitReason_JumpTo )
							{
								for( SInputTableItem::SInputItr itr( item ); itr.Next(); )
								{
									if( itr.l == strDelayedChargeInput.length() && 0 == strncmp( strDelayedChargeInput.c_str(), itr.sz, itr.l ) )
									{
										ExecuteInputtableItem( item, m_nCurStateSource );
										return true;
									}
								}
							}
							else
								ActionPreviewAddInputItem( m_nCurStateSource, &item );
						}
						else if( !m_bIsRealPlayer )
						{
							if( CheckAction( m_nCurActionGroup ) )
								return true;
						}
						else
						{
							int32 nMatchLen;
							auto szInput = CheckInputTableItem1( item, nMatchLen );
							if( szInput )
							{
								int32 nChargeKey = 0;
								if( nMatchLen && szInput[nMatchLen - 1] == '#' )
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
			}
			if( bChecked )
			{
				if( IsActionPreview() )
				{
					ActionPreviewAddInputItem( m_nCurStateSource, NULL );
					if( ActionPreviewWaitInput( nReason == ePawnStateTransitReason_JumpTo ) )
						return true;
				}
				else
					GetStage()->GetMasterLevel()->GetMainUI()->InsertDefaultFinishAction();
			}
		}
	}
	else if( nReason > ePawnStateTransitCondition_Finish )
		FlushInput( 0, 0, 2 );
	return false;
}

void CPlayer::BeginStealth( int32 nMaxValue )
{
	m_nStealthValue = m_nMaxStealthValue = nMaxValue;
}

void CPlayer::CancelStealth()
{
	m_nStealthValue = m_nMaxStealthValue = 0;
}

void CPlayer::UpdateStealthValue( int32 n )
{
	m_nStealthValue = Min( m_nMaxStealthValue, Max( 0, m_nStealthValue + n ) );
	if( !m_nStealthValue )
		m_nMaxStealthValue = 0;
}

void CPlayer::SetInputSequence( const char* szInput )
{
	SetInputSequence( szInput, m_parsedInputSequence );
}

void CPlayer::SetInputSequence( const char* szInput, vector<int8>& result )
{
	auto l = strlen( szInput );
	result.resize( l );
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
		result[i] = nInput;
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

	int32 nMaxInput = 99;
	if( m_parsedInputSequence.size() > nMaxInput )
	{
		if( m_strScriptInputOverflow.length() )
		{
			auto pLuaState = CLuaMgr::GetCurLuaState();
			pLuaState->Load( m_strScriptInputOverflow );
			pLuaState->PushLua( this );
			pLuaState->Call( 1, 0 );
		}
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
	auto strDelayedChargeInput = m_strDelayedChargeInput;
	m_strDelayedChargeInput = "";
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
						if( IsActionPreview() )
						{
							if( strDelayedChargeInput.length() && nReason == ePawnStateTransitReason_JumpTo )
							{
								for( SInputTableItem::SInputItr itr( item ); itr.Next(); )
								{
									if( itr.l == strDelayedChargeInput.length() && 0 == strncmp( strDelayedChargeInput.c_str(), itr.sz, itr.l ) )
									{
										ExecuteInputtableItem( item, m_nCurStateSource );
										return true;
									}
								}
							}
							else
								ActionPreviewAddInputItem( m_nCurStateSource, &item );
						}
						else if( !m_bIsRealPlayer )
						{
							if( CheckAction( m_nCurActionGroup ) )
								return true;
						}
						else
						{
							int32 nMatchLen;
							auto szInput = CheckInputTableItem1( item, nMatchLen );
							if( szInput )
							{
								int32 nChargeKey = 0;
								if( nMatchLen && szInput[nMatchLen - 1] == '#' )
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
			}
			if( bChecked )
			{
				if( IsActionPreview() )
				{
					ActionPreviewAddInputItem( m_nCurStateSource, NULL );
					if( ActionPreviewWaitInput( nReason == ePawnStateTransitReason_JumpTo ) )
						return true;
				}
				else
				{
					m_nActionEftFrame = ACTION_EFT_FRAMES;
					GetStage()->GetMasterLevel()->GetMainUI()->InsertDefaultFinishAction();
				}
			}
		}
	}
	else if( nReason > ePawnStateTransitCondition_Finish )
		FlushInput( 0, 0, 2 );
forcebreak:
	auto nChargeKeyDown = m_nChargeKeyDown;
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
					if( !ChangeState( arrStates[j], ePlayerStateSource_Mount, false ) )
						return false;
					if( nReason == ePawnStateTransitReason_JumpTo )
						m_nChargeKeyDown = nChargeKeyDown;
					return true;
				}
			}
		}
		else
		{
			for( int i0 = 0; i0 < ePlayerEquipment_Count; i0++ )
			{
				auto i = i0 == 0 ? ePlayerEquipment_Large : i0 - 1;
				if( m_nCurStateSource && m_nCurStateSource != ePlayerStateSource_Mount && i != m_nCurStateSource - 1 )
					continue;
				if( m_pCurEquipment[i] )
				{
					auto& arrStates = m_pCurEquipment[i]->m_arrSubStates;
					for( int j = 0; j < arrStates.Size(); j++ )
					{
						if( arrStates[j].strName == szToName )
						{
							m_nCurState = j;
							if( !ChangeState( arrStates[j], i + 1, false ) )
								return false;
							if( nReason == ePawnStateTransitReason_JumpTo )
								m_nChargeKeyDown = nChargeKeyDown;
							return true;
						}
					}
				}
			}
		}
	}
	if( CPawn::TransitTo( szToName, nTo, nReason ) )
	{
		if( nReason == ePawnStateTransitReason_JumpTo )
			m_nChargeKeyDown = nChargeKeyDown;
		return true;
	}
	return false;
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
		if( pStateSource && pStateSource == m_pCurStateSource )
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

bool CPlayer::ChangeState( SPawnState& state, int32 nStateSource, bool bInit )
{
	if( !bInit && GetLevel() )
	{
		if( m_arrForms.Size() && state.nForm != m_nCurForm )
		{
			if( !GetLevel()->PawnTransform( this, state.nForm, TVector2<int32>( 0, 0 ), false ) )
				return false;
		}
	}
	else
		ASSERT( !m_arrForms.Size() || state.nForm == m_nCurForm );
	if( !bInit && GetLevel() )
		GetLevel()->PawnMoveBreak( this );
	if( m_pCurUsingPawn )
	{
		if( m_pCurUsingPawn->GetStage() )
			m_pCurUsingPawn->GetUsage()->EndUse( this );
		m_pCurUsingPawn = NULL;
	}
	if( m_pCurMount )
	{
		if( nStateSource != ePlayerStateSource_Mount )
			UnMount();
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
		auto pTracerEffect = SafeCast<CTracerEffect>( m_pOrigRenderObject.GetPtr() );
		if( pTracerEffect && m_pCurStateSource->m_pSpecialRenderObject )
			m_pNewRenderObject = m_pCurStateSource->m_pSpecialRenderObject;
		else
			m_pNewRenderObject = m_pCurStateSource->m_pOrigRenderObject;
		if( m_bIsRealPlayer )
		{
			for( int i = 0; i < 2; i++ )
				m_pNewEft[i] = m_pCurStateSource->m_pEft[i];
		}
	}
	else
	{
		m_pNewRenderObject = m_pOrigRenderObject;
		if( m_bIsRealPlayer )
		{
			for( int i = 0; i < 2; i++ )
				m_pNewEft[i] = m_pEft[i];
		}
	}
	m_nCurStateSource = nStateSource;

	m_curStateBeginPos = ( nStateSource == ePlayerStateSource_Mount && !m_bMountAnimPlayerOriented ?
		CVector2( m_pCurMountingPawn->GetPos().x, m_pCurMountingPawn->GetPos().y ) : CVector2( m_pos.x, m_pos.y ) ) * LEVEL_GRID_SIZE;
	CRectangle rect( origRect.x - state.nImgExtLeft * origRect.width, origRect.y - state.nImgExtTop * origRect.height,
		origRect.width * ( 1 + state.nImgExtLeft + state.nImgExtRight ), origRect.height * ( 1 + state.nImgExtTop + state.nImgExtBottom ) );
	if( m_nCurDir )
		rect = CRectangle( LEVEL_GRID_SIZE_X * m_nWidth - rect.GetRight(), rect.y, rect.width, rect.height );
	m_curStateOrigPos = m_pos;
	m_curStateRect = rect;
	m_curStateOrigTexRect = origTexRect;
	m_nCurStateCheckAction = -1;
	memset( m_nCurStateSpecialState, 0, sizeof( m_nCurStateSpecialState ) );
	m_bCurStateDirty = true;
	if( GetLevel() )
		GetLevel()->OnPlayerChangeState( state, nStateSource, m_nCurDir );
	if( m_pAI )
		m_pAI->OnChangeState();
	m_trigger.Trigger( 2, NULL );
	return true;
}

void CPlayer::Update0()
{
	if( m_pNewRenderObject )
	{
		if( m_pNewRenderObject != GetRenderObject() )
			SetRenderObject( m_pNewRenderObject );
		m_pNewRenderObject = NULL;
	}
	if( m_bIsRealPlayer )
	{
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
	}
	CPawn::Update0();
	if( m_pCurMountingPawn && m_nCurStateSource == ePlayerStateSource_Mount && m_bUseMountRenderOrder )
		m_nCurStateRenderOrder = ( m_pCurMountingPawn->GetPosY() + m_pCurMountingPawn->GetToY() ) * 64 - m_pCurMountingPawn->GetRenderOrder();
}

bool CPlayer::IsCurStateInterrupted()
{
	//if( !m_pCurMount && m_nCurStateSource == ePlayerStateSource_Mount )
	//	return true;
	return false;
}

bool CPlayer::CheckAction( int32 nGroup )
{
	if( !m_bIsRealPlayer )
	{
		if( m_bMounted )
			return false;
		if( m_pAI )
		{
			auto nDir = m_nCurDir;
			CString strState = "";
			if( m_pAI->CheckAction1( strState, m_nCurDir ) )
			{
				if( TransitTo( strState, -1, -1 ) )
					return true;
				m_nCurDir = nDir;
			}
		}
		return false;
	}
	m_nCurActionGroup = nGroup;
	if( !m_bActionStop || IsActionPreview() )
	{
		ParseInputSequence();
		bool b = false;
		if( CPawn::CheckAction( nGroup ) )
			b = true;
		else
			b = HandleInput( nGroup );
		m_bActionStop = !b;
		return b;
	}
	return false;
}

bool CPlayer::CheckCanFinish()
{
	return !m_nChargeKeyDown && !m_strDelayedChargeInput.length() && !m_pControllingPawn;
}

bool CPlayer::HandleInput( int32 nActionGroup )
{
	if( m_pControllingPawn )
	{
		auto& inputTable = *m_pControllingPawn->GetControllingInputTable();
		for( int j = inputTable.Size() - 1; j >= 0; j-- )
		{
			auto& item = inputTable[j];
			if( item.nActionGroup != nActionGroup )
				continue;
			if( IsActionPreview() )
				ActionPreviewAddInputItem( m_nCurStateSource, &item );
			else
			{
				auto len = CheckInputTableItem( item );
				if( len )
				{
					bool b = ExecuteInputtableItem( item, -1 );
					if( !b )
						break;
					FlushInput( len, 0, 0 );
					return true;
				}
			}
		}
		if( IsActionPreview() )
			return ActionPreviewWaitInput( false );
		FlushInput( 0, 0, 0 );
		return false;
	}
	if( m_pCurMount )
	{
		auto& inputTable = m_pCurMount->m_inputTable;
		for( int j = inputTable.Size() - 1; j >= 0; j-- )
		{
			auto& item = inputTable[j];
			if( item.nActionGroup != nActionGroup )
				continue;
			if( IsActionPreview() )
				ActionPreviewAddInputItem( ePlayerStateSource_Mount, &item );
			else
			{
				auto len = CheckInputTableItem( item );
				if( len )
				{
					bool b = ExecuteInputtableItem( item, ePlayerStateSource_Mount );
					if( !b )
						break;
					FlushInput( len, 0, 0 );
					return true;
				}
			}
		}
		if( IsActionPreview() )
		{
			if( ActionPreviewWaitInput( false ) )
				return true;
		}
		//m_nCurState = 0;
		//ChangeState( m_pCurMount->m_arrSubStates[0], ePlayerStateSource_Mount, false );
		FlushInput( 0, 0, 0 );
		return false;
	}
	if( m_pCurEquipment[ePlayerEquipment_Large] )
	{
		auto& inputTable = m_pCurEquipment[ePlayerEquipment_Large]->m_inputTable;
		for( int j = inputTable.Size() - 1; j >= 0; j-- )
		{
			auto& item = inputTable[j];
			if( item.nActionGroup != nActionGroup )
				continue;
			if( IsActionPreview() )
				ActionPreviewAddInputItem( ePlayerEquipment_Large + 1, &item );

			else
			{
				auto len = CheckInputTableItem( item );
				if( len )
				{
					bool b = ExecuteInputtableItem( item, ePlayerEquipment_Large + 1 );
					if( !b )
						break;
					FlushInput( len, 0, 0 );
					return true;
				}
			}
		}
		if( !IsActionPreview() && m_parsedInputSequence.size() && m_parsedInputSequence.back() == -4 )
		{
			FlushInput( 1, 0, 0 );
			auto pMount = GetLevel()->FindMount( TVector2<int32>( 0, 0 ) );
			if( pMount )
			{
				if( pMount->GetCostEquipment().length() )
				{
					ASSERT( m_pCurEquipment[ePlayerEquipment_Large]->GetEquipmentName() == pMount->GetCostEquipment() );
					if( pMount->GetCostEquipmentType() == 0 )
						m_pCurEquipment[ePlayerEquipment_Large] = NULL;
				}
				pMount->Mount( this );
				return true;
			}
			TransitTo( "break", 0, -1 );
			return true;
		}
		if( IsActionPreview() )
		{
			if( ActionPreviewWaitInput( false ) )
				return true;
		}
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
				if( item.nActionGroup != nActionGroup )
					continue;
				if( IsActionPreview() )
					ActionPreviewAddInputItem( i + 1, &item );

				else
				{
					auto len = CheckInputTableItem( item );
					if( len )
					{
						bool b = ExecuteInputtableItem( item, i + 1 );
						if( !b )
							break;
						FlushInput( len, 0, 0 );
						return true;
					}
				}
			}
		}
	}
	for( int i = m_inputTable.Size() - 1; i >= 0; i-- )
	{
		auto& item = m_inputTable[i];
		if( item.nActionGroup != nActionGroup )
			continue;
		if( IsActionPreview() )
			ActionPreviewAddInputItem( 0, &item );
		else
		{
			auto len = CheckInputTableItem( item );
			if( len )
			{
				bool b = ExecuteInputtableItem( item, 0 );
				if( !b )
					break;
				FlushInput( len, 0, 0 );
				return true;
			}
		}
	}
	if( IsActionPreview() )
		return ActionPreviewWaitInput( false );

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
			auto pMount = GetLevel()->FindMount( TVector2<int32>( 0, 0 ) );
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

int32 CPlayer::CheckInputTableItem( SInputTableItem& item )
{
	int32 l = 0;
	CheckInputTableItem1( item, l );
	return l;
}

const char* CPlayer::CheckInputTableItem1( SInputTableItem& item, int32& len )
{
	for( SInputTableItem::SInputItr itr( item ); itr.Next(); )
	{
		if( itr.lCondition )
		{
			char* sz = (char*)alloca( itr.lCondition + 1 );
			memcpy( sz, itr.szCondition, itr.lCondition );
			sz[itr.lCondition] = 0;
			if( !GetLevel()->GetMasterLevel()->EvaluateKeyInt( sz ) )
				continue;
		}
		auto sz = itr.sz;
		int32 l = itr.l;
		if( l && sz[l - 1] == '#' )
		{
			if( !m_nChargeKeyDown )
				continue;
			l--;
		}
		if( m_parsedInputSequence.size() < l )
			continue;
		auto nCurDir = m_pControllingPawn ? m_pControllingPawn->GetCurDir() : m_nCurDir;
		bool bOK = true;
		for( int j = 0; j < l; j++ )
		{
			auto nInput = m_parsedInputSequence[j + m_parsedInputSequence.size() - l];
			auto chInput = sz[j];
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
			{
				bOK = false;
				break;
			}
		}
		if( bOK )
		{
			len = itr.l;
			return sz;
		}
	}
	return NULL;
}

bool CPlayer::ExecuteInputtableItem( SInputTableItem& item, int32 nStateSource )
{
	CString strStateName = item.strStateName;
	if( strStateName.length() && strStateName.c_str()[0] == ':' )
	{
		auto pLuaState = CLuaMgr::GetCurLuaState();
		pLuaState->Load( strStateName.c_str() + 1 );
		pLuaState->PushLua( this );
		pLuaState->Call( 1, 0 );
		strStateName = "";
	}
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
		m_strActionPreviewCharge = item.strCharge;
	}

	if( m_pControllingPawn )
	{
		auto nDir = m_pControllingPawn->GetCurDir();
		if( item.bInverse )
			nDir = 1 - nDir;
		m_pControllingPawn->StateTransit( strStateName, item.nStateIndex, nDir );
	}
	else
	{
		if( item.bInverse )
			m_nCurDir = 1 - m_nCurDir;
		if( nStateSource )
		{
			auto pStateSource = GetStateSource( nStateSource );
			if( strStateName.length() )
			{
				for( int k = 0; k < pStateSource->m_arrSubStates.Size(); k++ )
				{
					auto& state = pStateSource->m_arrSubStates[k];
					if( state.strName == strStateName )
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
						if( state.strName == strStateName )
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
			TransitTo( strStateName, item.nStateIndex, -1 );
	}
	m_nChargeKeyDown = nChargeKeyDown;
	return true;
}

void CPlayer::FlushInput( int32 nMatchLen, int8 nChargeKey, int8 nType )
{
	if( !m_bIsRealPlayer || IsActionPreview() )
		return;
	m_vecInputQueues.resize( 0 );
	if( nType == 2 )
		m_parsedInputSequence.resize( 0 );
	else if( !m_pControllingPawn )
		m_nActionEftFrame = ACTION_EFT_FRAMES;
	GetStage()->GetMasterLevel()->GetMainUI()->OnPlayerAction( m_parsedInputSequence, nMatchLen, nChargeKey, nType );
	GetLevel()->OnPlayerAction( m_parsedInputSequence, nMatchLen, nType );
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

void CPlayer::ActionPreviewAddInputItem( int8 nType, SInputTableItem* pItem )
{
	if( pItem )
	{
		if( pItem->nStateIndex < 0 )
			return;
		auto& vec = m_vecActionPreviewInputItem[nType];
		for( auto& pItem1 : vec )
		{
			if( !pItem1 )
				continue;
			if( pItem1->bInverse != pItem->bInverse )
				continue;
			if( pItem1->strStateName.length() || pItem->strStateName.length() )
			{
				if( pItem1->strStateName == pItem->strStateName )
				{
					pItem1 = pItem;
					return;
				}
			}
			else if( pItem1->nStateIndex == pItem->nStateIndex )
			{
				pItem1 = pItem;
				return;
			}
		}
	}
	m_vecActionPreviewInputItem[nType].push_back( pItem );
}

bool CPlayer::ActionPreviewWaitInput( bool bJumpTo )
{
	m_nActionPreviewType = -1;
	m_nActionPreviewIndex = -1;
	while( m_nActionPreviewIndex == -1 )
		GetLevel()->ActionPreviewPause();

	if( m_nActionPreviewType == -1 )
		return false;
	auto pItem = m_vecActionPreviewInputItem[m_nActionPreviewType][m_nActionPreviewIndex];
	for( int i = 0; i < ELEM_COUNT( m_vecActionPreviewInputItem ); i++ )
	{
		m_vecActionPreviewInputItem[i].clear();
	}
	if( pItem )
	{
		if( !bJumpTo && pItem->strInput.length() )
		{
			SInputTableItem::SInputItr itr( *pItem );
			auto l = itr.Next();
			if( l && itr.sz[l - 1] == '#' )
			{
				m_strDelayedChargeInput.assign( itr.sz, itr.sz + itr.l );
				return true;
			}
		}
		ExecuteInputtableItem( *pItem, m_nActionPreviewType );
		return true;
	}
	return false;
}

void CPlayer::ActionPreviewInput( int8 nType, int32 nIndex )
{
	m_nActionPreviewType = nType;
	m_nActionPreviewIndex = nIndex;
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
			CPawn* pPawn = pGrid->pPawn0;
			if( pPawn && pPawn != m_pCreator && !pPawn->IsIgnoreBullet() && !pPawn->IsSpecialState( eSpecialState_Fall ) )
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
		auto pPawn = pGrid->pPawn0;
		bool bHit = pPawn && pPawn.GetPtr() != m_pCreator && !( desc.nHitType && pPawn->IsSpecialState( eSpecialState_Fall ) );
		if( bHit )
		{
			auto nCheckHit = pPawn->CheckHit( p, desc.nDamageType );
			if( nCheckHit <= 0 )
			{
				bHit = false;
				if( nCheckHit < 0 )
					pPawn->Block( hitOfs + m_hitOfs );
			}
		}
		if( !bHit )
		{
			if( desc.nDamageType > 0 )
				pGrid->nMissBashEft = ( desc.nHitType == 1 ? -1 : 1 ) * (int32)CGlobalCfg::Inst().lvIndicatorData.vecMissParams.size();
			else
				pGrid->nMissEft = ( desc.nHitType == 1 ? -1 : 1 ) * (int32)CGlobalCfg::Inst().lvIndicatorData.vecMissParams.size();
			continue;
		}

		if( desc.nDamage )
		{
			auto n = pPawn->Damage( desc.nDamage, desc.nDamageType, hitOfs + m_hitOfs, this );
			if( !n )
				pGrid->nHitBlockedEft = ( desc.nHitType == 1 ? -1 : 1 ) * (int32)CGlobalCfg::Inst().lvIndicatorData.vecHitBlockedParams.size();
			else if( desc.nDamageType > 0 )
				pGrid->nHitBashEft = ( desc.nHitType == 1 ? -1 : 1 ) * (int32)CGlobalCfg::Inst().lvIndicatorData.vecHitParams.size();
			else
				pGrid->nHitEft = ( desc.nHitType == 1 ? -1 : 1 ) * (int32)CGlobalCfg::Inst().lvIndicatorData.vecHitParams.size();
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

void CPickUp::LoadData( IBufReader& buf )
{
	CPawn::LoadData( buf );
	m_bDropped = false;
	m_bUseInitState = false;
	if( m_pEquipment )
		m_pEquipment->LoadData( buf );
}

void CPickUp::SaveData( CBufFile& buf )
{
	m_bDropped = false;
	m_bUseInitState = false;
	CPawn::SaveData( buf );
	if( m_pEquipment )
		m_pEquipment->SaveData( buf );
}

void CPickUp::Init()
{
	CPawnHit::Init();
	if( m_pEquipment && m_pEquipment->GetParentEntity() == this )
	{
		m_pEquipment->SetParentEntity( NULL );
		m_pEquipment->Init();
	}
	m_nLifeLeft = m_nLife;
}

void CPickUp::Update()
{
	CPawn::Update();
	if( m_nLifeLeft && IsPickUpReady() )
	{
		m_nLifeLeft--;
		if( !m_nLifeLeft )
		{
			GetLevel()->PawnDeath( this );
			return;
		}
		if( m_nLifeLeft < 300 )
			GetRenderObject()->bVisible = !( ( m_nLifeLeft / 2 ) % 5 );
		else
			GetRenderObject()->bVisible = !( ( m_nLifeLeft / 15 ) & 1 );
	}
}

bool CPickUp::IsPickUpReady()
{
	return 0 == strncmp( "stand", GetCurState().strName.c_str(), 5 );
}

bool CPickUp::PickUp( CPlayer* pPlayer )
{
	if( GetLevel() )
	{
		if( m_strScript.length() )
		{
			auto pLuaState = CLuaMgr::GetCurLuaState();
			pLuaState->Load( m_strScript );
			pLuaState->PushLua( this );
			pLuaState->Call( 1, 1 );
			int32 nResult = pLuaState->PopLuaValue<int32>();
			if( nResult )
				return false;
		}
	}
	if( m_pEquipment )
	{
		m_pEquipment->PrePickedUp( this );
		pPlayer->Equip( m_pEquipment );
		m_pEquipment = NULL;
	}
	if( GetLevel() )
	{
		OnKilled();
		m_strKillScript = "";
		GetLevel()->RemovePawn( this );
	}
	return true;
}

bool CPickUp::PickUp1( CPlayer * pPlayer )
{
	if( m_strScript.length() )
	{
		auto pLuaState = CLuaMgr::GetCurLuaState();
		pLuaState->Load( m_strScript );
		pLuaState->PushLua( this );
		pLuaState->Call( 1, 1 );
		int32 nResult = pLuaState->PopLuaValue<int32>();
		if( nResult )
			return false;
	}
	if( m_pEquipment )
	{
		m_pEquipment->PrePickedUp( this );
		pPlayer->Equip( m_pEquipment );
		m_pEquipment = NULL;
	}
	return true;
}

int32 CPickUp::GetDefaultState()
{
	if( !m_bDropped )
		return GetStateIndexByName( "stand" );
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
		REGISTER_ENUM_ITEM( ePawnStateEventType_SpecialState )
		REGISTER_ENUM_ITEM( ePawnStateEventType_Interaction )
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
		REGISTER_MEMBER( arrTags )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( SMapIconData )
		REGISTER_MEMBER( ofs )
		REGISTER_MEMBER( strTag )
		REGISTER_MEMBER( strCondition )
		REGISTER_MEMBER( nConditionValue )
		REGISTER_MEMBER( arrFilter )
		REGISTER_MEMBER( bKeepSize )
		REGISTER_MEMBER( nTexX )
		REGISTER_MEMBER( nTexY )
		REGISTER_MEMBER( nDir )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( SPawnHitSpawnDesc )
		REGISTER_MEMBER( pHit )
		REGISTER_MEMBER( strInitState )
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
		REGISTER_LUA_CFUNCTION( GetIntValue )
		REGISTER_LUA_CFUNCTION( SetIntValue )
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
		REGISTER_MEMBER( nActionGroup )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( SStateInputTableItem )
		REGISTER_MEMBER( arrStates )
		REGISTER_MEMBER( input )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CPawn )
		REGISTER_BASE_CLASS( CEntity )
		REGISTER_BASE_CLASS( ISignalObj )
		REGISTER_MEMBER( m_bIsEnemy )
		REGISTER_MEMBER( m_bIsDynamic )
		REGISTER_MEMBER( m_bBlockSight )
		REGISTER_MEMBER( m_bIgnoreBullet )
		REGISTER_MEMBER( m_bForceHit )
		REGISTER_MEMBER( m_bIgnoreBlockedExit )
		REGISTER_MEMBER( m_bNextStageBlock )
		REGISTER_MEMBER( m_bHideInEditor )
		REGISTER_MEMBER( m_nInitDir )
		REGISTER_MEMBER( m_nArmorType )
		REGISTER_MEMBER( m_bUseInitState )
		REGISTER_MEMBER( m_bUseDefaultState )
		REGISTER_MEMBER( m_nLevelDataType )
		REGISTER_MEMBER( m_bIconOnly )
		REGISTER_MEMBER( m_nInitState )
		REGISTER_MEMBER( m_nDefaultState )
		REGISTER_MEMBER( m_nWidth )
		REGISTER_MEMBER( m_nHeight )
		REGISTER_MEMBER( m_nHitWidth )
		REGISTER_MEMBER( m_nHitHeight )
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
		REGISTER_MEMBER( m_nMapIconType )
		REGISTER_MEMBER( m_bMapIconKeepSize )
		REGISTER_MEMBER( m_nMapIconTexX )
		REGISTER_MEMBER( m_nMapIconTexY )
		REGISTER_MEMBER( m_strMapIconTag )
		REGISTER_MEMBER( m_arrTags )

		DEFINE_LUA_REF_OBJECT()
		REGISTER_LUA_CFUNCTION( ScriptDamage )
		REGISTER_LUA_CFUNCTION( SetDamaged )
		REGISTER_LUA_CFUNCTION( GetLevel )
		REGISTER_LUA_CFUNCTION( GetCreator )
		REGISTER_LUA_CFUNCTION( GetPosX )
		REGISTER_LUA_CFUNCTION( GetPosY )
		REGISTER_LUA_CFUNCTION( GetToX )
		REGISTER_LUA_CFUNCTION( GetToY )
		REGISTER_LUA_CFUNCTION( GetCurStateDestX )
		REGISTER_LUA_CFUNCTION( GetCurStateDestY )
		REGISTER_LUA_CFUNCTION( IsPosHidden )
		REGISTER_LUA_CFUNCTION( IsToHidden )
		REGISTER_LUA_CFUNCTION( IsEnemy )
		REGISTER_LUA_CFUNCTION( HasTag )
		REGISTER_LUA_CFUNCTION( GetTag )
		REGISTER_LUA_CFUNCTION( GetCurDir )
		REGISTER_LUA_CFUNCTION( GetCurStateIndex )
		REGISTER_LUA_CFUNCTION( SetInitState )
		REGISTER_LUA_CFUNCTION( SetDefaultState )
		REGISTER_LUA_CFUNCTION( GetHp )
		REGISTER_LUA_CFUNCTION( GetMaxHp )
		REGISTER_LUA_CFUNCTION( SetHp )
		REGISTER_LUA_CFUNCTION( IsKilled )
		REGISTER_LUA_CFUNCTION( SetForceHide )
		REGISTER_LUA_CFUNCTION( IsDamaged )
		REGISTER_LUA_CFUNCTION( GetDamageType )
		REGISTER_LUA_CFUNCTION( GetDamageOfsX )
		REGISTER_LUA_CFUNCTION( GetDamageOfsY )
		REGISTER_LUA_CFUNCTION( GetDamageOfsDir )
		REGISTER_LUA_CFUNCTION( GetDamageOfsDir1 )
		REGISTER_LUA_CFUNCTION( HasStateTag )
		REGISTER_LUA_CFUNCTION( IsLocked )
		REGISTER_LUA_CFUNCTION( SetLocked )
		REGISTER_LUA_CFUNCTION( PlayState )
		REGISTER_LUA_CFUNCTION( PlayStateTurnBack )
		REGISTER_LUA_CFUNCTION( PlayStateSetDir )
		REGISTER_LUA_CFUNCTION( PlayStateForceMove )
		REGISTER_LUA_CFUNCTION( GetAI )
		REGISTER_LUA_CFUNCTION( ChangeAI )
		REGISTER_LUA_CFUNCTION( GetCurStateName )
		REGISTER_LUA_CFUNCTION( RegisterSignalScript )
		REGISTER_LUA_CFUNCTION( RegisterKilledScript )
		REGISTER_LUA_CFUNCTION( SetTracerEffectDisabled )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CPlayerEquipment )
		REGISTER_BASE_CLASS( CEntity )
		REGISTER_MEMBER( m_strEquipmentName )
		REGISTER_MEMBER( m_nEquipType )
		REGISTER_MEMBER( m_nAmmoType )
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
		REGISTER_MEMBER_BEGIN( m_strOnBlock )
			MEMBER_ARG( text, 1 )
		REGISTER_MEMBER_END()
		REGISTER_MEMBER_TAGGED_PTR( m_pEft[0], 0 )
		REGISTER_MEMBER_TAGGED_PTR( m_pEft[1], 1 )
		REGISTER_MEMBER_TAGGED_PTR( m_pSpecialRenderObject, sp )
		DEFINE_LUA_REF_OBJECT()
		REGISTER_LUA_CFUNCTION( GetAmmo )
		REGISTER_LUA_CFUNCTION( SetAmmo )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CPlayerMount )
		REGISTER_BASE_CLASS( CEntity )
		REGISTER_MEMBER( m_bDisabled )
		REGISTER_MEMBER( m_bHidden )
		REGISTER_MEMBER( m_bAnimPlayerOriented )
		REGISTER_MEMBER( m_bUseMountRenderOrder )
		REGISTER_MEMBER( m_bShowPawnOnMount )
		REGISTER_MEMBER( m_bNeedLevelComplete )
		REGISTER_MEMBER( m_bEnablePreview )
		REGISTER_MEMBER( m_nEnterDir )
		REGISTER_MEMBER( m_nCostEquipType )
		REGISTER_MEMBER( m_nOfsX )
		REGISTER_MEMBER( m_nOfsY )
		REGISTER_MEMBER( m_nPawnOfsX )
		REGISTER_MEMBER( m_nPawnOfsY )
		REGISTER_MEMBER( m_strEntryState )
		REGISTER_MEMBER( m_strCostEquipment )
		REGISTER_MEMBER( m_nNeedStateIndex )
		REGISTER_MEMBER( m_nNeedEquipCharge )
		REGISTER_MEMBER_TAGGED_PTR( m_pEquipment, equipment )
		DEFINE_LUA_REF_OBJECT()
		REGISTER_LUA_CFUNCTION( SetEnabled )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CPlayer )
		REGISTER_BASE_CLASS( CPawn )
		REGISTER_MEMBER( m_bIsRealPlayer )
		REGISTER_MEMBER_BEGIN( m_strScriptDamaged )
			MEMBER_ARG( text, 1 )
		REGISTER_MEMBER_END()
		REGISTER_MEMBER_BEGIN( m_strScriptInputOverflow )
			MEMBER_ARG( text, 1 )
		REGISTER_MEMBER_END()
		REGISTER_MEMBER( m_inputTable )
		REGISTER_MEMBER( m_stateInputTable )
		REGISTER_MEMBER( m_actionEftOfs )
		REGISTER_MEMBER( m_actionEftParam )
		REGISTER_MEMBER_TAGGED_PTR( m_pEft[0], 0 )
		REGISTER_MEMBER_TAGGED_PTR( m_pEft[1], 1 )
		REGISTER_MEMBER_TAGGED_PTR( m_pDefaultEquipment, default_weapon )
		DEFINE_LUA_REF_OBJECT()
		REGISTER_LUA_CFUNCTION( Reset )
		REGISTER_LUA_CFUNCTION( Drop )
		REGISTER_LUA_CFUNCTION( DropAll )
		REGISTER_LUA_CFUNCTION( GetEquipment )
		REGISTER_LUA_CFUNCTION( GetEquipmentName )
		REGISTER_LUA_CFUNCTION( ForceUnMount )
		REGISTER_LUA_CFUNCTION( GetCurMountingPawn )
		REGISTER_LUA_CFUNCTION( IsReadyForMount )
		REGISTER_LUA_CFUNCTION( EnableDefaultEquipment )
		REGISTER_LUA_CFUNCTION( DisableDefaultEquipment )
		REGISTER_LUA_CFUNCTION( HasEquipment )
		REGISTER_LUA_CFUNCTION( RemoveEquipment )
		REGISTER_LUA_CFUNCTION( RestoreAmmo )
		REGISTER_LUA_CFUNCTION( BeginStealth )
		REGISTER_LUA_CFUNCTION( CancelStealth )
		REGISTER_LUA_CFUNCTION( GetStealthValue )
		REGISTER_LUA_CFUNCTION( IsHidden )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( SHitGridDesc )
		REGISTER_MEMBER( nHitIndex )
		REGISTER_MEMBER( nOfsX )
		REGISTER_MEMBER( nOfsY )
		REGISTER_MEMBER( nDamage )
		REGISTER_MEMBER( nHitType )
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
		REGISTER_MEMBER( m_nLife )
		REGISTER_MEMBER_TAGGED_PTR( m_pEquipment, equipment )
		DEFINE_LUA_REF_OBJECT()
		REGISTER_LUA_CFUNCTION( GetEquipment )
	REGISTER_CLASS_END()
}