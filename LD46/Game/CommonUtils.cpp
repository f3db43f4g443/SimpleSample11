#include "stdafx.h"
#include "CommonUtils.h"

void CLuaTrigger::Run( void* pContext )
{
	int32 nParams = m_nParamType > eParam_None ? 1 : 0;
	if( m_nParamType == eParam_Int )
		m_pLuaState->PushLua( (int32)pContext );
	else if( m_nParamType == eParam_Obj )
		m_pLuaState->PushLua( (CBaseObject*)pContext );
	if( !m_pLuaState->Resume( nParams, 0 ) )
		Unregister();
}

CLuaTrigger* CLuaTrigger::CreateFromText( const char* sz, int8 nParamType )
{
	auto pLuaState = CLuaState::GetCurLuaState();
	auto pCoroutine = pLuaState->CreateCoroutine( sz );
	ASSERT( pCoroutine );
	auto p = new CLuaTrigger;
	p->m_pLuaState = pCoroutine;
	p->m_nParamType = nParamType;
	return p;
}

CLuaTrigger* CLuaTrigger::CreateAuto( int8 nParamType )
{
	auto pLuaState = CLuaState::GetCurLuaState();
	auto pCoroutine = pLuaState->CreateCoroutineAuto();
	ASSERT( pCoroutine );
	auto p = new CLuaTrigger;
	p->m_pLuaState = pCoroutine;
	p->m_nParamType = nParamType;
	return p;
}
