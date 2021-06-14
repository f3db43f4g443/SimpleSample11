#include "stdafx.h"
#include "MyGame.h"
#include "MyLevel.h"
#include "GameState.h"
#include "Entities/MiscElem.h"
#include "Entities/UtilEntities.h"
#include "Common/Rand.h"
#include "GlobalCfg.h"
#include "CommonUtils.h"
#include "Common/FileUtil.h"

void RunLuaFile( const char* szName )
{
	uint32 nLen;
	const char* sz = GetFileContent( szName, true, nLen );
	CLuaMgr::Inst().Run( sz );
	free( (void*)sz );
}

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

bool IsKey( uint32 nKey )
{
	return CGame::Inst().IsKey( nKey );
}

bool IsKeyUp( uint32 nKey )
{
	return CGame::Inst().IsKeyUp( nKey );
}

bool IsKeyDown( uint32 nKey )
{
	return CGame::Inst().IsKeyDown( nKey );
}

bool IsChar( int32 nInput )
{
	return CGame::Inst().IsChar( nInput );
}

bool IsInput( int32 nInput )
{
	return CGame::Inst().IsInput( nInput );
}

bool IsInputUp( int32 nInput )
{
	return CGame::Inst().IsInputUp( nInput );
}

bool IsInputDown( int32 nInput )
{
	return CGame::Inst().IsInputDown( nInput );
}

float GetX( CRenderObject2D* p )
{
	return p->x;
}

float GetY( CRenderObject2D* p )
{
	return p->y;
}

float GetScale( CRenderObject2D* p )
{
	return p->s;
}

void SetPosition( CRenderObject2D* p, float x, float y )
{
	p->SetPosition( CVector2( x, y ) );
}

void SetScale( CRenderObject2D* p, float s )
{
	p->s = s;
	p->SetTransformDirty();
}

void PlayerPickUp( const char* szName, CPlayer* pPlayer = NULL )
{
	auto pPrefab = CResourceManager::Inst()->CreateResource<CPrefab>( szName );
	if( !pPrefab || !pPrefab->GetRoot()->GetStaticDataSafe<CPickUp>() )
		return;
	auto pPickUp = SafeCast<CPickUp>( pPrefab->GetRoot()->CreateInstance() );
	pPickUp->strCreatedFrom = szName;
	pPickUp->AutoCreateSpawnHelper();
	auto pEquipment = pPickUp->GetEquipment();
	if( pEquipment )
		pEquipment->Init();
	if( !pPlayer )
		pPlayer = GetPlayer();
	pPickUp->PickUp( pPlayer );
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

void SetImgTexRect( CEntity* pEntity, const CRectangle& rect )
{
	auto pImg = SafeCast<CImage2D>( pEntity->GetRenderObject() );
	if( pImg )
		pImg->SetTexRect( rect );
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

CEntity* CreateLighningEft_Script( CVector2 begin, CVector2 end, float fStrength, float fTurbulence, int32 nDuration )
{
	auto pLightning = SafeCast<CLightningEffect>( CGlobalCfg::Inst().pFailLightningEffectPrefab->GetRoot()->CreateInstance() );
	pLightning->SetParentEntity( GetCurLevel() );
	pLightning->SetPosition( begin );
	auto ofs = end - begin;
	auto p1 = TVector2<int32>( floor( ofs.x / 8 + 0.5f ), floor( ofs.y / 8 + 0.5f ) );
	pLightning->Set( p1, nDuration, fStrength, fTurbulence );
	return pLightning;
}

CEntity* CreateCommonLink( CEntity* pSrc, CEntity* pDst, int8 nKillType, int8 nTargetEffecType )
{
	auto pCommonLink = SafeCast<CCommonLink>( CGlobalCfg::Inst().pCommonLinkPrefab->GetRoot()->CreateInstance() );
	pCommonLink->Set( pSrc, pDst, nKillType, nTargetEffecType );
	pCommonLink->SetParentEntity( GetCurLevel() );
	return pCommonLink;
}

bool PawnCanMoveTo( CPawn* pPawn, int32 x, int32 y )
{
	return pPawn->GetLevel()->IsGridMoveable( TVector2<int32>( x, y ), pPawn );
}

bool BulletCanPass( CPawn* pTarget, int32 x, int32 y )
{
	auto pLevel = pTarget->GetLevel();
	auto pGrid = pLevel->GetGrid( TVector2<int32>( x, y ) );
	if( !pGrid || !pGrid->bCanEnter )
		return false;
	CPawn* pPawn = pGrid->pPawn0;
	if( pPawn && pPawn != pTarget && !pPawn->IsIgnoreBullet() && !pPawn->IsSpecialState( CPawn::eSpecialState_Fall ) )
		return false;
	return true;
}

int32 FindPath( int32 x1, int32 y1, int32 x2, int32 y2, int32 nCheckFlag )
{
	auto pLevel = GetCurLevel();
	auto result = pLevel->SimpleFindPath( TVector2<int32>( x1, y1 ), TVector2<int32>( x2, y2 ), nCheckFlag );
	auto pLuaState = CLuaMgr::GetCurLuaState();
	pLuaState->PushLua( result.x );
	pLuaState->PushLua( result.y );
	return 2;
}

void ForceAllVisible()
{
	auto& worldData = GetMasterLevel()->GetWorldData();
	worldData.curFrame.bForceAllVisible = true;
}

int32 GetCurTick()
{
	return CGame::Inst().GetTimeStamp();
}


void RegisterGlobalLuaCFunc()
{
	REGISTER_LUA_CFUNCTION_GLOBAL( RunLuaFile )
	REGISTER_LUA_CFUNCTION_GLOBAL( GetMasterLevel )
	REGISTER_LUA_CFUNCTION_GLOBAL( GetCurLevel )
	REGISTER_LUA_CFUNCTION_GLOBAL( GetCurCutScene )
	REGISTER_LUA_CFUNCTION_GLOBAL( GetPlayer )
	REGISTER_LUA_CFUNCTION_GLOBAL( GetPlayerName )
	REGISTER_LUA_CFUNCTION_GLOBAL( IsKey )
	REGISTER_LUA_CFUNCTION_GLOBAL( IsKeyUp )
	REGISTER_LUA_CFUNCTION_GLOBAL( IsKeyDown )
	REGISTER_LUA_CFUNCTION_GLOBAL( IsChar )
	REGISTER_LUA_CFUNCTION_GLOBAL( IsInput )
	REGISTER_LUA_CFUNCTION_GLOBAL( IsInputUp )
	REGISTER_LUA_CFUNCTION_GLOBAL( IsInputDown )
	REGISTER_LUA_CFUNCTION_GLOBAL( GetX )
	REGISTER_LUA_CFUNCTION_GLOBAL( GetY )
	REGISTER_LUA_CFUNCTION_GLOBAL( GetScale )
	REGISTER_LUA_CFUNCTION_GLOBAL( SetPosition )
	REGISTER_LUA_CFUNCTION_GLOBAL( SetScale )
	REGISTER_LUA_CFUNCTION_GLOBAL( PlayerPickUp )
	REGISTER_LUA_CFUNCTION_GLOBAL( RandInt )
	REGISTER_LUA_CFUNCTION_GLOBAL( RandFloat )
	REGISTER_LUA_CFUNCTION_GLOBAL( Signal )
	REGISTER_LUA_CFUNCTION_GLOBAL( SetImgTexRect )
	REGISTER_LUA_CFUNCTION_GLOBAL( SetImgParam )
	REGISTER_LUA_CFUNCTION_GLOBAL( RunScenario )
	REGISTER_LUA_CFUNCTION_GLOBAL( LevelRegisterBegin )
	REGISTER_LUA_CFUNCTION_GLOBAL( LevelRegisterUpdate )
	REGISTER_LUA_CFUNCTION_GLOBAL( LevelRegisterUpdate1 )
	REGISTER_LUA_CFUNCTION_GLOBAL( LevelRegisterAlwaysUpdate )
	REGISTER_LUA_CFUNCTION_GLOBAL( CreateLighningEft_Script )
	REGISTER_LUA_CFUNCTION_GLOBAL( CreateCommonLink )
	REGISTER_LUA_CFUNCTION_GLOBAL( PawnCanMoveTo )
	REGISTER_LUA_CFUNCTION_GLOBAL( BulletCanPass )
	REGISTER_LUA_CFUNCTION_GLOBAL( PlaySoundEffect )
	REGISTER_LUA_CFUNCTION_GLOBAL_RETUNWR( FindPath )
	REGISTER_LUA_CFUNCTION_GLOBAL( ForceAllVisible )
	REGISTER_LUA_CFUNCTION_GLOBAL( GetCurTick )
}