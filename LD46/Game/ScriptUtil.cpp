#include "stdafx.h"
#include "MyLevel.h"
#include "GameState.h"
#include "Entities/UtilEntities.h"
#include "Common/Rand.h"

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

void RegisterGlobalLuaCFunc()
{
	REGISTER_LUA_CFUNCTION_GLOBAL( GetMasterLevel )
	REGISTER_LUA_CFUNCTION_GLOBAL( GetCurLevel )
	REGISTER_LUA_CFUNCTION_GLOBAL( GetCurCutScene )
	REGISTER_LUA_CFUNCTION_GLOBAL( GetPlayer )
	REGISTER_LUA_CFUNCTION_GLOBAL( GetPlayerName )
	REGISTER_LUA_CFUNCTION_GLOBAL( RandInt )
	REGISTER_LUA_CFUNCTION_GLOBAL( RandFloat )
	REGISTER_LUA_CFUNCTION_GLOBAL( Signal )
	REGISTER_LUA_CFUNCTION_GLOBAL( SetImgParam )
	REGISTER_LUA_CFUNCTION_GLOBAL( RunScenario )
}