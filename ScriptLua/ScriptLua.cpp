#include "Common.h"
#include "ScriptLua.h"
#include "lua-5.3.5/lua.hpp"

vector<CLuaState*> CLuaState::g_vecRunningCoroutines;
CLuaState::~CLuaState()
{
	if( this != &CLuaMgr::Inst() )
	{
		ASSERT( m_nState != 2 );
		lua_pushliteral( (lua_State*)m_pLuaState, "__threads" );
		lua_rawget( (lua_State*)m_pLuaState, LUA_REGISTRYINDEX );
		lua_pushnil( (lua_State*)m_pLuaState );
		lua_rawsetp( (lua_State*)m_pLuaState, -2, this );
		lua_pop( (lua_State*)m_pLuaState, 1 );
	}
}

void CLuaState::PushLuaBool( void* p, bool b )
{
	lua_pushboolean( (lua_State*)p, b );
}

void CLuaState::PushLuaInt( void* p, int64 i )
{
	lua_pushinteger( (lua_State*)p, i );
}

void CLuaState::PushLuaNumber( void* p, float f )
{
	lua_pushnumber( (lua_State*)p, f );
}

void CLuaState::PushLuaVec( void* p, float x, float y, float z, float w, int8 n )
{
	lua_createtable( (lua_State*)p, n, 0 );
	float f[4] = { x, y, z, w };
	for( int i = 0; i < n; i++ )
	{
		lua_pushnumber( (lua_State*)p, f[i] );
		lua_seti( (lua_State*)p, -2, i + 1 );
	}
}

void CLuaState::PushLuaString( void* p, const char* str )
{
	lua_pushstring( (lua_State*)p, str );
}

void CLuaState::PushReference( void* p, CReferenceObject* pObj, SClassMetaData* pMetaData )
{
	if( !pObj )
	{
		lua_pushnil( (lua_State*)p );
		return;
	}
	auto nOfs = pMetaData->GetBaseClassOfs( CClassMetaDataMgr::Inst().GetClassData<CBaseObject>() );
	if( nOfs >= 0 )
		pMetaData = CClassMetaDataMgr::Inst().GetClassData( ( (CBaseObject*)pObj )->GetTypeID() );
	new ( lua_newuserdata( (lua_State*)p, sizeof( CReference<CReferenceObject> ) ) ) CReference<CReferenceObject>( pObj );
	if( pMetaData )
	{
		if( PushMetaTable( p, pMetaData ) )
			lua_setmetatable( (lua_State*)p, -2 );
	}
}

bool CLuaState::PushMetaTable( void* p, SClassMetaData* pMetaData )
{
	lua_pushliteral( (lua_State*)p, "__types" );
	lua_rawget( (lua_State*)p, LUA_REGISTRYINDEX );

	for( auto nType = lua_geti( (lua_State*)p, -1, pMetaData->nID + 1 );; )
	{
		if( nType == LUA_TTABLE )
		{
			lua_remove( (lua_State*)p, -2 );
			return true;
		}

		lua_pop( (lua_State*)p, 1 );
		if( pMetaData->vecBaseClassData.size() )
		{
			pMetaData = pMetaData->vecBaseClassData[0].pBaseClass;
			if( !pMetaData )
				break;
			nType = lua_rawgeti( (lua_State*)p, -1, pMetaData->nID + 1 );
			continue;
		}
		break;
	}

	lua_pop( (lua_State*)p, 1 );
	return false;
}

void CLuaState::PushLuaCFunction( void* p, SLuaCFunction& func )
{
	struct STemp
	{
		static int Lua_CFunc( lua_State* L )
		{
			auto p = (SLuaCFunction*)lua_touserdata( L, lua_upvalueindex( 1 ) );
			p->Func( L );
			return p->nRet;
		}
	};
	auto pLuaState = (lua_State*)p;
	lua_pushlightuserdata( pLuaState, &func );
	lua_pushcclosure( pLuaState, &STemp::Lua_CFunc, 1 );
}

void CLuaState::SetGlobal( void* p, const char* szName )
{
	lua_setglobal( (lua_State*)p, szName );
}

bool CLuaState::FetchLuaBool( void* p, int32 idx )
{
	return lua_toboolean( (lua_State*)p, idx );
}

int64 CLuaState::FetchLuaInt( void* p, int32 idx )
{
	if( lua_isboolean( (lua_State*)p, idx ) )
		return lua_toboolean( (lua_State*)p, idx );
	return lua_tointeger( (lua_State*)p, idx );
}

float CLuaState::FetchLuaNumber( void* p, int32 idx )
{
	return lua_tonumber( (lua_State*)p, idx );
}

void CLuaState::FetchLuaVec( void* p, int32 idx, int32 nCount, float* pVec )
{
	for( int i = 0; i < nCount; i++ )
	{
		lua_geti( (lua_State*)p, idx, i + 1 );
		pVec[i] = lua_tonumber( (lua_State*)p, -1 );
		lua_pop( (lua_State*)p, 1 );
	}
}

const char* CLuaState::FetchLuaString( void* p, int32 idx )
{
	return lua_tostring( (lua_State*)p, idx );
}

CReferenceObject* CLuaState::FetchReference( void* p, int32 idx )
{
	if( !lua_isuserdata( (lua_State*)p, idx ) )
		return NULL;
	return ( (CReference<CReferenceObject>*)lua_touserdata( (lua_State*)p, idx ) )->GetPtr();
}

void CLuaState::PopLua( void* p, int32 n )
{
	lua_pop( (lua_State*)p, n );
}

void CLuaState::Run( const char* str, int32 nRet )
{
	Load( str );
	Call( 0, nRet );
}

void CLuaState::Load( const char* str )
{
	EnsureInited();
	if( str[0] == '@' )
	{
		if( str[1] == '@' )
		{
			CheckError( 0, luaL_loadstring( (lua_State*)m_pLuaState, str + 2 ) );
		}
		else
		{
			string s = "return ";
			s += str + 1;
			CheckError( 0, luaL_loadstring( (lua_State*)m_pLuaState, s.c_str() ) );
		}
		CheckError( 0, lua_pcall( (lua_State*)m_pLuaState, 0, 1, 0 ) );
		ASSERT( lua_isfunction( (lua_State*)m_pLuaState, -1 ) );
	}
	else
		CheckError( 0, luaL_loadstring( (lua_State*)m_pLuaState, str ) );
}

void CLuaState::Call( int32 nParams, int32 nRet )
{
	CheckError( 0, lua_pcall( (lua_State*)m_pLuaState, nParams, nRet, 0 ) );
}

CLuaState* CLuaState::CreateCoroutine( const char* str )
{
	auto pNewState = CreateNewState();
	pNewState->Load( str );
	return pNewState;
}

CLuaState* CLuaState::CreateCoroutineFromFunc()
{
	auto pNewState = CreateNewState();
	VERIFY( lua_isfunction( (lua_State*)m_pLuaState, -1 ) );
	lua_rawsetp( (lua_State*)m_pLuaState, LUA_REGISTRYINDEX, this );
	lua_rawgetp( (lua_State*)pNewState->m_pLuaState, LUA_REGISTRYINDEX, this );
	lua_pushnil( (lua_State*)m_pLuaState );
	lua_rawsetp( (lua_State*)m_pLuaState, LUA_REGISTRYINDEX, this );
	return pNewState;
}

CLuaState* CLuaState::CreateCoroutineAuto()
{
	if( lua_isfunction( (lua_State*)m_pLuaState, -1 ) )
		return CreateCoroutineFromFunc();
	else if( lua_isstring( (lua_State*)m_pLuaState, -1 ) )
	{
		auto sz = lua_tostring( (lua_State*)m_pLuaState, -1 );
		auto pNewState = CreateCoroutine( sz );
		lua_pop( (lua_State*)m_pLuaState, 1 );
		return pNewState;
	}
	return NULL;
}

CLuaState* CLuaState::CreateNewState()
{
	EnsureInited();
	lua_pushliteral( (lua_State*)m_pLuaState, "__threads" );
	lua_rawget( (lua_State*)m_pLuaState, LUA_REGISTRYINDEX );
	auto pNewState = lua_newthread( (lua_State*)m_pLuaState );
	CLuaState* pLuaState = new CLuaState( pNewState );
	lua_rawsetp( (lua_State*)m_pLuaState, -2, pNewState );
	lua_pop( (lua_State*)m_pLuaState, 1 );
	return pLuaState;
}

bool CLuaState::Resume( int32 nParams, int32* pRet )
{
	g_vecRunningCoroutines.push_back( this );
	ASSERT( m_nState <= 1 );
	auto nStack0 = lua_gettop( (lua_State*)m_pLuaState ) - nParams - ( m_nState == 0 ? 1 : 0 );
	m_nState = 2;
	auto nResult = lua_resume( (lua_State*)m_pLuaState, NULL, nParams );
	CheckError( 1, nResult );
	m_nState = nResult == LUA_YIELD ? 1 : 3;
	if( *pRet < 0 )
		*pRet = lua_gettop( (lua_State*)m_pLuaState ) - nStack0;
	else
		lua_settop( (lua_State*)m_pLuaState, nStack0 + *pRet );
	g_vecRunningCoroutines.pop_back();
	return nResult == LUA_YIELD;
}

void CLuaState::CheckError( int32 nType, int32 nResult )
{
	if( nResult > nType )
	{
		auto err = lua_tostring( (lua_State*)m_pLuaState, -1 );
		VERIFY( false );
	}
}

CLuaState* CLuaState::GetCurLuaState()
{
	if( g_vecRunningCoroutines.size() )
		return g_vecRunningCoroutines.back();
	return &CLuaMgr::Inst();
}


void CLuaMgr::InitLua()
{
	if( m_pLuaState )
		return;
	m_pLuaState = luaL_newstate();  /* create state */

	const luaL_Reg loadedlibs[] = {
		{ "_G", luaopen_base },
		{ LUA_LOADLIBNAME, luaopen_package },
		{ LUA_COLIBNAME, luaopen_coroutine },
		{ LUA_TABLIBNAME, luaopen_table },
		//{ LUA_IOLIBNAME, luaopen_io },
		//{ LUA_OSLIBNAME, luaopen_os },
		{ LUA_STRLIBNAME, luaopen_string },
		{ LUA_MATHLIBNAME, luaopen_math },
		//{ LUA_UTF8LIBNAME, luaopen_utf8 },
		//{ LUA_DBLIBNAME, luaopen_debug },
		{ NULL, NULL }
	};
	const luaL_Reg *lib;
	for( lib = loadedlibs; lib->func; lib++ ) {
		luaL_requiref( (lua_State*)m_pLuaState, lib->name, lib->func, 1 );
		lua_pop( (lua_State*)m_pLuaState, 1 );
	}

	lua_pushliteral( (lua_State*)m_pLuaState, "__threads" );
	lua_createtable( (lua_State*)m_pLuaState, 0, 32 );
	lua_rawset( (lua_State*)m_pLuaState, LUA_REGISTRYINDEX );
	lua_pushliteral( (lua_State*)m_pLuaState, "__types" );
	lua_createtable( (lua_State*)m_pLuaState, 0, 32 );
	lua_rawset( (lua_State*)m_pLuaState, LUA_REGISTRYINDEX );
	lua_pushliteral( (lua_State*)m_pLuaState, "__obj_data" );
	lua_createtable( (lua_State*)m_pLuaState, 0, 32 );
	lua_rawset( (lua_State*)m_pLuaState, LUA_REGISTRYINDEX );

	for( auto n : m_vecGlobalLuaCFuncs )
	{
		auto& globalFunc = m_vecLuaCFuncs[n];
		PushLuaCFunction( m_pLuaState, globalFunc );
		lua_setglobal( (lua_State*)m_pLuaState, globalFunc.strFuncName.c_str() );
	}

	lua_pushliteral( (lua_State*)m_pLuaState, "__types" );
	lua_rawget( (lua_State*)m_pLuaState, LUA_REGISTRYINDEX );
	for( auto& p : m_vecScriptData )
		InitMetaTable( p );
	lua_pop( (lua_State*)m_pLuaState, 1 );
}

CClassScriptDataLua* CLuaMgr::CreateScriptData( SClassMetaData* pClassData )
{
	auto pScript = new CClassScriptDataLua;
	pScript->pClassData = pClassData;
	m_vecScriptData.push_back( pScript );
	return pScript;
}

void CLuaMgr::InitMetaTable( CClassScriptDataLua* pScriptData )
{
	auto pLuaState = (lua_State*)m_pLuaState;
	lua_newtable( pLuaState );

	struct STemp
	{
		static int __eq( lua_State* L )
		{
			auto p1 = ( ( CReference<CReferenceObject>* )lua_touserdata( (lua_State*)L, 1 ) )->GetPtr();
			auto p2 = ( ( CReference<CReferenceObject>* )lua_touserdata( (lua_State*)L, 2 ) )->GetPtr();
			lua_pushboolean( L, p1 == p2 );
			return 1;
		}
		static int __gc( lua_State* L )
		{
			( ( CReference<CReferenceObject>* )lua_touserdata( (lua_State*)L, 1 ) )->~CReference();
			return 0;
		}
		static int __data_init( lua_State* L )
		{
			auto p = ( ( CReference<CReferenceObject>* )lua_touserdata( (lua_State*)L, 1 ) )->GetPtr();
			lua_pushliteral( L, "__obj_data" );
			lua_rawget( L, LUA_REGISTRYINDEX );
			lua_newtable( L );
			lua_pushlightuserdata( L, p );
			lua_pushvalue( L, -2 );
			lua_rawset( L, -4 );
			return 1;
		}
		static int __data( lua_State* L )
		{
			auto p = ( ( CReference<CReferenceObject>* )lua_touserdata( (lua_State*)L, 1 ) )->GetPtr();
			lua_pushliteral( L, "__obj_data" );
			lua_rawget( L, LUA_REGISTRYINDEX );
			lua_pushlightuserdata( L, p );
			lua_rawget( L, -2 );
			return 1;
		}
		static int __data_destroy( lua_State* L )
		{
			auto p = ( ( CReference<CReferenceObject>* )lua_touserdata( (lua_State*)L, 1 ) )->GetPtr();
			lua_pushliteral( L, "__obj_data" );
			lua_rawget( L, LUA_REGISTRYINDEX );
			lua_pushlightuserdata( L, p );
			lua_pushnil( L );
			lua_rawset( L, -3 );
			return 0;
		}
	};
	lua_pushcfunction( pLuaState, &STemp::__eq );
	lua_setfield( pLuaState, -2, "__eq" );
	lua_pushcfunction( pLuaState, &STemp::__gc );
	lua_setfield( pLuaState, -2, "__gc" );
	lua_pushcfunction( pLuaState, &STemp::__data_init );
	lua_setfield( pLuaState, -2, "__data_init" );
	lua_pushcfunction( pLuaState, &STemp::__data );
	lua_setfield( pLuaState, -2, "__data" );
	lua_pushcfunction( pLuaState, &STemp::__data_destroy );
	lua_setfield( pLuaState, -2, "__data_destroy" );
	lua_pushvalue( pLuaState, -1 );
	lua_setfield( pLuaState, -2, "__index" );
	for( auto n : pScriptData->vecLuaCFuncs )
	{
		auto& C = m_vecLuaCFuncs[n];
		PushLuaCFunction( m_pLuaState, C );
		lua_setfield( pLuaState, -2, C.strFuncName.c_str() );
	}
	if( pScriptData->pClassData->vecBaseClassData.size() )
	{
		auto pBaseClass = pScriptData->pClassData->vecBaseClassData[0].pBaseClass;
		if( PushMetaTable( m_pLuaState, pBaseClass ) )
			lua_setmetatable( (lua_State*)m_pLuaState, -2 );
	}

	lua_rawseti( pLuaState, -2, pScriptData->pClassData->nID + 1 );
}