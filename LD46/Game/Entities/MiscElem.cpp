#include "stdafx.h"
#include "MiscElem.h"
#include "Stage.h"
#include "UtilEntities.h"
#include "CommonUtils.h"
#include "Common/Rand.h"
#include "GlobalCfg.h"

void CLevelScriptCustom::OnInit( CMyLevel* pLevel )
{
	if( m_strInit.length() )
		CLuaMgr::Inst().Run( m_strInit );
}

void CLevelScriptCustom::OnBegin( CMyLevel * pLevel )
{
	if( m_strBegin.length() )
		CLuaMgr::Inst().Run( m_strBegin );
}

void CLevelScriptCustom::OnDestroy( CMyLevel* pLevel )
{
	if( m_strDestroy.length() )
		CLuaMgr::Inst().Run( m_strDestroy );
}

void CLevelScriptCustom::OnUpdate( CMyLevel* pLevel )
{
	if( m_strUpdate.length() )
		CLuaMgr::Inst().Run( m_strUpdate );
}

void CLevelScriptCustom::OnUpdate1( CMyLevel* pLevel )
{
	if( m_strUpdate1.length() )
		CLuaMgr::Inst().Run( m_strUpdate1 );
}

void CLevelScriptCustom::OnPlayerChangeState( SPawnState& state, int32 nStateSource, int8 nDir )
{
	if( m_strPlayerChangeState.length() )
	{
		CLuaMgr::Inst().Load( m_strPlayerChangeState );
		CLuaMgr::Inst().PushLua( state.strName.c_str() );
		CLuaMgr::Inst().PushLua( nStateSource );
		CLuaMgr::Inst().PushLua( nDir );
		CLuaMgr::Inst().Call( 3, 0 );
	}
}

void CLevelScriptCustom::OnPlayerAction( int32 nMatchLen, int8 nType )
{
	if( m_strPlayerAction.length() )
	{
		CLuaMgr::Inst().Load( m_strPlayerAction );
		CLuaMgr::Inst().PushLua( nMatchLen );
		CLuaMgr::Inst().PushLua( nType );
		CLuaMgr::Inst().Call( 2, 0 );
	}
}

void CLevelScriptCustom::OnAlert( CPawn* pTriggeredPawn, const TVector2<int32>& pawnOfs )
{
	if( m_strAlert.length() )
	{
		CLuaMgr::Inst().Load( m_strAlert );
		CLuaMgr::Inst().PushLua( pTriggeredPawn );
		CLuaMgr::Inst().PushLua( pawnOfs.x );
		CLuaMgr::Inst().PushLua( pawnOfs.y );
		CLuaMgr::Inst().Call( 3, 0 );
	}
}

int32 CLevelScriptCustom::Signal( int32 i )
{
	if( m_strSignal.length() )
	{
		CLuaMgr::Inst().Load( m_strSignal );
		CLuaMgr::Inst().PushLua( i );
		CLuaMgr::Inst().Call( 1, 1 );
		return CLuaMgr::Inst().PopLuaValue<int32>();
	}
	return 0;
}

void CCommonLink::OnAddedToStage()
{
	auto pLevel = GetStage()->GetMasterLevel()->GetCurLevel();
	pLevel->RegisterBegin( &m_onBegin );
	pLevel->RegisterUpdate1( &m_onUpdate );
}

void CCommonLink::OnRemovedFromStage()
{
	if( m_nTargetEffectType == 2 )
	{
		auto pPawn = SafeCast<CPawn>( m_pSrc.GetPtr() );
		if( pPawn )
			pPawn->DecSpecialState( CPawn::eSpecialState_Frenzy );
		pPawn = SafeCast<CPawn>( m_pDst.GetPtr() );
		if( pPawn )
			pPawn->DecSpecialState( CPawn::eSpecialState_Frenzy );
	}
	else
	{
		if( m_bSrcKilled )
		{
			auto pPawn = SafeCast<CPawn>( m_nTargetEffectType == 1 ? m_pDst.GetPtr() : m_pSrc.GetPtr() );
			if( pPawn )
				pPawn->DecSpecialState( CPawn::eSpecialState_Frenzy );
		}
		if( m_bDstKilled )
		{
			auto pPawn = SafeCast<CPawn>( m_nTargetEffectType == 1 ? m_pSrc.GetPtr() : m_pDst.GetPtr() );
			if( pPawn )
				pPawn->DecSpecialState( CPawn::eSpecialState_Frenzy );
		}
	}
	if( m_onBegin.IsRegistered() )
		m_onBegin.Unregister();
	if( m_onUpdate.IsRegistered() )
		m_onUpdate.Unregister();
	if( m_onSrcKilled.IsRegistered() )
		m_onSrcKilled.Unregister();
	if( m_onDstKilled.IsRegistered() )
		m_onDstKilled.Unregister();
	if( m_pSound )
	{
		m_pSound->FadeOut( 0.5f );
		m_pSound = NULL;
	}
}

void CCommonLink::Begin()
{
	auto pLevel = GetStage()->GetMasterLevel()->GetCurLevel();
	if( !m_pSrc || !m_pDst )
		return;
	auto pPawn = SafeCast<CPawn>( m_pSrc.GetPtr() );
	if( pPawn )
	{
		if( pPawn->IsKilled() )
		{
			if( m_nKillType <= 1 )
				m_pSrc = NULL;
			else
				m_bSrcKilled = true;
		}
		else
			pPawn->RegisterKilled( &m_onSrcKilled );
	}
	else
		m_pSrc->RegisterEntityEvent( eEntityEvent_RemovedFromStage, &m_onSrcKilled );
	pPawn = SafeCast<CPawn>( m_pDst.GetPtr() );
	if( pPawn )
	{
		if( pPawn->IsKilled() )
		{
			if( m_nKillType == 0 )
				m_pDst = NULL;
			else
				m_bDstKilled = true;
		}
		else
			pPawn->RegisterKilled( &m_onDstKilled );
	}
	else
		m_pDst->RegisterEntityEvent( eEntityEvent_RemovedFromStage, &m_onDstKilled );
	m_pSound = PlaySoundLoop( "electric1" );
	if( m_bSrcKilled && m_bDstKilled )
		m_pSrc = m_pDst = NULL;
	if( m_pSrc && m_pDst )
	{
		if( m_nTargetEffectType == 2 )
		{
			auto pPawn = SafeCast<CPawn>( m_pSrc.GetPtr() );
			if( pPawn )
				pPawn->IncSpecialState( CPawn::eSpecialState_Frenzy );
			pPawn = SafeCast<CPawn>( m_pDst.GetPtr() );
			if( pPawn )
				pPawn->IncSpecialState( CPawn::eSpecialState_Frenzy );
		}
		else
		{
			if( m_bSrcKilled )
			{
				auto pPawn = SafeCast<CPawn>( m_nTargetEffectType == 1 ? m_pDst.GetPtr() : m_pSrc.GetPtr() );
				if( pPawn )
					pPawn->IncSpecialState( CPawn::eSpecialState_Frenzy );
			}
			if( m_bDstKilled )
			{
				auto pPawn = SafeCast<CPawn>( m_nTargetEffectType == 1 ? m_pSrc.GetPtr() : m_pDst.GetPtr() );
				if( pPawn )
					pPawn->IncSpecialState( CPawn::eSpecialState_Frenzy );
			}
		}
	}
	else
		m_bSrcKilled = m_bDstKilled = false;
}

void CCommonLink::Update()
{
	if( !m_pSrc || !m_pDst )
	{
		SetParentEntity( NULL );
		return;
	}

	if( !m_nTick )
	{
		CVector2 p[2] = { m_srcOfs, m_dstOfs };
		CEntity* pEntity[2] = { m_pSrc, m_pDst };
		for( int i = 0; i < 2; i++ )
		{
			auto pPawn = SafeCast<CPawn>( pEntity[i] );
			if( pPawn )
			{
				auto p0 = TVector2<int32>( pPawn->GetWidth(), pPawn->GetHeight() ) + pPawn->GetPos() + pPawn->GetMoveTo();
				p[i] = p[i] + CVector2( p0.x * 0.5f, p0.y * 0.5f ) * LEVEL_GRID_SIZE;
			}
			else
				p[i] = p[i] + pEntity[i]->globalTransform.GetPosition();
		}

		auto pLightning = SafeCast<CLightningEffect>( m_pLightningPrefab->GetRoot()->CreateInstance() );
		pLightning->SetParentBeforeEntity( this );
		pLightning->SetPosition( p[0] );
		auto ofs = p[1] - p[0];
		auto p1 = TVector2<int32>( floor( ofs.x / 8 + 0.5f ), floor( ofs.y / 8 + 0.5f ) );
		pLightning->Set( p1, m_nEftLife, m_fEftStrength );
		m_nTick = m_nEftInterval;
	}
	m_nTick--;
}

void CCommonLink::OnSrcKilled()
{
	if( m_nKillType <= 1 )
		SetParentEntity( NULL );
	else
	{
		if( m_bSrcKilled )
			return;
		m_bSrcKilled = true;
		if( m_nTargetEffectType == 0 )
		{
			auto pPawn = SafeCast<CPawn>( m_pSrc.GetPtr() );
			if( pPawn )
				pPawn->IncSpecialState( CPawn::eSpecialState_Frenzy );
		}
		if( m_nTargetEffectType == 1 )
		{
			auto pPawn = SafeCast<CPawn>( m_pDst.GetPtr() );
			if( pPawn )
				pPawn->IncSpecialState( CPawn::eSpecialState_Frenzy );
		}
		if( m_bDstKilled )
			SetParentEntity( NULL );
	}
}

void CCommonLink::OnDstKilled()
{
	if( m_nKillType == 0 )
		SetParentEntity( NULL );
	else
	{
		if( m_bDstKilled )
			return;
		m_bDstKilled = true;
		if( m_nTargetEffectType == 0 )
		{
			auto pPawn = SafeCast<CPawn>( m_pDst.GetPtr() );
			if( pPawn )
				pPawn->IncSpecialState( CPawn::eSpecialState_Frenzy );
		}
		if( m_nTargetEffectType == 1 )
		{
			auto pPawn = SafeCast<CPawn>( m_pSrc.GetPtr() );
			if( pPawn )
				pPawn->IncSpecialState( CPawn::eSpecialState_Frenzy );
		}
		if( m_bSrcKilled )
			SetParentEntity( NULL );
	}
}

void CPawnUsageCommon::UseHit( class CPlayer* pPlayer )
{
	switch( m_nType )
	{
	case 0:
		pPlayer->SetHp( pPlayer->GetMaxHp() );
		break;
	case 1:
	{
		auto pEquipment = pPlayer->GetEquipment( ePlayerEquipment_Ranged );
		if( pEquipment )
			pEquipment->SetAmmo( pEquipment->GetMaxAmmo() );
		break;
	}
	}
}

void CPawnUsageButton::OnAddedToStage()
{
	if( static_cast<CImage2D*>( GetRenderObject() )->GetParamCount() )
		m_param0 = static_cast<CImage2D*>( GetRenderObject() )->GetParam()[0];
}

void CPawnUsageButton::UseHit( class CPlayer* pPlayer )
{
	bool bOK = false;
	if( m_pTarget )
		bOK = SafeCastToInterface<ISignalObj>( m_pTarget.GetPtr() )->Signal( m_nSignal );
	m_bEftFramesType = bOK;
	m_nEftFramesLeft = bOK ? m_nEftFrames1 : m_nEftFrames0;
	if( bOK )
		PlaySoundEffect( "btn" );
	else
		PlaySoundEffect( "btn_error" );
}

void CPawnUsageButton::Update()
{
	if( static_cast<CImage2D*>( GetRenderObject() )->GetParamCount() )
	{
		float t = m_nEftFramesLeft * 1.0f / ( m_bEftFramesType ? m_nEftFrames1 : m_nEftFrames0 );
		t = t * t;
		static_cast<CImage2D*>( GetRenderObject() )->GetParam()[0] = m_param0 + ( ( m_bEftFramesType ? m_eftParam1 : m_eftParam0 ) - m_param0 ) * t;
	}
	if( m_nEftFramesLeft )
		m_nEftFramesLeft--;
}

void CPawnAIAutoDoor::OnInit()
{
	auto pPawn = SafeCast<CPawn>( GetParentEntity() );
	if( m_strBrokenKey.length() )
	{
		if( GetStage()->GetMasterLevel()->EvaluateKeyInt( m_strBrokenKey ) )
			pPawn->ChangeState( eState_Broken, true );
	}
}

int32 CPawnAIAutoDoor::CheckAction( int8& nCurDir )
{
	auto pPawn = SafeCast<CPawn>( GetParentEntity() );
	auto pLevel = pPawn->GetLevel();
	auto pGrid = pLevel->GetGrid( pPawn->GetPos() );
	bool bOpen;
	bool bBlockPlayerClose = false;
	if( m_nType )
		bOpen = GetStage()->GetMasterLevel()->EvaluateKeyInt( m_strOpenCondition );
	else
		bOpen = !pLevel->IsGridBlockedExit( pGrid, true );
	bool bBlockPlayer = false;
	if( m_strBlockPlayerCondition.length() )
	{
		bBlockPlayer = GetStage()->GetMasterLevel()->EvaluateKeyInt( m_strBlockPlayerCondition );
		if( bBlockPlayer && bOpen )
		{
			auto pPlayer = pPawn->GetLevel()->GetPlayer();
			if( pPlayer && !pPlayer->IsHidden() )
			{
				if( pPlayer->GetCurStateDest() == pPawn->GetPos() )
				{
					bBlockPlayerClose = true;
					bOpen = false;
				}
			}
		}
	}
	if( pPawn->GetCurStateIndex() == eState_Open )
	{
		if( bBlockPlayer && m_strBrokenKey.length() )
		{
			auto p = pLevel->FindPickUp( pPawn->GetPos(), 2, 1 );
			if( p )
			{
				pLevel->RemovePawn( p );
				GetStage()->GetMasterLevel()->SetKeyInt( m_strBrokenKey, 1 );
				return eState_Closing_Broken;
			}
		}
		if( !pGrid->pPawn0 && !bOpen )
		{
			if( bBlockPlayerClose )
			{
				if( m_strOnBlockPlayerClose.length() )
					CLuaMgr::GetCurLuaState()->Run( m_strOnBlockPlayerClose );
			}
			return eState_Closing;
		}
	}
	else if( pPawn->GetCurStateIndex() == eState_Close )
	{
		if( bOpen )
			return eState_Opening;
		else if( pGrid->pPawn0 && pGrid->pPawn0 != pPawn )
			return eState_Opening;
	}
	return -1;
}

void CPawnAIAutoDoor::CreateIconData( CPrefabNode* pNode, const char* szCondition0, TArray<SMapIconData>& arrData ) const
{
	if( m_nType == 0 )
		return;
	CVector2 p( floor( pNode->x / LEVEL_GRID_SIZE_X + 0.5f ), floor( pNode->y / LEVEL_GRID_SIZE_Y + 0.5f ) );
	arrData.Resize( arrData.Size() + 2 );
	auto& item = arrData[arrData.Size() - 2];
	item.ofs = p;
	item.nDir = pNode->GetPatchedNode()->GetStaticDataSafe<CPawn>()->GetInitDir();
	item.nTexX = m_nStateMapIconX[0];
	item.nTexY = m_nStateMapIconY[0];
	item.bKeepSize = false;
	item.strTag = "";
	auto pSpawnData = pNode->GetStaticDataSafe<CLevelSpawnHelper>();
	if( szCondition0[0] )
	{
		item.arrFilter.Resize( item.arrFilter.Size() + 1 );
		item.arrFilter[item.arrFilter.Size() - 1] = szCondition0;
	}
	if( pSpawnData && pSpawnData->GetSpawnCondition().length() )
	{
		item.arrFilter.Resize( item.arrFilter.Size() + 1 );
		item.arrFilter[item.arrFilter.Size() - 1] = pSpawnData->GetSpawnCondition();
	}
	if( m_strOpenCondition.length() )
		item.strCondition = m_strOpenCondition;
	else
		item.strCondition = "=false";
	item.nConditionValue = 0;

	auto& item1 = arrData[arrData.Size() - 1];
	item1.ofs = p;
	item1.nDir = item.nDir;
	item1.nTexX = m_nStateMapIconX[1];
	item1.nTexY = m_nStateMapIconY[1];
	item1.bKeepSize = false;
	item1.strTag = "";
	item1.arrFilter = item.arrFilter;
	item1.strCondition = item.strCondition;
	item1.nConditionValue = 1;
}

void CPawnAIBot::OnInit()
{
	auto pPawn = SafeCast<CPawn>( GetParentEntity() );;
	pPawn->RegisterSignal( &m_onSignal );
}

int32 CPawnAIBot::CheckAction( int8& nCurDir )
{
	return 0;
}

void CPawnAIBot::CreateIconData( CPrefabNode * pNode, const char * szCondition0, TArray<SMapIconData>& arrData ) const
{
}

void CPawnAIBot::OnSignal( int32 i )
{
}

void CSeal::Update()
{
	if( m_nCurState == 0 )
	{
		auto pPlayer = GetLevel()->GetPlayer();
		if( pPlayer )
		{
			if( pPlayer->GetPos() == GetPos() || pPlayer->GetMoveTo() == GetPos() )
			{
				ChangeState( 1, false );
				if( m_strStateKey )
					GetStage()->GetMasterLevel()->SetKeyInt( m_strStateKey, 1 );
				if( m_strStateChangeScript.c_str() )
				{
					auto pLuaState = CLuaMgr::GetCurLuaState();
					pLuaState->Load( m_strStateChangeScript );
					pLuaState->PushLua( 1 );
					pLuaState->Call( 1, 0 );
				}
			}
		}
	}
	else if( m_nCurState == 2 )
	{
		if( GetLevel()->PawnTransform( this, 1, TVector2<int32>( 0, 0 ) ) )
			ChangeState( 4, false );
	}
	CPawn::Update();
}

int32 CSeal::Signal( int32 i )
{
	if( m_nCurState == 0 )
	{
		ChangeState( 3, false );
		if( m_strStateKey )
			GetStage()->GetMasterLevel()->SetKeyInt( m_strStateKey, 2 );
		if( m_strStateChangeScript.c_str() )
		{
			auto pLuaState = CLuaMgr::GetCurLuaState();
			pLuaState->Load( m_strStateChangeScript );
			pLuaState->PushLua( 2 );
			pLuaState->Call( 1, 0 );
		}
		return 1;
	}
	return 0;
}

void CSeal::CreateIconData( CPrefabNode* pNode, const char* szCondition0, TArray<SMapIconData>& arrData ) const
{
	CVector2 p( floor( pNode->x / LEVEL_GRID_SIZE_X + 0.5f ), floor( pNode->y / LEVEL_GRID_SIZE_Y + 0.5f ) );
	arrData.Resize( arrData.Size() + 3 );
	auto& item = arrData[arrData.Size() - 3];
	item.ofs = p;
	item.nDir = pNode->GetPatchedNode()->GetStaticDataSafe<CPawn>()->GetInitDir();
	item.nTexX = m_nStateMapIconX[0];
	item.nTexY = m_nStateMapIconY[0];
	item.bKeepSize = false;
	item.strTag = "";
	auto pSpawnData = pNode->GetStaticDataSafe<CLevelSpawnHelper>();
	if( szCondition0[0] )
	{
		item.arrFilter.Resize( item.arrFilter.Size() + 1 );
		item.arrFilter[item.arrFilter.Size() - 1] = szCondition0;
	}
	if( pSpawnData && pSpawnData->GetSpawnCondition().length() )
	{
		item.arrFilter.Resize( item.arrFilter.Size() + 1 );
		item.arrFilter[item.arrFilter.Size() - 1] = pSpawnData->GetSpawnCondition();
	}
	item.strCondition = m_strStateKey;
	item.nConditionValue = 0;
	for( int i = 1; i < 3; i++ )
	{
		auto& item1 = arrData[arrData.Size() - 3 + i];
		item1.ofs = p;
		item1.nDir = item.nDir;
		item1.nTexX = m_nStateMapIconX[i];
		item1.nTexY = m_nStateMapIconY[i];
		item1.bKeepSize = false;
		item1.strTag = "";
		item1.arrFilter = item.arrFilter;
		item1.strCondition = item.strCondition;
		item1.nConditionValue = i;
	}
}

void CSeal::InitState()
{
	if( m_strStateKey )
	{
		m_bUseInitState = true;
		m_nInitState = GetStage()->GetMasterLevel()->EvaluateKeyInt( m_strStateKey );
	}
	CPawn::InitState();
}

void CHitButton::Init()
{
	if( m_strRepairedKey.length() )
	{
		if( GetStage()->GetMasterLevel()->EvaluateKeyInt( m_strRepairedKey ) )
			m_bReady = true;
	}
	else
		m_bReady = true;
	CPawn::Init();
}

void CHitButton::OnPreview()
{
	if( m_strRepairedKey.length() )
	{
		m_bUseInitState = true;
		m_nInitState = m_nBrokenState;
	}
	CPawn::OnPreview();
}

int32 CHitButton::Signal( int32 i )
{
	if( !m_bReady )
	{
		m_bReady = true;
		ChangeState( m_arrTransferStates[m_nCur], false );
		if( m_strRepairedKey.length() )
			GetStage()->GetMasterLevel()->SetKeyInt( m_strRepairedKey, 1 );
		return 1;
	}
	return 0;
}

int32 CHitButton::Damage( int32 nDamage, int8 nDamageType, TVector2<int32> hitOfs )
{
	if( !m_bReady )
		return 0;
	auto curState = GetCurState();
	if( curState.nTotalTicks )
		return 0;
	auto nMaxState = m_arrStates.Size();
	if( hitOfs.x >= 0 )
		m_nCur = m_nCur == 0 ? nMaxState - 1 : m_nCur - 1;
	else
		m_nCur = m_nCur == nMaxState - 1 ? 0 : m_nCur + 1;
	ChangeState( m_arrTransferStates[m_nCur], false );
	if( m_strStateChangeSound.c_str() )
		PlaySoundEffect( m_strStateChangeSound );
	if( m_strStateKey.c_str() )
		GetStage()->GetMasterLevel()->SetKeyInt( m_strStateKey, m_nCur );
	if( m_strStateChangeScript.c_str() )
	{
		auto pLuaState = CLuaMgr::GetCurLuaState();
		pLuaState->Load( m_strStateChangeScript );
		pLuaState->PushLua( m_nCur );
		pLuaState->Call( 1, 0 );
	}
	return nDamage;
}

void CHitButton::CreateIconData( CPrefabNode* pNode, const char* szCondition0, TArray<SMapIconData>& arrData ) const
{
	if( !m_arrStates.Size() )
		return;
	CVector2 p( floor( pNode->x / LEVEL_GRID_SIZE_X + 0.5f ), floor( pNode->y / LEVEL_GRID_SIZE_Y + 0.5f ) );
	int32 nSize0 = arrData.Size() + ( m_strRepairedKey.length() ? 1 : 0 );
	arrData.Resize( nSize0 + m_arrStates.Size() );
	auto& item = arrData[nSize0];
	item.ofs = p;
	item.nDir = pNode->GetPatchedNode()->GetStaticDataSafe<CPawn>()->GetInitDir();
	item.nTexX = m_arrStateIconTexX[0];
	item.nTexY = m_arrStateIconTexY[0];
	item.bKeepSize = false;
	item.strTag = "";
	auto pSpawnData = pNode->GetStaticDataSafe<CLevelSpawnHelper>();
	if( szCondition0[0] )
	{
		item.arrFilter.Resize( item.arrFilter.Size() + 1 );
		item.arrFilter[item.arrFilter.Size() - 1] = szCondition0;
	}
	if( pSpawnData && pSpawnData->GetSpawnCondition().length() )
	{
		item.arrFilter.Resize( item.arrFilter.Size() + 1 );
		item.arrFilter[item.arrFilter.Size() - 1] = pSpawnData->GetSpawnCondition();
	}
	if( m_strRepairedKey )
	{
		auto& item0 = arrData[nSize0 - 1];
		item0.ofs = p;
		item0.nDir = item.nDir;
		item0.nTexX = m_nBrokenIconTexX;
		item0.nTexY = m_nBrokenIconTexY;
		item0.bKeepSize = false;
		item0.strTag = "";
		item0.arrFilter = item.arrFilter;
		item0.strCondition = m_strRepairedKey;
		item0.nConditionValue = 0;
		item.arrFilter.Resize( item.arrFilter.Size() + 1 );
		item.arrFilter[item.arrFilter.Size() - 1] = m_strRepairedKey;
	}
	item.strCondition = m_strStateKey;
	item.nConditionValue = 0;
	for( int i = 1; i < m_arrStates.Size(); i++ )
	{
		auto& item1 = arrData[nSize0 + i];
		item1.ofs = p;
		item1.nDir = item.nDir;
		item1.nTexX = m_arrStateIconTexX[i];
		item1.nTexY = m_arrStateIconTexY[i];
		item1.bKeepSize = false;
		item1.strTag = "";
		item1.arrFilter = item.arrFilter;
		item1.strCondition = item.strCondition;
		item1.nConditionValue = i;
	}
}

void CHitButton::InitState()
{
	m_bUseInitState = true;
	if( m_bReady )
	{
		if( m_strStateKey.c_str() )
			m_nCur = GetStage()->GetMasterLevel()->EvaluateKeyInt( m_strStateKey );
		else
			m_nCur = 0;
		m_nInitState = m_arrStates[m_nCur];
	}
	else
		m_nInitState = m_nBrokenState;
	CPawn::InitState();
}

void CConsole::Update()
{
	CPawn::Update();
	if( m_pExtra )
	{
		bool bRunning = m_pExtra->Resume( 0, 1 );
		bool bResult = m_pExtra->PopLuaValue<bool>();
		if( !bRunning )
		{
			m_pExtra = NULL;
			if( bResult )
				RunDefault();
		}
	}
	else if( m_pDefault )
	{
		if( !m_pDefault->Resume( 0, 0 ) )
			m_pDefault = NULL;
	}
}

int32 CConsole::Signal( int32 i )
{
	if( m_pDefault || m_pExtra )
		return 0;
	if( m_strExtraScript.length() )
	{
		m_pExtra = CLuaMgr::GetCurLuaState()->CreateCoroutine( m_strExtraScript );
		m_pExtra->PushLua( i );
		bool bRunning = m_pExtra->Resume( 1, 1 );
		bool bResult = m_pExtra->PopLuaValue<bool>();
		if( !bRunning )
		{
			m_pExtra = NULL;
			if( bResult )
				RunDefault();
			else
				GetLevel()->GetPlayer()->ForceUnMount();
		}
	}
	else
		RunDefault();
	return 1;
}

void CConsole::RunDefault()
{
	if( m_strDefaultScript.length() )
	{
		m_pDefault = CLuaMgr::GetCurLuaState()->CreateCoroutine( m_strDefaultScript );
		if( !m_pDefault->Resume( 0, 0 ) )
			m_pDefault = NULL;
	}
}

void CPressurePlate::Update()
{
	bool bPress = false;
	for( int i = 0; i < m_nWidth && !bPress; i++ )
	{
		for( int j = 0; j < m_nHeight && !bPress; j++ )
		{
			auto pGrid = GetLevel()->GetGrid( GetPos() );
			if( pGrid && pGrid->pPawn1 && pGrid->pPawn1->HasStateTag( m_strPressStateTag ) )
				bPress = true;
		}
	}
	bool bPress0 = m_nCurState > 0;
	if( bPress != bPress0 )
		OnPress( bPress );
	CPawnHit::Update();
}

void CPressurePlate::OnPress( bool bDown )
{
	if( m_strPressKey.length() )
		GetStage()->GetMasterLevel()->SetKeyInt( m_strPressKey, bDown ? 1 : 0 );
	if( m_strPressScript.length() )
	{
		auto pLuaState = CLuaMgr::GetCurLuaState();
		pLuaState->Load( m_strPressScript );
		pLuaState->PushLua( bDown );
		pLuaState->Call( 1, 0 );
	}
	if( m_strSound[bDown ? 1 : 0].length() )
		PlaySoundEffect( m_strSound[bDown ? 1 : 0] );
	ChangeState( bDown ? 1 : 0 );
}

void CAlarm::Update()
{
	CPawnHit::Update();
	if( !m_bTriggered )
	{
		auto pPlayer = GetLevel()->GetPlayer();
		if( pPlayer && pPlayer->GetPos() == GetPos() )
		{
			m_bTriggered = true;
			m_p[0]->bVisible = false;
			m_p[1]->bVisible = true;
			if( m_strTriggerScript.length() )
				CLuaMgr::Inst().Run( m_strTriggerScript );
			if( m_strSound.length() )
				PlaySoundEffect( m_strSound );
		}
	}
}

void CFallPoint::Init()
{
	if( !m_strKey.length() || GetStage()->GetMasterLevel()->EvaluateKeyInt( m_strKey ) )
	{
		auto p = static_cast<CImage2D*>( GetRenderObject() );
		auto texRect = p->GetElem().texRect;
		texRect.x += texRect.width;
		p->SetTexRect( texRect );
		m_bVisited = true;
	}
	CPawnHit::Init();
}

void CFallPoint::Update()
{
	auto pPlayer = GetLevel()->GetPlayer();
	if( pPlayer->GetMoveTo() == GetPos() && pPlayer->GetMoveTo() == pPlayer->GetPos() )
	{
		if( !m_bVisited )
		{
			if( m_strKey.length() )
				GetStage()->GetMasterLevel()->SetKeyInt( m_strKey, 1 );
			auto p = static_cast<CImage2D*>( GetRenderObject() );
			auto texRect = p->GetElem().texRect;
			texRect.x += texRect.width;
			p->SetTexRect( texRect );
			m_bVisited = true;
			AddChild( m_pEft->GetRoot()->CreateInstance() );
			PlaySoundEffect( m_strSound );
		}
		auto& nxtStage = GetLevel()->GetNextLevelData( m_nNxtStage );
		GetStage()->GetMasterLevel()->TransferTo1( nxtStage.pNxtStage, pPlayer->GetMoveTo() -
			TVector2<int32>( nxtStage.nOfsX, nxtStage.nOfsY ), pPlayer->GetCurDir(), 1 );
	}
}

void CClimbPoint::Init()
{
	if( m_strKey.length() )
	{
		if( GetStage()->GetMasterLevel()->EvaluateKeyInt( m_strKey ) )
			m_bReady = true;
	}
	else
		m_bReady = true;
	CPawnHit::Init();
}

int32 CClimbPoint::Signal( int32 i )
{
	if( m_bReady )
	{
		auto& nxtStage = GetLevel()->GetNextLevelData( m_nNxtStage );
		GetStage()->GetMasterLevel()->TransferTo1( nxtStage.pNxtStage, GetMoveTo() -
			TVector2<int32>( nxtStage.nOfsX, nxtStage.nOfsY ) + TVector2<int32>( GetCurDir() ? -2 : 2, 0 ), GetCurDir(), 2 );
	}
	else
	{
		m_bReady = true;
		ChangeState( 1 );
		if( m_strKey.length() )
			GetStage()->GetMasterLevel()->SetKeyInt( m_strKey, 1 );
	}
	return 1;
}

void CClimbPoint::CreateIconData( CPrefabNode* pNode, const char* szCondition0, TArray<SMapIconData>& arrData ) const
{
	CVector2 p( floor( pNode->x / LEVEL_GRID_SIZE_X + 0.5f ), floor( pNode->y / LEVEL_GRID_SIZE_Y + 0.5f ) );
	arrData.Resize( arrData.Size() + 2 );
	auto& item = arrData[arrData.Size() - 2];
	item.ofs = p;
	item.nDir = pNode->GetPatchedNode()->GetStaticDataSafe<CPawn>()->GetInitDir();
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
	if( pSpawnData && pSpawnData->GetSpawnCondition().length() )
	{
		item.arrFilter.Resize( item.arrFilter.Size() + 1 );
		item.arrFilter[item.arrFilter.Size() - 1] = pSpawnData->GetSpawnCondition();
	}
	item.strCondition = m_strKey;
	item.nConditionValue = 0;

	auto& item1 = arrData[arrData.Size() - 1];
	item1.ofs = p;
	item1.nDir = item.nDir;
	item1.nTexX = m_nMapIconTexX[1];
	item1.nTexY = m_nMapIconTexY[1];
	item1.bKeepSize = m_bMapIconKeepSize[1];
	item1.strTag = m_strMapIconTag[1];
	item1.arrFilter = item.arrFilter;
	item1.strCondition = item.strCondition;
	item1.nConditionValue = 1;
}

int32 CClimbPoint::GetDefaultState()
{
	return m_bReady ? 1 : 0;
}

int32 CHeavyDoor::Signal( int32 i )
{
	auto pPlayer = GetLevel()->GetPlayer();
	auto& nxtStage = GetLevel()->GetNextLevelData( m_nNxtStage );
	GetStage()->GetMasterLevel()->TransferTo1( nxtStage.pNxtStage, GetMoveTo() - TVector2<int32>( nxtStage.nOfsX, nxtStage.nOfsY )
		+ TVector2<int32>( pPlayer->GetCurDir() ? -2 : 2, 0 ), 1 - pPlayer->GetCurDir(), 3 );
	return 1;
}

void CSmoke::OnRemovedFromStage()
{
	if( m_pSound )
	{
		m_pSound->FadeOut( 0.5f );
		m_pSound = NULL;
	}
	CPawnHit::OnRemovedFromStage();
}

void CSmoke::Init()
{
	CPawnHit::Init();
	InitImages();
	if( m_bImg )
		UpdateImages();
	if( m_pLightningEft )
	{
		SafeCast<CLightningEffect>( m_pLightningEft.GetPtr() )->Set( TVector2<int32>( 0, 40 ), m_nEftInterval, 2, 0.05f );
		m_pSound = PlaySoundLoop( "electric1" );
	}
}

void CSmoke::Update()
{
	CPawnHit::Update();
	if( m_bImg )
	{
		UpdateImages();
		m_t += m_fAnimSpeed;
		m_t -= floor( m_t );
		for( int i = 0; i < 3; i++ )
		{
			m_items[i].tex.x += m_items[i].fTexSpeed;
			m_items[i].tex.x -= floor( m_items[i].tex.x );
		}
	}
	if( m_pLightningEft && !m_pLightningEft->GetParentEntity() )
	{
		m_pLightningEft->SetParentEntity( this );
		SafeCast<CLightningEffect>( m_pLightningEft.GetPtr() )->Set( TVector2<int32>( 0, 40 ), m_nEftInterval, 2, 0.05f );
	}

	auto pPlayer = GetLevel()->GetPlayer();
	if( pPlayer->GetMoveTo() == GetPos() || pPlayer->GetPos() == GetPos() )
		GetLevel()->Fail( 1 );
}

void CSmoke::Render( CRenderContext2D& context )
{
	if( !m_bImg )
		return;
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

void CSmoke::UpdateRendered( double dTime )
{
	if( m_bPreview )
	{
		if( m_bImg )
		{
			UpdateImages();
			m_t += m_fAnimSpeed;
			m_t -= floor( m_t );
			for( int i = 0; i < 3; i++ )
			{
				m_items[i].tex.x += m_items[i].fTexSpeed;
				m_items[i].tex.x -= floor( m_items[i].tex.x );
			}
		}
		if( m_pLightningEft )
			SafeCast<CLightningEffect>( m_pLightningEft.GetPtr() )->Update();
	}
}

void CSmoke::OnPreview()
{
	m_bPreview = true;
	InitImages();
	if( m_bImg )
		UpdateImages();
	if( m_pLightningEft )
		SafeCast<CLightningEffect>( m_pLightningEft.GetPtr() )->Set( TVector2<int32>( 0, 40 ), 0, 2, 0.05f );
}

void CSmoke::InitImages()
{
	auto pImg = static_cast<CImage2D*>( GetRenderObject() );
	if( !pImg )
		return;
	m_bImg = true;
	auto pParam = pImg->GetParam();
	m_origParam[0] = pParam[0];
	m_origParam[1] = pParam[1];
	SetRenderObject( NULL );
	m_fSplitOfs[0] = SRand::Inst().Rand( 0.0f, 0.4f );
	m_fSplitOfs[1] = m_fSplitOfs[0] + SRand::Inst().Rand( 0.4f, 0.6f );
	for( int i = 0; i < 3; i++ )
	{
		m_items[i].tex = CVector2( SRand::Inst<eRand_Render>().Rand( 0.0f, 1.0f ), SRand::Inst<eRand_Render>().Rand( 0.0f, 1.0f ) );
		m_items[i].fTexSpeed = SRand::Inst<eRand_Render>().Rand( 0.003f, 0.004f ) * ( SRand::Inst<eRand_Render>().Rand( 0, 2 ) ? 1 : -1 );
		m_items[i].ofs = CVector2( SRand::Inst<eRand_Render>().Rand( 1, 5 ) * 2 * ( SRand::Inst<eRand_Render>().Rand( 0, 2 ) ? 1 : -1 ),
			SRand::Inst<eRand_Render>().Rand( -1, 2 ) * 2 );
		m_elems[i].nInstDataSize = sizeof( CVector4 ) * 2;
		m_elems[i].pInstData = &m_params[i * 2];
	}
	m_fAnimSpeed = SRand::Inst<eRand_Render>().Rand( 0.01f, 0.015f );
	SetLocalBound( CRectangle( 0, 0, 48, 32 ) );
}

void CSmoke::UpdateImages()
{
	float y[2] = { m_t + m_fSplitOfs[0], m_t + m_fSplitOfs[1] };
	for( int i = 0; i < 2; i++ )
		y[i] = floor( y[i] * 16 + 0.5f ) * 2;
	if( y[1] <= 32 )
	{
		m_elems[0].rect = CRectangle( 0, 0, 48, y[0] );
		m_elems[1].rect = CRectangle( 0, y[0], 48, y[1] - y[0] );
		m_elems[2].rect = CRectangle( 0, y[1], 48, 32 - y[1] );
	}
	else if( y[0] < 32 )
	{
		m_elems[2].rect = CRectangle( 0, 0, 48, y[1] - 32 );
		m_elems[0].rect = CRectangle( 0, y[1] - 32, 48, y[0] - y[1] + 32 );
		m_elems[1].rect = CRectangle( 0, y[0], 48, 32 - y[0] );
	}
	else
	{
		m_elems[1].rect = CRectangle( 0, 0, 48, y[0] - 32 );
		m_elems[2].rect = CRectangle( 0, y[0] - 32, 48, y[1] - y[0] );
		m_elems[0].rect = CRectangle( 0, y[1] - 32, 48, 64 - y[1] );
	}
	const float fTexSize = 128.0f;
	for( int i = 0; i < 3; i++ )
	{
		auto tex = m_items[i].tex * fTexSize;
		tex = CVector2( floor( tex.x + 0.5f ), floor( tex.y + 0.5f ) );
		m_elems[i].texRect = CRectangle( tex.x, tex.y, m_elems[i].rect.width * 0.5f, m_elems[i].rect.height * 0.5f ) / fTexSize;
		m_params[i * 2] = CVector4( m_origParam[0].x, m_origParam[0].y, m_origParam[0].z, m_items[i].ofs.x );
		m_params[i * 2 + 1] = CVector4( m_origParam[1].x, m_origParam[1].y, m_origParam[1].z, m_items[i].ofs.y );
	}
}

void CElevator::OnAddedToStage()
{
	auto pImg = static_cast<CImage2D*>( GetRenderObject() );
	pImg->bVisible = false;
	if( pImg->GetParamCount() )
		m_param0 = pImg->GetParam()[0];
	m_origTexRect = pImg->GetElem().texRect;
}

void CElevator::OnInit( CMyLevel* pLevel )
{
	m_pLevel = pLevel;
	auto nCount = pLevel->GetNextLevelCount();
	m_nFloorBegin = Min( m_nFloorBegin, nCount - 1 );
	if( m_nFloorCount <= 0 )
		m_nFloorCount = nCount - m_nFloorCount;

	auto nLastLevel = pLevel->FindNextLevelIndex( GetStage()->GetMasterLevel()->GetLastLevelName() );
	m_nCurFloor = Max( 0, Min( m_nFloorCount - 1, nLastLevel - m_nFloorBegin ) );
	OnCurFloorChanged();
}

void CElevator::OnUpdate1( CMyLevel* pLevel )
{
	bool bBlocked = pLevel->IsExitBlocked( m_nRedirect );
	auto param0 = bBlocked ? m_invalidParam : m_param0;
	auto pImg = static_cast<CImage2D*>( GetRenderObject() );
	pImg->bVisible = true;
	pImg->SetTexRect( m_origTexRect.Offset( CVector2( m_origTexRect.width * m_nCurFloor, 0 ) ) );
	if( pImg->GetParamCount() )
	{
		float t = m_nEftFramesLeft * 1.0f / m_nEftFrames;
		static_cast<CImage2D*>( GetRenderObject() )->GetParam()[0] = param0 + ( m_eftParam - param0 ) * t;
	}
	if( m_nEftFramesLeft )
		m_nEftFramesLeft--;
}

int32 CElevator::Signal( int32 i )
{
	if( i >= 0 )
	{
		if( i >= m_nFloorCount || i == m_nCurFloor )
			return 0;
		m_nCurFloor = i;
	}
	else if( i == -1 )
	{
		if( !m_nCurFloor )
			return 0;
		m_nCurFloor--;
	}
	else if( i == -2 )
	{
		if( m_nCurFloor >= m_nFloorCount - 1 )
			return 0;
		m_nCurFloor++;
	}
	else if( i == -3 )
		return m_nCurFloor;
	else
		return 0;
	OnCurFloorChanged();
	return 1;
}

void CElevator::OnCurFloorChanged()
{
	m_pLevel->Redirect( m_nRedirect, m_nCurFloor + m_nFloorBegin );
	m_nEftFramesLeft = m_nEftFrames;
}

void CProjector::OnUpdate1( CMyLevel* pLevel )
{
	if( !m_bSetTarget && ( m_nFixedTarget < 0 || m_nFixedTarget >= m_arrFixedTargets.Size() ) )
	{
		if( m_p )
			m_p->bVisible = false;
		if( m_p1 )
			m_p1->bVisible = false;
		return;
	}

	auto target = m_bSetTarget ? m_target : m_arrFixedTargets[m_nFixedTarget];
	if( m_p )
	{
		m_p->bVisible = true;
		m_p->SetPosition( target );
	}
	if( m_p1 )
	{
		m_p1->bVisible = true;
		auto p1 = target + m_projTargetOfs;
		int32 i = 0;
		for( auto pChild = m_p1->Get_ChildEntity(); pChild && i < m_arrProjPos.Size(); pChild = pChild->NextChildEntity() )
		{
			if( pChild->GetName() != "item" )
				continue;
			auto p2 = ( p1 - m_p1->GetPosition() ) * m_arrProjPos[i];
			pChild->SetPosition( CVector2( floor( p2.x * 0.5f + 0.5f ) * 2, floor( p2.y * 0.5f + 0.5f ) * 2 ) );
			i++;
		}
	}
}

void CProjector::SetTarget( const CVector2& target )
{
	bool b0 = IsActivated();
	m_bSetTarget = true;
	m_target = target;
	m_nFixedTarget = -1;
	bool b1 = IsActivated();
	if( b1 && !b0 )
		PlaySoundEffect( "activate" );
	else if( !b1 && b0 )
		PlaySoundEffect( "deactivate" );
}

bool CProjector::IsActivated()
{
	return m_bSetTarget || m_nFixedTarget >= 0 && m_nFixedTarget < m_arrFixedTargets.Size();
}

int32 CProjector::Signal( int32 i )
{
	bool b0 = IsActivated();
	m_bSetTarget = false;
	m_nFixedTarget = i;
	bool b1 = IsActivated();
	if( b1 && !b0 )
		PlaySoundEffect( "activate" );
	else if( !b1 && b0 )
		PlaySoundEffect( "deactivate" );
	return 1;
}

void CTutorialMoving::OnUpdate1( CMyLevel* pLevel )
{
	if( !m_nType )
		return;
	m_p->bVisible = true;
	m_p1->bVisible = true;
	auto pPlayer = pLevel->GetPlayer();
	auto& curState = pPlayer->GetCurState();
	auto pt = CVector2( pPlayer->GetMoveTo().x, pPlayer->GetMoveTo().y ) * LEVEL_GRID_SIZE;
	m_p->SetPosition( pt );
	auto nDir = pPlayer->GetCurDir();
	for( int i = 0; i < 4; i++ )
	{
		m_pImgs[i]->bVisible = nDir == 0;
		m_pImgs[i + 4]->bVisible = nDir == 1;
	}
	CVector2 ofs[] = { { 2, 0 }, { 1, 1 }, { 1, -1 }, { -2, 0 }, { -1, 1 }, { -1, -1 } };

	if( m_nState == 0 )
	{
		if( pPlayer->GetCurStateTick() == 1 && curState.strName != "stand" && curState.strName != "break" )
		{
			bool bSucceed = true;
			if( m_nType == 1 )
			{
				auto c = m_str[m_nCurStep];
				switch( c )
				{
				case '6':
					bSucceed = curState.strName == "move_x" && nDir == 0;
					break;
				case '4':
					bSucceed = curState.strName == "move_x" && nDir == 1;
					break;
				case '9':
					bSucceed = curState.strName == "move_up" && nDir == 0;
					break;
				case '7':
					bSucceed = curState.strName == "move_up" && nDir == 1;
					break;
				case '3':
					bSucceed = curState.strName == "move_down" && nDir == 0;
					break;
				case '1':
					bSucceed = curState.strName == "move_down" && nDir == 1;
				default:
					break;
				}
				if( !bSucceed )
				{
					pPlayer->PlayStateSetDir( "break", m_nLastDir );
					if( m_strFailedScript.length() )
					{
						auto pLuaState = CLuaMgr::GetCurLuaState();
						pLuaState->Run( m_strFailedScript );
					}
				}
			}

			if( bSucceed )
			{
				if( curState.strName == "move_x" )
					m_nState = 1 + 3 * nDir;
				else if( curState.strName == "move_up" )
					m_nState = 2 + 3 * nDir;
				else if( curState.strName == "move_down" )
					m_nState = 3 + 3 * nDir;
				m_nLastDir = pPlayer->GetCurDir();
			}
		}
		if( m_nState )
			m_nStateTick = 10;
		else
		{
			m_pImg1->bVisible = false;
			auto pInputResult = pPlayer->GetCurInputResult();
			if( pInputResult )
			{
				auto nState1 = 0;
				if( pInputResult->strStateName == "move_x" )
					nState1 = 1 + 3 * ( pInputResult->bInverse ? 1 - nDir : nDir );
				else if( pInputResult->strStateName == "move_up" )
					nState1 = 2 + 3 * ( pInputResult->bInverse ? 1 - nDir : nDir );
				else if( pInputResult->strStateName == "move_down" )
					nState1 = 3 + 3 * ( pInputResult->bInverse ? 1 - nDir : nDir );
				if( nState1 )
				{
					m_pImg1->bVisible = true;
					m_pImg1->SetPosition( ofs[nState1 - 1] * LEVEL_GRID_SIZE );
					static_cast<CImage2D*>( m_pImg1.GetPtr() )->GetParam()[0] = CVector4( 2, 0.5, 0.25, 0 );
				}
			}
		}
	}

	m_pImgNxt->bVisible = false;
	if( m_nState )
	{
		m_pImg1->bVisible = true;
		m_pImg1->SetPosition( ofs[m_nState - 1] * LEVEL_GRID_SIZE );
		CVector4 frames[] =
		{
			{ 5, 8, 8, 0 },
			{ 0, 0, 0, 0 },
			{ 4, 7, 7, 0 },
			{ 0, 0, 0, 0 },
			{ 3, 3, 3, 0 },
			{ 2, 1, 1, 0 },
			{ 1, 0.25, 0.25, 0 },
			{ 0.5, 0.12, 0.12, 0 },
			{ 0.12, 0.06, 0.06, 0 },
			{ 0.06, 0.03, 0.03, 0 },
		};
		static_cast<CImage2D*>( m_pImg1.GetPtr() )->GetParam()[0] = frames[10 - m_nStateTick];
		m_nStateTick--;
		if( !m_nStateTick )
		{
			m_nState = 0;
			if( m_nType == 1 )
			{
				m_nCurStep++;
				if( m_nCurStep >= m_str.length() )
				{
					m_nType = 2;
					if( m_strFinishedScript )
					{
						auto pLuaState = CLuaMgr::GetCurLuaState();
						pLuaState->Run( m_strFinishedScript );
					}
				}
			}
		}
	}
	else if( m_nType == 1 )
	{
		m_pImgNxt->bVisible = true;
		auto c = m_str[m_nCurStep];
		switch( c )
		{
		case '6':
			m_pImgNxt->SetPosition( ofs[0] * LEVEL_GRID_SIZE );
			break;
		case '4':
			m_pImgNxt->SetPosition( ofs[3] * LEVEL_GRID_SIZE );
			break;
		case '9':
			m_pImgNxt->SetPosition( ofs[1] * LEVEL_GRID_SIZE );
			break;
		case '7':
			m_pImgNxt->SetPosition( ofs[4] * LEVEL_GRID_SIZE );
			break;
		case '3':
			m_pImgNxt->SetPosition( ofs[2] * LEVEL_GRID_SIZE );
			break;
		case '1':
			m_pImgNxt->SetPosition( ofs[5] * LEVEL_GRID_SIZE );
		default:
			break;
		}
	}

	auto p1 = pt + CVector2( -64, 64 );
	float t[3] = { 0.1f, 0.3f, 0.6f };
	for( int i = 0; i < 3; i++ )
	{
		auto p2 = ( p1 - m_p1->GetPosition() ) * t[i];
		m_pImg2[i]->SetPosition( CVector2( floor( p2.x * 0.5f + 0.5f ) * 2, floor( p2.y * 0.5f + 0.5f ) * 2 ) );
	}
}

int32 CTutorialMoving::Signal( int32 i )
{
	bool b0 = m_nType > 0;
	m_nType = i;
	bool b1 = m_nType > 0;
	if( b1 && !b0 )
		PlaySoundEffect( "activate" );
	else if( !b1 && b0 )
		PlaySoundEffect( "deactivate" );
	m_nLastDir = GetStage()->GetMasterLevel()->GetCurLevel()->GetPlayer()->GetCurDir();
	return 1;
}

void CTutorialFollowing::OnUpdate1( CMyLevel* pLevel )
{
	auto pPlayer = pLevel->GetPlayer();
	if( m_bError )
	{
		if( pLevel->IsScenario() )
			return;
		if( m_nEndTick )
		{
			m_nEndTick--;
			if( !m_nEndTick )
				CLuaMgr::Inst().Run( m_strFailedScript );
			return;
		}
		for( auto& p : m_vecError )
		{
			if( p == pPlayer->GetPos() || p == pPlayer->GetMoveTo() )
			{
				Fail( p );
				return;
			}
		}
		m_nErrorTick++;
		if( m_nErrorTick == 30 )
		{
			m_nErrorTick = 0;
			auto nxt = pLevel->SimpleFindPath( m_curPos, pPlayer->GetMoveTo(), 1 );
			if( nxt.x >= 0 )
			{
				auto pImg = CreateErrImg();
				m_vecImgs.push_back( pImg );
				pImg->SetPosition( CVector2( m_curPos.x, m_curPos.y ) * LEVEL_GRID_SIZE );
				m_curPos = nxt;
				m_vecError.push_back( m_curPos );
				m_pProjector->SetTarget( CVector2( m_curPos.x, m_curPos.y ) * LEVEL_GRID_SIZE );

				auto pLightning = SafeCast<CLightningEffect>( m_pFailEffect->GetRoot()->CreateInstance() );
				pLightning->SetParentEntity( this );
				pLightning->SetPosition( m_pProjector->GetProjSrc() );
				auto ofs = CVector2( m_curPos.x + 1, m_curPos.y + 0.5f ) * LEVEL_GRID_SIZE - pLightning->GetPosition();
				auto p = TVector2<int32>( floor( ofs.x / 8 + 0.5f ), floor( ofs.y / 8 + 0.5f ) );
				pLightning->Set( p, 60 );
				PlaySoundEffect( "electric" );
			}
		}
		return;
	}
	if( !m_bEnabled )
		return;
	if( !m_nState )
	{
		if( pPlayer->GetMoveTo() == TVector2<int32>( m_nBeginX, m_nBeginY ) )
		{
			m_curPos = pPlayer->GetMoveTo();
			m_nCurStep = 0;
			m_nState = 1;
			UpdateStep();
			UpdateStateImg();
		}
	}
	else if( m_nState >= 2 )
	{
		if( m_nEndTick )
		{
			m_nEndTick--;
			if( !m_nEndTick )
			{
				if( m_nState == 2 )
				{
					if( m_strFinishedScript.length() )
						CLuaMgr::Inst().Run( m_strFinishedScript );
				}
				else
				{
					if( m_strFailedScript.length() )
						CLuaMgr::Inst().Run( m_strFailedScript );
				}
			}
		}
	}
}

void CTutorialFollowing::OnPlayerChangeState( SPawnState& state, int32 nStateSource, int8 nDir )
{
	if( m_nState != 1 )
		return;
	if( state.strName == "stand" )
		return;

	auto c = m_str[m_nCurStep];
	bool bSucceed = false;
	auto newPos = m_curPos;
	switch( c )
	{
	case '6':
		bSucceed = state.strName == "move_x" && nDir == 0;
		newPos = TVector2<int32>( 2, 0 ) + m_curPos;
		break;
	case '4':
		bSucceed = state.strName == "move_x" && nDir == 1;
		newPos = TVector2<int32>( -2, 0 ) + m_curPos;
		break;
	case '9':
		bSucceed = state.strName == "move_up" && nDir == 0;
		newPos = TVector2<int32>( 1, 1 ) + m_curPos;
		break;
	case '7':
		bSucceed = state.strName == "move_up" && nDir == 1;
		newPos = TVector2<int32>( -1, 1 ) + m_curPos;
		break;
	case '3':
		bSucceed = state.strName == "move_down" && nDir == 0;
		newPos = TVector2<int32>( 1, -1 ) + m_curPos;
		break;
	case '1':
		bSucceed = state.strName == "move_down" && nDir == 1;
		newPos = TVector2<int32>( -1, -1 ) + m_curPos;
	default:
		break;
	}
	if( bSucceed )
	{
		m_curPos = newPos;
		Succeed();
	}
	else
		Fail( m_curPos );
}

void CTutorialFollowing::OnPlayerAction( int32 nMatchLen, int8 nType )
{
	if( m_nState != 1 )
		return;
	if( !nMatchLen && m_bNoStop )
		Fail( m_curPos );
}

int32 CTutorialFollowing::Signal( int32 i )
{
	if( i == 0 )
	{
		if( m_bError )
		{
			m_bError = false;
			for( CRenderObject2D* p : m_vecImgs )
				p->RemoveThis();
			m_vecImgs.resize( 0 );
			m_pProjector->Signal( -1 );
		}
		else if( m_bEnabled )
		{
			m_bEnabled = false;
			for( CRenderObject2D* p : m_vecImgs )
				p->bVisible = false;
			m_nState = 0;
			m_pProjector->Signal( -1 );
		}
		if( m_pFailEftObj )
		{
			m_pFailEftObj->SetParentEntity( NULL );
			m_pFailEftObj = NULL;
		}
	}
	else if( i == 1 )
	{
		if( m_bError )
			return 0;
		if( !m_bEnabled )
		{
			m_pProjector->SetTarget( CVector2( m_nBeginX, m_nBeginY ) * LEVEL_GRID_SIZE );
			m_bEnabled = true;
			m_nCurStep = 0;
			UpdateStateImg();
		}
	}
	else if( i == 2 )
	{
		if( m_bError )
			return 0;
		m_bError = true;
		m_nErrorTick = 0;
		m_curPos = TVector2<int32>( m_nBeginX, m_nBeginY );
		m_pProjector->SetTarget( CVector2( m_nBeginX, m_nBeginY ) * LEVEL_GRID_SIZE );
		for( CRenderObject2D* p : m_vecImgs )
			p->RemoveThis();
		m_vecImgs.resize( 0 );
		m_vecError.push_back( m_curPos );
		UpdateStateImg();
	}
	return 1;
}

void CTutorialFollowing::Succeed()
{
	do
	{
		m_nCurStep++;
	} while( m_str[m_nCurStep] == ' ' );
	UpdateStep();
	UpdateStateImg();
	if( m_strStepScript.length() )
	{
		CLuaMgr::Inst().Load( m_strStepScript );
		CLuaMgr::Inst().PushLua( m_nCurStep );
		CLuaMgr::Inst().Call( 1 );
	}
}

void CTutorialFollowing::Fail( const TVector2<int32>& pos )
{
	m_nState = 3;
	m_nEndTick = 20;
	UpdateStateImg();
	auto pLightning = SafeCast<CLightningEffect>( m_pFailEffect->GetRoot()->CreateInstance() );
	pLightning->SetParentEntity( this );
	pLightning->SetPosition( m_pProjector->GetProjSrc() );
	auto ofs = CVector2( pos.x + 1, pos.y + 0.5f ) * LEVEL_GRID_SIZE - pLightning->GetPosition();
	auto p = TVector2<int32>( floor( ofs.x / 8 + 0.5f ), floor( ofs.y / 8 + 0.5f ) );
	pLightning->Set( p, 0, 1.0f, 2.0f );
	m_pFailEftObj = pLightning;
}

void CTutorialFollowing::UpdateStep()
{
	auto p = m_curPos;
	m_pProjector->SetTarget( CVector2( p.x, p.y ) * LEVEL_GRID_SIZE );
	int32 nShowStep = m_nCurStep;
	int32 i;
	for( i = 0; i < m_nMaxShowStepCount; i++ )
	{
		auto c = m_str[nShowStep];
		if( c == 0 )
			break;
		switch( c )
		{
		case '6':
			p = TVector2<int32>( 2, 0 ) + p;
			break;
		case '4':
			p = TVector2<int32>( -2, 0 ) + p;
			break;
		case '9':
			p = TVector2<int32>( 1, 1 ) + p;
			break;
		case '7':
			p = TVector2<int32>( -1, 1 ) + p;
			break;
		case '3':
			p = TVector2<int32>( 1, -1 ) + p;
			break;
		case '1':
			p = TVector2<int32>( -1, -1 ) + p;
		default:
			break;
		}
		if( i >= m_vecImgs.size() )
			m_vecImgs.push_back( CreateImg() );
		m_vecImgs[i]->SetPosition( CVector2( p.x, p.y ) * LEVEL_GRID_SIZE );
		m_vecImgs[i]->bVisible = true;
		nShowStep++;
		if( m_str[nShowStep] == ' ' )
		{
			i++;
			break;
		}
	}
	if( i == 0 )
	{
		m_nState = 2;
		m_nEndTick = 20;
	}
	for( ; i < m_vecImgs.size(); i++ )
		m_vecImgs[i]->bVisible = false;
}

CRenderObject2D* CTutorialFollowing::CreateImg()
{
	auto pEntity = SafeCast<CEntity>( m_pPrefab->GetRoot()->CreateInstance() );
	pEntity->SetParentEntity( this );
	return pEntity;
}

CRenderObject2D* CTutorialFollowing::CreateErrImg()
{
	auto pDrawable = static_cast<CDrawableGroup*>( m_pStateImg->GetResource() );
	auto p = static_cast<CImage2D*>( pDrawable->CreateInstance() );
	p->SetRect( CRectangle( 16, 8, 16, 16 ) );
	p->SetTexRect( CRectangle( 88, 40, 8, 8 ) / 512.0f );
	AddChild( p );
	return p;
}

void CTutorialFollowing::UpdateStateImg()
{
	auto p = static_cast<CImage2D*>( m_pStateImg->GetRenderObject() );
	if( m_bError || m_nState == 3 )
	{
		p->SetRect( CRectangle( 16, 8, 16, 16 ) );
		p->SetTexRect( CRectangle( 88, 40, 8, 8 ) / 512.0f );
		return;
	}
	if( m_nState == 2 )
	{
		p->SetRect( CRectangle( 16, 8, 16, 16 ) );
		p->SetTexRect( CRectangle( 80, 40, 8, 8 ) / 512.0f );
		return;
	}
	auto c = m_str[m_nCurStep];
	auto pPlayer = GetStage()->GetMasterLevel()->GetCurLevel()->GetPlayer();
	switch( c )
	{
	case '6':
		p->SetRect( CRectangle( 16, 8, 16, 16 ) );
		p->SetTexRect( CRectangle( 64, 40, 8, 8 ) / 512.0f );
		break;
	case '9':
		if( pPlayer->GetCurDir() == 0 )
		{
			p->SetRect( CRectangle( 16, 8, 16, 16 ) );
			p->SetTexRect( CRectangle( 48, 40, 8, 8 ) / 512.0f );
		}
		else
		{
			p->SetRect( CRectangle( 0, 8, 48, 16 ) );
			p->SetTexRect( CRectangle( 48, 48, 24, 8 ) / 512.0f );
		}
		break;
	case '3':
		if( pPlayer->GetCurDir() == 0 )
		{
			p->SetRect( CRectangle( 16, 8, 16, 16 ) );
			p->SetTexRect( CRectangle( 56, 40, 8, 8 ) / 512.0f );
		}
		else
		{
			p->SetRect( CRectangle( 0, 8, 48, 16 ) );
			p->SetTexRect( CRectangle( 48, 56, 24, 8 ) / 512.0f );
		}
		break;
	case '4':
		p->SetRect( CRectangle( 16, 8, 16, 16 ) );
		p->SetTexRect( CRectangle( 72, 40, 8, 8 ) / 512.0f );
		break;
	case '7':
		if( pPlayer->GetCurDir() == 1 )
		{
			p->SetRect( CRectangle( 16, 8, 16, 16 ) );
			p->SetTexRect( CRectangle( 48, 40, 8, 8 ) / 512.0f );
		}
		else
		{
			p->SetRect( CRectangle( 0, 8, 48, 16 ) );
			p->SetTexRect( CRectangle( 72, 48, 24, 8 ) / 512.0f );
		}
		break;
	case '1':
		if( pPlayer->GetCurDir() == 1 )
		{
			p->SetRect( CRectangle( 16, 8, 16, 16 ) );
			p->SetTexRect( CRectangle( 56, 40, 8, 8 ) / 512.0f );
		}
		else
		{
			p->SetRect( CRectangle( 0, 8, 48, 16 ) );
			p->SetTexRect( CRectangle( 72, 56, 24, 8 ) / 512.0f );
		}
		break;
	}
}

void RegisterGameClasses_MiscElem()
{
	REGISTER_CLASS_BEGIN( CLevelScriptCustom )
		REGISTER_BASE_CLASS( CLevelScript )
		REGISTER_BASE_CLASS( ISignalObj )
		REGISTER_MEMBER_BEGIN( m_strInit )
			MEMBER_ARG( text, 1 )
		REGISTER_MEMBER_END()
		REGISTER_MEMBER_BEGIN( m_strBegin )
			MEMBER_ARG( text, 1 )
		REGISTER_MEMBER_END()
		REGISTER_MEMBER_BEGIN( m_strDestroy )
			MEMBER_ARG( text, 1 )
		REGISTER_MEMBER_END()
		REGISTER_MEMBER_BEGIN( m_strUpdate )
			MEMBER_ARG( text, 1 )
		REGISTER_MEMBER_END()
		REGISTER_MEMBER_BEGIN( m_strUpdate1 )
			MEMBER_ARG( text, 1 )
		REGISTER_MEMBER_END()
		REGISTER_MEMBER_BEGIN( m_strPlayerChangeState )
			MEMBER_ARG( text, 1 )
		REGISTER_MEMBER_END()
		REGISTER_MEMBER_BEGIN( m_strPlayerAction )
			MEMBER_ARG( text, 1 )
		REGISTER_MEMBER_END()
		REGISTER_MEMBER_BEGIN( m_strAlert )
			MEMBER_ARG( text, 1 )
		REGISTER_MEMBER_END()
		REGISTER_MEMBER_BEGIN( m_strSignal )
			MEMBER_ARG( text, 1 )
		REGISTER_MEMBER_END()
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CCommonLink )
		REGISTER_BASE_CLASS( CEntity )
		REGISTER_MEMBER( m_nKillType )
		REGISTER_MEMBER( m_nTargetEffectType )
		REGISTER_MEMBER( m_pSrc )
		REGISTER_MEMBER( m_pDst )
		REGISTER_MEMBER( m_srcOfs )
		REGISTER_MEMBER( m_dstOfs )
		REGISTER_MEMBER( m_nEftInterval )
		REGISTER_MEMBER( m_nEftLife )
		REGISTER_MEMBER( m_fEftStrength )
		REGISTER_MEMBER( m_pLightningPrefab )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CPawnUsageCommon )
		REGISTER_BASE_CLASS( CPawnUsage )
		REGISTER_MEMBER( m_nType )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CPawnUsageButton )
		REGISTER_BASE_CLASS( CPawnUsage )
		REGISTER_MEMBER( m_pTarget )
		REGISTER_MEMBER( m_nSignal )
		REGISTER_MEMBER( m_nEftFrames0 )
		REGISTER_MEMBER( m_eftParam0 )
		REGISTER_MEMBER( m_nEftFrames1 )
		REGISTER_MEMBER( m_eftParam1 )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CPawnAIAutoDoor )
		REGISTER_BASE_CLASS( CPawnAI )
		REGISTER_MEMBER( m_nType )
		REGISTER_MEMBER( m_strOpenCondition )
		REGISTER_MEMBER( m_strBlockPlayerCondition )
		REGISTER_MEMBER( m_strBrokenKey )
		REGISTER_MEMBER( m_strOnBlockPlayerClose )
		REGISTER_MEMBER( m_nStateMapIconX )
		REGISTER_MEMBER( m_nStateMapIconY )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CSeal )
		REGISTER_BASE_CLASS( CPawn )
		REGISTER_MEMBER( m_strStateKey )
		REGISTER_MEMBER_BEGIN( m_strStateChangeScript )
			MEMBER_ARG( text, 1 )
		REGISTER_MEMBER_END()
		REGISTER_MEMBER( m_nStateMapIconX )
		REGISTER_MEMBER( m_nStateMapIconY )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CHitButton )
		REGISTER_BASE_CLASS( CPawn )
		REGISTER_MEMBER( m_arrStates )
		REGISTER_MEMBER( m_arrTransferStates )
		REGISTER_MEMBER( m_arrStateIconTexX )
		REGISTER_MEMBER( m_arrStateIconTexY )
		REGISTER_MEMBER( m_nBrokenState )
		REGISTER_MEMBER( m_nBrokenIconTexX )
		REGISTER_MEMBER( m_nBrokenIconTexY )
		REGISTER_MEMBER( m_strStateKey )
		REGISTER_MEMBER( m_strRepairedKey )
		REGISTER_MEMBER_BEGIN( m_strStateChangeScript )
			MEMBER_ARG( text, 1 )
		REGISTER_MEMBER_END()
		REGISTER_MEMBER( m_strStateChangeSound )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CConsole )
		REGISTER_BASE_CLASS( CPawn )
		REGISTER_MEMBER_BEGIN( m_strDefaultScript )
			MEMBER_ARG( text, 1 )
		REGISTER_MEMBER_END()
		REGISTER_MEMBER_BEGIN( m_strExtraScript )
			MEMBER_ARG( text, 1 )
		REGISTER_MEMBER_END()
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CPressurePlate )
		REGISTER_BASE_CLASS( CPawnHit )
		REGISTER_MEMBER( m_strPressStateTag )
		REGISTER_MEMBER( m_strPressKey )
		REGISTER_MEMBER_BEGIN( m_strPressScript )
			MEMBER_ARG( text, 1 )
		REGISTER_MEMBER_END()
		REGISTER_MEMBER( m_strSound )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CAlarm )
		REGISTER_BASE_CLASS( CPawnHit )
		REGISTER_MEMBER_BEGIN( m_strTriggerScript )
			MEMBER_ARG( text, 1 )
		REGISTER_MEMBER_END()
		REGISTER_MEMBER( m_strSound )
		REGISTER_MEMBER_TAGGED_PTR( m_p[0], 1 )
		REGISTER_MEMBER_TAGGED_PTR( m_p[1], 2 )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CFallPoint )
		REGISTER_BASE_CLASS( CPawnHit )
		REGISTER_MEMBER( m_nNxtStage )
		REGISTER_MEMBER( m_strKey )
		REGISTER_MEMBER( m_pEft )
		REGISTER_MEMBER( m_strSound )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CClimbPoint )
		REGISTER_BASE_CLASS( CPawnHit )
		REGISTER_MEMBER( m_nNxtStage )
		REGISTER_MEMBER( m_strKey )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CHeavyDoor )
		REGISTER_BASE_CLASS( CPawn )
		REGISTER_MEMBER( m_nNxtStage )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CSmoke )
		REGISTER_BASE_CLASS( CPawnHit )
		REGISTER_MEMBER_TAGGED_PTR( m_pLightningEft, eft )
		REGISTER_MEMBER( m_nEftInterval )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CElevator )
		REGISTER_BASE_CLASS( CLevelScript )
		REGISTER_BASE_CLASS( ISignalObj )
		REGISTER_MEMBER( m_nRedirect )
		REGISTER_MEMBER( m_nFloorBegin )
		REGISTER_MEMBER( m_nFloorCount )
		REGISTER_MEMBER( m_nEftFrames )
		REGISTER_MEMBER( m_eftParam )
		REGISTER_MEMBER( m_invalidParam )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CProjector )
		REGISTER_BASE_CLASS( CLevelScript )
		REGISTER_BASE_CLASS( ISignalObj )
		REGISTER_MEMBER( m_nFixedTarget )
		REGISTER_MEMBER( m_projTargetOfs )
		REGISTER_MEMBER( m_arrFixedTargets )
		REGISTER_MEMBER( m_arrProjPos )
		REGISTER_MEMBER_TAGGED_PTR( m_p, 1 )
		REGISTER_MEMBER_TAGGED_PTR( m_p1, 2 )
		DEFINE_LUA_REF_OBJECT()
		REGISTER_LUA_CFUNCTION( GetProjSrc )
		REGISTER_LUA_CFUNCTION( SetTarget )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CTutorialMoving )
		REGISTER_BASE_CLASS( CLevelScript )
		REGISTER_BASE_CLASS( ISignalObj )
		REGISTER_MEMBER( m_str )
		REGISTER_MEMBER_BEGIN( m_strFinishedScript )
			MEMBER_ARG( text, 1 )
		REGISTER_MEMBER_END()
		REGISTER_MEMBER_BEGIN( m_strFailedScript )
			MEMBER_ARG( text, 1 )
		REGISTER_MEMBER_END()
		REGISTER_MEMBER_TAGGED_PTR( m_p, 1 )
		REGISTER_MEMBER_TAGGED_PTR( m_p1, 2 )
		REGISTER_MEMBER_TAGGED_PTR( m_pImgs[0], 1/1 )
		REGISTER_MEMBER_TAGGED_PTR( m_pImgs[1], 1/2 )
		REGISTER_MEMBER_TAGGED_PTR( m_pImgs[2], 1/3 )
		REGISTER_MEMBER_TAGGED_PTR( m_pImgs[3], 1/4 )
		REGISTER_MEMBER_TAGGED_PTR( m_pImgs[4], 1/5 )
		REGISTER_MEMBER_TAGGED_PTR( m_pImgs[5], 1/6 )
		REGISTER_MEMBER_TAGGED_PTR( m_pImgs[6], 1/7 )
		REGISTER_MEMBER_TAGGED_PTR( m_pImgs[7], 1/8 )
		REGISTER_MEMBER_TAGGED_PTR( m_pImg1, 1/0 )
		REGISTER_MEMBER_TAGGED_PTR( m_pImg2[0], 2/2 )
		REGISTER_MEMBER_TAGGED_PTR( m_pImg2[1], 2/3 )
		REGISTER_MEMBER_TAGGED_PTR( m_pImg2[2], 2/4 )
		REGISTER_MEMBER_TAGGED_PTR( m_pImgNxt, 1/nxt )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CTutorialFollowing )
		REGISTER_BASE_CLASS( CLevelScript )
		REGISTER_BASE_CLASS( ISignalObj )
		REGISTER_MEMBER( m_bNoStop )
		REGISTER_MEMBER( m_nMaxShowStepCount )
		REGISTER_MEMBER( m_nBeginX )
		REGISTER_MEMBER( m_nBeginY )
		REGISTER_MEMBER( m_str )
		REGISTER_MEMBER_BEGIN( m_strStepScript )
			MEMBER_ARG( text, 1 )
		REGISTER_MEMBER_END()
		REGISTER_MEMBER_BEGIN( m_strFinishedScript )
			MEMBER_ARG( text, 1 )
		REGISTER_MEMBER_END()
		REGISTER_MEMBER_BEGIN( m_strFailedScript )
			MEMBER_ARG( text, 1 )
		REGISTER_MEMBER_END()
		REGISTER_MEMBER( m_pPrefab )
		REGISTER_MEMBER( m_pFailEffect )
		REGISTER_MEMBER( m_pStateImg )
		REGISTER_MEMBER_TAGGED_PTR( m_pProjector, proj )
	REGISTER_CLASS_END()
}