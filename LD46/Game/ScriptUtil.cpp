#include "stdafx.h"
#include "MyLevel.h"
#include "GameState.h"
#include "Entities/UtilEntities.h"
#include "Common/Rand.h"
#include "GlobalCfg.h"
#include "CommonUtils.h"

CMasterLevel* GetMasterLevel()
{
	return CMainGameState::Inst().GetWorld()->GetCurStage()->GetMasterLevel();
}

CMyLevel* GetCurLevel()
{
	return GetMasterLevel()->GetCurLevel();
}

CCutScene* GetCurCutScene()
{
	return GetMasterLevel()->GetCurCutScene();
}

CPlayer* GetPlayer()
{
	return CMainGameState::Inst().GetWorld()->GetPlayer();
}

const char* GetPlayerName()
{
	return "sir";
}

void PlayerPickUp( const char* szName )
{
	auto pPrefab = CResourceManager::Inst()->CreateResource<CPrefab>( szName );
	if( !pPrefab || !pPrefab->GetRoot()->GetStaticDataSafe<CPickUp>() )
		return;
	auto pPickUp = SafeCast<CPickUp>( pPrefab->GetRoot()->CreateInstance() );
	pPickUp->strCreatedFrom = szName;
	auto pEquipment = pPickUp->GetEquipment();
	if( pEquipment )
		pEquipment->Init();
	pPickUp->PickUp( GetPlayer() );
}

int32 RandInt( int32 nMin, int32 nMax )
{
	return SRand::Inst().Rand( nMin, nMax );
}

float RandFloat( float fMin, float fMax )
{
	return SRand::Inst().Rand( fMin, fMax );
}

int32 Signal( CEntity* pEntity, int32 n )
{
	auto p = SafeCastToInterface<ISignalObj>( pEntity );
	if( p )
		return p->Signal( n );
	return 0;
}

void SetImgParam( CEntity* pEntity, const CVector4& param )
{
	auto p = SafeCastToInterface<IImageEffectTarget>( pEntity );
	if( p )
	{
		p->SetParam( param );
		return;
	}
	auto pImg = SafeCast<CImage2D>( pEntity->GetRenderObject() );
	if( pImg && pImg->GetParamCount() )
		pImg->GetParam()[0] = param;
}

void RunScenario()
{
	GetMasterLevel()->RunScenarioScript();
}

void LevelRegisterBegin()
{
	auto pTrigger = CLuaTrigger::CreateAuto();
	GetCurLevel()->RegisterBegin( pTrigger );
}

void LevelRegisterUpdate()
{
	auto pTrigger = CLuaTrigger::CreateAuto();
	GetCurLevel()->RegisterUpdate( pTrigger );
}

void LevelRegisterUpdate1()
{
	auto pTrigger = CLuaTrigger::CreateAuto();
	GetCurLevel()->RegisterUpdate1( pTrigger );
}

void LevelRegisterAlwaysUpdate()
{
	auto pTrigger = CLuaTrigger::CreateAuto();
	GetCurLevel()->RegisterAlwaysUpdate( pTrigger );
}

CEntity* CreateLighningEft( CVector2 begin, CVector2 end )
{
	auto pLightning = SafeCast<CLightningEffect>( CGlobalCfg::Inst().pFailLightningEffectPrefab->GetRoot()->CreateInstance() );
	pLightning->SetParentEntity( GetCurLevel() );
	pLightning->SetPosition( begin );
	auto ofs = end - begin;
	auto p1 = TVector2<int32>( floor( ofs.x / 8 + 0.5f ), floor( ofs.y / 8 + 0.5f ) );
	pLightning->Set( p1, 0, 1.0f, 2.0f );
	return pLightning;
}


void ForceAllVisible()
{
	auto& worldData = GetMasterLevel()->GetWorldData();
	worldData.curFrame.bForceAllVisible = true;
}


void RegisterGlobalLuaCFunc()
{
	REGISTER_LUA_CFUNCTION_GLOBAL( GetMasterLevel )
	REGISTER_LUA_CFUNCTION_GLOBAL( GetCurLevel )
	REGISTER_LUA_CFUNCTION_GLOBAL( GetCurCutScene )
	REGISTER_LUA_CFUNCTION_GLOBAL( GetPlayer )
	REGISTER_LUA_CFUNCTION_GLOBAL( GetPlayerName )
	REGISTER_LUA_CFUNCTION_GLOBAL( PlayerPickUp )
	REGISTER_LUA_CFUNCTION_GLOBAL( RandInt )
	REGISTER_LUA_CFUNCTION_GLOBAL( RandFloat )
	REGISTER_LUA_CFUNCTION_GLOBAL( Signal )
	REGISTER_LUA_CFUNCTION_GLOBAL( SetImgParam )
	REGISTER_LUA_CFUNCTION_GLOBAL( RunScenario )
	REGISTER_LUA_CFUNCTION_GLOBAL( LevelRegisterBegin )
	REGISTER_LUA_CFUNCTION_GLOBAL( LevelRegisterUpdate )
	REGISTER_LUA_CFUNCTION_GLOBAL( LevelRegisterUpdate1 )
	REGISTER_LUA_CFUNCTION_GLOBAL( LevelRegisterAlwaysUpdate )
	REGISTER_LUA_CFUNCTION_GLOBAL( CreateLighningEft )
	REGISTER_LUA_CFUNCTION_GLOBAL( PlaySoundEffect )
	REGISTER_LUA_CFUNCTION_GLOBAL( ForceAllVisible )
}