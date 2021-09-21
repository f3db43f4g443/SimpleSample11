#pragma once
#include "ClassMetaData.h"

struct SLuaCFunction
{
	string strFuncName;
	function<int( void* )> Func;
};

class CClassScriptDataLua : public CReferenceObject
{
public:
	SClassMetaData* pClassData;
	vector<int32> vecLuaCFuncs;
};

class CLuaState
{
public:
	CLuaState() : m_pLuaState( NULL ), m_nRefCount( 0 ), m_nState( 0 ) {}
	CLuaState( void* pLuaState ) : m_pLuaState( pLuaState ), m_nRefCount( 0 ), m_nState( 0 ) {}
	virtual ~CLuaState();

	virtual void AddRef() { m_nRefCount++; }
	virtual void Release() { m_nRefCount--; if( !m_nRefCount ) delete this; }

	template <typename T>
	struct PushLua_Impl { static void call( void* p, T pValue ) { ASSERT( false ); } };
	template <typename T>
	struct PushLua_Impl<T*> { static void call( void* p, T* pValue ) { PushReference( p, pValue, CClassMetaDataMgr::Inst().GetClassData<T>() ); } };
	template <typename T>
	struct PushLua_Impl<CReference<T> > { static void call( void* p, const CReference<T>& pValue ) { PushReference( p, pValue, CClassMetaDataMgr::Inst().GetClassData<T>() ); } };
	template <>
	struct PushLua_Impl<bool> { static void call( void* p, bool pValue ) { PushLuaBool( p, pValue ); } };
	template <>
	struct PushLua_Impl<uint8> { static void call( void* p, uint8 pValue ) { PushLuaInt( p, pValue ); } };
	template <>
	struct PushLua_Impl<uint16> { static void call( void* p, uint16 pValue ) { PushLuaInt( p, pValue ); } };
	template <>
	struct PushLua_Impl<uint32> { static void call( void* p, uint32 pValue ) { PushLuaInt( p, pValue ); } };
	template <>
	struct PushLua_Impl<uint64> { static void call( void* p, uint64 pValue ) { PushLuaInt( p, pValue ); } };
	template <>
	struct PushLua_Impl<int8> { static void call( void* p, int8 pValue ) { PushLuaInt( p, pValue ); } };
	template <>
	struct PushLua_Impl<int16> { static void call( void* p, int16 pValue ) { PushLuaInt( p, pValue ); } };
	template <>
	struct PushLua_Impl<int32> { static void call( void* p, int32 pValue ) { PushLuaInt( p, pValue ); } };
	template <>
	struct PushLua_Impl<int64> { static void call( void* p, int64 pValue ) { PushLuaInt( p, pValue ); } };
	template <>
	struct PushLua_Impl<float> { static void call( void* p, float pValue ) { PushLuaNumber( p, pValue ); } };
	template <>
	struct PushLua_Impl<CVector2> { static void call( void* p, CVector2 pValue )
	{ PushLuaVec( p, pValue.x, pValue.y, 0, 0, 2 ); } };
	template <>
	struct PushLua_Impl<CVector3> { static void call( void* p, CVector3 pValue )
	{ PushLuaVec( p, pValue.x, pValue.y, pValue.z, 0, 3 ); } };
	template <>
	struct PushLua_Impl<CVector4> { static void call( void* p, CVector4 pValue )
	{ PushLuaVec( p, pValue.x, pValue.y, pValue.z, pValue.w, 4 ); } };
	template <>
	struct PushLua_Impl<CRectangle> { static void call( void* p, CRectangle pValue )
	{ PushLuaVec( p, pValue.x, pValue.y, pValue.width, pValue.height, 4 ); } };
	template <>
	struct PushLua_Impl<const char*> { static void call( void* p, const char* pValue ) { PushLuaString( p, pValue ); } };

	template <typename T>
	struct PushLua_Impl<const T&> { static void call( void* p, const T& pValue ) { PushLua_Impl<T>::call( p, pValue ); } };

	template <typename T>
	static void PushLua( void* p, T pValue ) { PushLua_Impl<T>::call( p, pValue ); }
	template <typename T>
	static void SetLuaGlobal( void* p, T pValue, const char* szName ) { PushLua( p, pValue ); SetGlobal( p, szName ); }
	template <typename T>
	void PushLua( T pValue ) { PushLua_Impl<T>::call( m_pLuaState, pValue ); }
	template <typename T>
	void SetLuaGlobal( T pValue, const char* szName ) { PushLua( m_pLuaState, pValue ); SetGlobal( m_pLuaState, szName ); }

	template <typename T>
	struct FecthLua_Impl { static T call( void* p, int32 n ) { ASSERT( false ); } };
	template <typename T>
	struct FecthLua_Impl<T*> { static T* call( void* p, int32 n ) { return static_cast<T*>( FetchReference( p, n ) ); } };
	template <>
	struct FecthLua_Impl<bool> { static bool call( void* p, int32 n ) { return FetchLuaBool( p, n ); } };
	template <>
	struct FecthLua_Impl<uint8> { static uint8 call( void* p, int32 n ) { return FetchLuaInt( p, n ); } };
	template <>
	struct FecthLua_Impl<uint16> { static uint16 call( void* p, int32 n ) { return FetchLuaInt( p, n ); } };
	template <>
	struct FecthLua_Impl<uint32> { static uint32 call( void* p, int32 n ) { return FetchLuaInt( p, n ); } };
	template <>
	struct FecthLua_Impl<uint64> { static uint64 call( void* p, int32 n ) { return FetchLuaInt( p, n ); } };
	template <>
	struct FecthLua_Impl<int8> { static int8 call( void* p, int32 n ) { return FetchLuaInt( p, n ); } };
	template <>
	struct FecthLua_Impl<int16> { static int16 call( void* p, int32 n ) { return FetchLuaInt( p, n ); } };
	template <>
	struct FecthLua_Impl<int32> { static int32 call( void* p, int32 n ) { return FetchLuaInt( p, n ); } };
	template <>
	struct FecthLua_Impl<int64> { static int64 call( void* p, int32 n ) { return FetchLuaInt( p, n ); } };
	template <>
	struct FecthLua_Impl<float> { static float call( void* p, int32 n ) { return FetchLuaNumber( p, n ); } };
	template <>
	struct FecthLua_Impl<CVector2> { static CVector2 call( void* p, int32 n )
	{ CVector2 result; FetchLuaVec( p, n, 2, &result.x ); return result; } };
	template <>
	struct FecthLua_Impl<CVector3> { static CVector3 call( void* p, int32 n )
	{ CVector3 result; FetchLuaVec( p, n, 3, &result.x ); return result; } };
	template <>
	struct FecthLua_Impl<CVector4> { static CVector4 call( void* p, int32 n )
	{ CVector4 result; FetchLuaVec( p, n, 4, &result.x ); return result; } };
	template <>
	struct FecthLua_Impl<CRectangle> { static CRectangle call( void* p, int32 n )
	{ CRectangle result; FetchLuaVec( p, n, 4, &result.x ); return result; } };
	template <>
	struct FecthLua_Impl<const char*> { static const char* call( void* p, int32 n ) { return FetchLuaString( p, n ); } };
	template <>
	struct FecthLua_Impl<CString> { static CString call( void* p, int32 n )
	{
		auto sz = FetchLuaString( p, n );
		return sz ? sz : "";
	} };
	
	template <typename T>
	struct FecthLua_Impl<const T&> { static T call( void* p, int32 n ) { return FecthLua_Impl<T>::call( p, n ); } };

	template <typename T>
	static T FetchLua( void* p, int32 n ) { return FecthLua_Impl<T>::call( p, n ); }
	template <typename T>
	T FetchLua( int32 n ) { return FecthLua_Impl<T>::call( m_pLuaState, n ); }
	template <typename T>
	T PopLuaValue() { T t = FetchLua<T>( -1 ); PopLua( 1 ); return t; }

	static void PushLuaBool( void* p, bool b );
	static void PushLuaInt( void* p, int64 i );
	static void PushLuaNumber( void* p, float f );
	static void PushLuaVec( void* p, float x, float y, float z, float w, int8 n );
	static void PushLuaString( void* p, const char* str );
	static void PushReference( void* p, CReferenceObject* pObj, SClassMetaData* pMetaData );
	static bool PushMetaTable( void* p, SClassMetaData* pMetaData );
	static void PushLuaCFunction( void* p, SLuaCFunction& func );
	static void PushLuaIndex( void* p, int32 i );
	static void SetGlobal( void* p, const char* szName );

	static bool FetchLuaBool( void* p, int32 idx );
	static int64 FetchLuaInt( void* p, int32 idx );
	static float FetchLuaNumber( void* p, int32 idx );
	static void FetchLuaVec( void* p, int32 idx, int32 nCount, float* pVec );
	static const char* FetchLuaString( void* p, int32 idx );
	static CReferenceObject* FetchReference( void* p, int32 idx );
	static void PopLua( void* p, int32 n );

	void PushLuaBool( bool b ) { return PushLuaBool( m_pLuaState, b ); }
	void PushLuaInt( int64 i ) { return PushLuaInt( m_pLuaState, i ); }
	void PushLuaNumber( float f ) { return PushLuaNumber( m_pLuaState, f ); }
	void PushLuaVec( float x, float y, float z, float w, int8 n ) { return PushLuaVec( m_pLuaState, x, y, z, w, n ); }
	void PushLuaString( const char* str ) { return PushLuaString( m_pLuaState, str ); }
	void PushReference( CReferenceObject* pObj, SClassMetaData* pMetaData ) { return PushReference( m_pLuaState, pObj, pMetaData ); }
	void PushLuaIndex( int32 i ) { PushLuaIndex( m_pLuaState, i ); }
	void SetGlobal( const char* szName ) { return SetGlobal( m_pLuaState, szName ); }

	bool FetchLuaBool( int32 idx ) { return FetchLuaBool( m_pLuaState, idx ); }
	int64 FetchLuaInt( int32 idx ) { return FetchLuaInt( m_pLuaState, idx ); }
	float FetchLuaNumber( int32 idx ) { return FetchLuaNumber( m_pLuaState, idx ); }
	void FetchLuaVec( int32 idx, int32 nCount, float* pVec ) { return FetchLuaVec( m_pLuaState, idx, nCount, pVec ); }
	const char* FetchLuaString( int32 idx ) { return FetchLuaString( m_pLuaState, idx ); }
	CReferenceObject* FetchReference( int32 idx ) { return FetchReference( m_pLuaState, idx ); }
	void PopLua( int32 n ) { return PopLua( m_pLuaState, n ); }
	int32 GetTop();

	void NewTable();
	void SetTableIndex( int32 idx, int32 i );

	void Run( const char* str, int32 nRet = 0 );
	void Load( const char* str );
	void Call( int32 nParams, int32 nRet = 0 );

	CLuaState* CreateCoroutine( const char* str );
	CLuaState* CreateCoroutineFromFunc();
	CLuaState* CreateCoroutineAuto();
	CLuaState* CreateNewState();
	bool Resume( int32 nParams, int32* pRet );
	bool Resume( int32 nParams, int32 nRet ) { return Resume( nParams, &nRet ); }
	void CheckError( int32 nType, int32 nResult );

	static CLuaState* GetCurLuaState();
protected:
	virtual void EnsureInited() {}
	void* m_pLuaState;
	int32 m_nRefCount;
	int32 m_nState;
	static vector<CLuaState*> g_vecRunningCoroutines;
};

class CLuaMgr : public CLuaState
{
public:
	virtual void AddRef() override {}
	virtual void Release() override {}

	void InitLua();
	CClassScriptDataLua* CreateScriptData( SClassMetaData* pClassData );

	template <bool RetVoid, typename T>
	struct SLuaCImpl { static void Init( CLuaMgr* pMgr, SLuaCFunction& C, T Func ) { ASSERT( false ); } };
	template <typename T>
	struct SLuaCImplRetUnWr { static void Init( CLuaMgr* pMgr, SLuaCFunction& C, T Func ) { ASSERT( false ); } };

	template<typename T>
	static constexpr bool is_ret_void = false;

	//----------------------------------------------------------------------------------------------------------------

#define LUA_MACRO_TN_SEQ( __n ) LUA_MACRO_TN_SEQ##__n
#define LUA_MACRO_TN_SEQ0
#define LUA_MACRO_TN_SEQ1 T1
#define LUA_MACRO_TN_SEQ2 LUA_MACRO_TN_SEQ1, T2
#define LUA_MACRO_TN_SEQ3 LUA_MACRO_TN_SEQ2, T3
#define LUA_MACRO_TN_SEQ4 LUA_MACRO_TN_SEQ3, T4
#define LUA_MACRO_TN_SEQ5 LUA_MACRO_TN_SEQ4, T5
#define LUA_MACRO_TN_SEQ6 LUA_MACRO_TN_SEQ5, T6
#define LUA_MACRO_TN_SEQ7 LUA_MACRO_TN_SEQ6, T7
#define LUA_MACRO_TN_SEQ8 LUA_MACRO_TN_SEQ7, T8
#define LUA_MACRO_TN_SEQ9 LUA_MACRO_TN_SEQ8, T9

#define LUA_MACRO_TYPENAME_TN_SEQ( __n ) LUA_MACRO_TYPENAME_TN_SEQ##__n
#define LUA_MACRO_TYPENAME_TN_SEQ0
#define LUA_MACRO_TYPENAME_TN_SEQ1 typename T1
#define LUA_MACRO_TYPENAME_TN_SEQ2 LUA_MACRO_TYPENAME_TN_SEQ1, typename T2
#define LUA_MACRO_TYPENAME_TN_SEQ3 LUA_MACRO_TYPENAME_TN_SEQ2, typename T3
#define LUA_MACRO_TYPENAME_TN_SEQ4 LUA_MACRO_TYPENAME_TN_SEQ3, typename T4
#define LUA_MACRO_TYPENAME_TN_SEQ5 LUA_MACRO_TYPENAME_TN_SEQ4, typename T5
#define LUA_MACRO_TYPENAME_TN_SEQ6 LUA_MACRO_TYPENAME_TN_SEQ5, typename T6
#define LUA_MACRO_TYPENAME_TN_SEQ7 LUA_MACRO_TYPENAME_TN_SEQ6, typename T7
#define LUA_MACRO_TYPENAME_TN_SEQ8 LUA_MACRO_TYPENAME_TN_SEQ7, typename T8
#define LUA_MACRO_TYPENAME_TN_SEQ9 LUA_MACRO_TYPENAME_TN_SEQ8, typename T9

#define LUA_MACRO_TYPENAME1_TN_SEQ( __n ) LUA_MACRO_TYPENAME1_TN_SEQ##__n
#define LUA_MACRO_TYPENAME1_TN_SEQ0
#define LUA_MACRO_TYPENAME1_TN_SEQ1 ,typename T1
#define LUA_MACRO_TYPENAME1_TN_SEQ2 LUA_MACRO_TYPENAME1_TN_SEQ1, typename T2
#define LUA_MACRO_TYPENAME1_TN_SEQ3 LUA_MACRO_TYPENAME1_TN_SEQ2, typename T3
#define LUA_MACRO_TYPENAME1_TN_SEQ4 LUA_MACRO_TYPENAME1_TN_SEQ3, typename T4
#define LUA_MACRO_TYPENAME1_TN_SEQ5 LUA_MACRO_TYPENAME1_TN_SEQ4, typename T5
#define LUA_MACRO_TYPENAME1_TN_SEQ6 LUA_MACRO_TYPENAME1_TN_SEQ5, typename T6
#define LUA_MACRO_TYPENAME1_TN_SEQ7 LUA_MACRO_TYPENAME1_TN_SEQ6, typename T7
#define LUA_MACRO_TYPENAME1_TN_SEQ8 LUA_MACRO_TYPENAME1_TN_SEQ7, typename T8
#define LUA_MACRO_TYPENAME1_TN_SEQ9 LUA_MACRO_TYPENAME1_TN_SEQ8, typename T9

#define LUA_MACRO_FETCHLUA_TN_SEQ( __n ) LUA_MACRO_FETCHLUA_TN_SEQ##__n
#define LUA_MACRO_FETCHLUA_TN_SEQ0( i )
#define LUA_MACRO_FETCHLUA_TN_SEQ1( i ) FecthLua_Impl<T1>::call( p, 1 + i )
#define LUA_MACRO_FETCHLUA_TN_SEQ2( i ) LUA_MACRO_FETCHLUA_TN_SEQ1( i ), FecthLua_Impl<T2>::call( p, 2 + i )
#define LUA_MACRO_FETCHLUA_TN_SEQ3( i ) LUA_MACRO_FETCHLUA_TN_SEQ2( i ), FecthLua_Impl<T3>::call( p, 3 + i )
#define LUA_MACRO_FETCHLUA_TN_SEQ4( i ) LUA_MACRO_FETCHLUA_TN_SEQ3( i ), FecthLua_Impl<T4>::call( p, 4 + i )
#define LUA_MACRO_FETCHLUA_TN_SEQ5( i ) LUA_MACRO_FETCHLUA_TN_SEQ4( i ), FecthLua_Impl<T5>::call( p, 5 + i )
#define LUA_MACRO_FETCHLUA_TN_SEQ6( i ) LUA_MACRO_FETCHLUA_TN_SEQ5( i ), FecthLua_Impl<T6>::call( p, 6 + i )
#define LUA_MACRO_FETCHLUA_TN_SEQ7( i ) LUA_MACRO_FETCHLUA_TN_SEQ6( i ), FecthLua_Impl<T7>::call( p, 7 + i )
#define LUA_MACRO_FETCHLUA_TN_SEQ8( i ) LUA_MACRO_FETCHLUA_TN_SEQ7( i ), FecthLua_Impl<T8>::call( p, 8 + i )
#define LUA_MACRO_FETCHLUA_TN_SEQ9( i ) LUA_MACRO_FETCHLUA_TN_SEQ8( i ), FecthLua_Impl<T9>::call( p, 9 + i )

#define LUA_MACRODEF_LUACIMPL_( TN_SEQ, TYPENAME_SEQ, TYPENAME_SEQ1, FETCHLUA_SEQ, FETCHLUA_SEQ1 ) \
	template <typename TRet TYPENAME_SEQ1> \
	struct SLuaCImpl<false, TRet(*)( TN_SEQ )> \
	{ \
		static void Init( CLuaMgr* pMgr, SLuaCFunction& C, TRet( *Func )( TN_SEQ ) ) \
		{ \
			C.Func = [pMgr, Func] ( void* p ) { PushLua( p, ( *Func )( FETCHLUA_SEQ ) ); return 1; }; \
		} \
	}; \
	template <TYPENAME_SEQ> \
	struct SLuaCImpl<true, void(*)( TN_SEQ )> \
	{ \
		static void Init( CLuaMgr* pMgr, SLuaCFunction& C, void( *Func )( TN_SEQ ) ) \
		{ \
			C.Func = [pMgr, Func] ( void* p ) { ( *Func )( FETCHLUA_SEQ ); return 0; }; \
		} \
	}; \
	template <TYPENAME_SEQ> \
	struct SLuaCImplRetUnWr<int32(*)( TN_SEQ )> \
	{ \
		static void Init( CLuaMgr* pMgr, SLuaCFunction& C, int32( *Func )( TN_SEQ ) ) \
		{ \
			C.Func = [pMgr, Func] ( void* p ) { return ( *Func )( FETCHLUA_SEQ ); }; \
		} \
	}; \
	template <typename TRet, typename TThis TYPENAME_SEQ1> \
	struct SLuaCImpl<false, TRet( TThis::* )( TN_SEQ )> \
	{ \
		static void Init( CLuaMgr* pMgr, SLuaCFunction& C, TRet( TThis::*Func )( TN_SEQ ) ) \
		{ \
			C.Func = [pMgr, Func] ( void* p ) { PushLua( p, ( FetchLua<TThis*>( p, 1 )->*Func )( FETCHLUA_SEQ1 ) ); return 1; }; \
		} \
	}; \
	template <typename TThis TYPENAME_SEQ1> \
	struct SLuaCImpl<true, void( TThis::* )( TN_SEQ )> \
	{ \
		static void Init( CLuaMgr* pMgr, SLuaCFunction& C, void( TThis::*Func )( TN_SEQ ) ) \
		{ \
			C.Func = [pMgr, Func] ( void* p ) { ( FetchLua<TThis*>( p, 1 )->*Func )( FETCHLUA_SEQ1 ); return 0; }; \
		} \
	}; \
	template <typename TThis TYPENAME_SEQ1> \
	struct SLuaCImplRetUnWr<int32( TThis::* )( TN_SEQ )> \
	{ \
		static void Init( CLuaMgr* pMgr, SLuaCFunction& C, int32( TThis::*Func )( TN_SEQ ) ) \
		{ \
			C.Func = [pMgr, Func] ( void* p ) { return ( FetchLua<TThis*>( p, 1 )->*Func )( FETCHLUA_SEQ1 ); }; \
		} \
	}; \
	template <typename TRet, typename TThis TYPENAME_SEQ1> \
	struct SLuaCImpl<false, TRet( TThis::* )( TN_SEQ ) const> \
	{ \
		static void Init( CLuaMgr* pMgr, SLuaCFunction& C, TRet( TThis::*Func )( TN_SEQ ) const ) \
		{ \
			C.Func = [pMgr, Func] ( void* p ) { PushLua( p, ( FetchLua<TThis*>( p, 1 )->*Func )( FETCHLUA_SEQ1 ) ); return 1; }; \
		} \
	}; \
	template <typename TThis TYPENAME_SEQ1> \
	struct SLuaCImpl<true, void( TThis::* )( TN_SEQ ) const> \
	{ \
		static void Init( CLuaMgr* pMgr, SLuaCFunction& C, void( TThis::*Func )( TN_SEQ ) const ) \
		{ \
			C.Func = [pMgr, Func] ( void* p ) { ( FetchLua<TThis*>( p, 1 )->*Func )( FETCHLUA_SEQ1 ); return 0; }; \
		} \
	}; \
	template <typename TThis TYPENAME_SEQ1> \
	struct SLuaCImplRetUnWr<int32( TThis::* )( TN_SEQ ) const> \
	{ \
		static void Init( CLuaMgr* pMgr, SLuaCFunction& C, int32( TThis::*Func )( TN_SEQ ) const ) \
		{ \
			C.Func = [pMgr, Func] ( void* p ) { return ( FetchLua<TThis*>( p, 1 )->*Func )( FETCHLUA_SEQ1 ); }; \
		} \
	}; \
	template<typename TRet TYPENAME_SEQ1> \
	static constexpr bool is_ret_void<TRet(*)( TN_SEQ )> = is_void<TRet>::value; \
	template<typename TRet, typename TThis TYPENAME_SEQ1> \
	static constexpr bool is_ret_void<TRet( TThis::* )( TN_SEQ )> = is_void<TRet>::value; \
	template<typename TRet, typename TThis TYPENAME_SEQ1> \
	static constexpr bool is_ret_void<TRet( TThis::* )( TN_SEQ ) const> = is_void<TRet>::value;

#define LUA_MACRODEF_LUACIMPL( __n ) LUA_MACRODEF_LUACIMPL_( LUA_MACRO_TN_SEQ( __n ), LUA_MACRO_TYPENAME_TN_SEQ( __n ), LUA_MACRO_TYPENAME1_TN_SEQ( __n ), LUA_MACRO_FETCHLUA_TN_SEQ( __n )( 0 ), LUA_MACRO_FETCHLUA_TN_SEQ( __n )( 1 ) )

	LUA_MACRODEF_LUACIMPL( 0 )
	LUA_MACRODEF_LUACIMPL( 1 )
	LUA_MACRODEF_LUACIMPL( 2 )
	LUA_MACRODEF_LUACIMPL( 3 )
	LUA_MACRODEF_LUACIMPL( 4 )
	LUA_MACRODEF_LUACIMPL( 5 )
	LUA_MACRODEF_LUACIMPL( 6 )
	LUA_MACRODEF_LUACIMPL( 7 )
	LUA_MACRODEF_LUACIMPL( 8 )
	LUA_MACRODEF_LUACIMPL( 9 )

//----------------------------------------------------------------------------------------------------------------
	template<typename T>
	int32 BindLuaCFunc( const char* szName, T Func, bool bGlobal = false )
	{
		m_vecLuaCFuncs.resize( m_vecLuaCFuncs.size() + 1 );
		SLuaCFunction& C = m_vecLuaCFuncs.back();
		SLuaCImpl<is_ret_void<T>, T>::Init( this, C, Func );
		C.strFuncName = szName;
		if( bGlobal )
			m_vecGlobalLuaCFuncs.push_back( m_vecLuaCFuncs.size() - 1 );
		return m_vecLuaCFuncs.size() - 1;
	}
	template<typename T>
	int32 BindLuaCFuncRetUnWr( const char* szName, T Func, bool bGlobal = false )
	{
		m_vecLuaCFuncs.resize( m_vecLuaCFuncs.size() + 1 );
		SLuaCFunction& C = m_vecLuaCFuncs.back();
		SLuaCImplRetUnWr<T>::Init( this, C, Func );
		C.strFuncName = szName;
		if( bGlobal )
			m_vecGlobalLuaCFuncs.push_back( m_vecLuaCFuncs.size() - 1 );
		return m_vecLuaCFuncs.size() - 1;
	}
	void InitMetaTable( CClassScriptDataLua* pScriptData );
	DECLARE_GLOBAL_INST_REFERENCE( CLuaMgr )
protected:
	virtual void EnsureInited() override { InitLua(); }
	vector<CReference<CClassScriptDataLua> > m_vecScriptData;
	vector<SLuaCFunction> m_vecLuaCFuncs;
	vector<int32> m_vecGlobalLuaCFuncs;
};

#define DEFINE_LUA_REF_OBJECT() \
	auto pScript = CLuaMgr::Inst().CreateScriptData( pData );

#define REGISTER_LUA_CFUNCTION( Name ) \
	pScript->vecLuaCFuncs.push_back( CLuaMgr::Inst().BindLuaCFunc( #Name, &__cur_class::Name ) ); \

#define REGISTER_LUA_CFUNCTION_GLOBAL( Name ) \
	CLuaMgr::Inst().BindLuaCFunc( #Name, &Name, true ); \

#define REGISTER_LUA_CFUNCTION_RETUNWR( Name ) \
	pScript->vecLuaCFuncs.push_back( CLuaMgr::Inst().BindLuaCFuncRetUnWr( #Name, &__cur_class::Name ) ); \

#define REGISTER_LUA_CFUNCTION_GLOBAL_RETUNWR( Name ) \
	CLuaMgr::Inst().BindLuaCFuncRetUnWr( #Name, &Name, true ); \
	