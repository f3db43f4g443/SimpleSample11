#include "Common.h"
#include "Coroutine.h"

ICoroutine::ICoroutine( uint32 nStackSize )
	: m_nStackSize( nStackSize )
	, m_eState( eState_Stopped )
{
}

ICoroutine::~ICoroutine()
{
}

//继续执行
uint32 ICoroutine::Resume()
{
	if( m_eState != eState_Pending )
		return 0;
	m_eState = eState_Running;
	return _Resume();
}

#define USE_BOOST
#if defined _WIN32 && !defined USE_BOOST
#include <setjmp.h>

class CCoroutine : public ICoroutine
{
public:
	CCoroutine( uint32 nStackSize );
	~CCoroutine();

	void Create( CoroutineFunc pFunc, void* pContext );
	void Yield( uint32 nReturnValue );
protected:
	void _Create( CoroutineFunc pFunc, void* pContext );
	uint32 _Resume();
private:
	uint8* m_pStack;
	jmp_buf m_resumeState;
	jmp_buf m_yieldState;
	uint32 m_nReturnValue;
};

CCoroutine::CCoroutine( uint32 nStackSize )
	: ICoroutine( nStackSize )
{
	m_pStack = (uint8*)malloc( nStackSize );
}

CCoroutine::~CCoroutine()
{
	free( m_pStack );
}

//初始化入口函数和参数
void CCoroutine::Create( CoroutineFunc pFunc, void* pContext )
{
	if( m_eState != eState_Stopped )
		return;
	m_eState = eState_Pending;
	if( !setjmp( m_yieldState ) )
	{
		//first call
		uint8* pStackBottom = m_pStack + m_nStackSize;
		static CCoroutine* g_pThis;
		static CoroutineFunc g_pFunc;
		static void* g_pContext;
		g_pThis = this;
		g_pFunc = pFunc;
		g_pContext = pContext;

		//switch the stack
		_asm
		{
			mov esp, pStackBottom
		}
		g_pThis->_Create( g_pFunc, g_pContext );
	}
	else
	{
		//jump back
	}
}

void CCoroutine::_Create( CoroutineFunc pFunc, void* pContext )
{
	if( !setjmp( m_resumeState ) )
	{
		//first call
		longjmp( m_yieldState, 1 );
	}
	else
	{
		//jump back
		m_nReturnValue = (*pFunc)( pContext );
		//finished
		m_eState = eState_Stopped;
		int* pReg = &m_yieldState[6];
		_asm
		{
			mov eax, dword ptr fs:[00000000h]
			mov ebx, dword ptr pReg
			mov dword ptr [ebx], eax
		}
		longjmp( m_yieldState, 1 );
	}
}

uint32 CCoroutine::_Resume()
{
	if( !setjmp( m_yieldState ) )
	{
		//first call
		int* pReg = &m_resumeState[6];
		_asm
		{
			mov eax, dword ptr fs:[00000000h]
			mov ebx, dword ptr pReg
			mov dword ptr [ebx], eax
		}
		longjmp( m_resumeState, 1 );
	}
	else
	{
		//jump back
		//yielded or finished
		return m_nReturnValue;
	}
	//should not reach here
	return *(uint32*)NULL;
}

//暂时返回
void CCoroutine::Yield( uint32 nReturnValue )
{
	m_nReturnValue = nReturnValue;
	m_eState = eState_Pending;
	if( !setjmp( m_resumeState ) )
	{
		//first call
		int* pReg = &m_yieldState[6];
		_asm
		{
			mov eax, dword ptr fs:[00000000h]
			mov ebx, dword ptr pReg
			mov dword ptr [ebx], eax
		}
		longjmp( m_yieldState, 1 );
	}
}

#else
#include "boost/coroutine/all.hpp"

class CCoroutine : public ICoroutine
{
public:
	typedef boost::coroutines::coroutine< void >::pull_type pull_coro_t;
	typedef boost::coroutines::coroutine< void >::push_type push_coro_t;

	CCoroutine( uint32 nStackSize );
	~CCoroutine();

	void Create( CoroutineFunc pFunc, void* pContext );
	void Yield( uint32 nReturnValue );
protected:
	static void WorkFunc( push_coro_t& caller );
	uint32 _Resume();
private:
	pull_coro_t* m_resumeState;
	push_coro_t* m_yieldState;
	uint32 m_nReturnValue;
	CoroutineFunc m_pFunc;
	void* m_pContext;

	static CCoroutine* s_pTempThis;
};

CCoroutine* CCoroutine::s_pTempThis;
CCoroutine::CCoroutine( uint32 nStackSize )
	: ICoroutine( nStackSize )
	, m_resumeState( NULL )
	, m_yieldState( NULL )
{
}

CCoroutine::~CCoroutine()
{
	if( m_resumeState )
	{
		delete m_resumeState;
		m_resumeState = NULL;
	}
}

//初始化入口函数和参数
void CCoroutine::Create( CoroutineFunc pFunc, void* pContext )
{
	if( m_eState != eState_Stopped )
		return;
	m_eState = eState_Pending;
	if( m_resumeState )
	{
		delete m_resumeState;
		m_resumeState = NULL;
	}

	m_pFunc = pFunc;
	m_pContext = pContext;
	s_pTempThis = this;
	boost::coroutines::attributes attr;
	attr.size = m_nStackSize;
	m_resumeState = new pull_coro_t( &CCoroutine::WorkFunc, attr );
}

void CCoroutine::WorkFunc( push_coro_t& caller )
{
	CCoroutine* pThis = s_pTempThis;
	pThis->m_yieldState = &caller;
	caller();
	pThis->m_nReturnValue = (*pThis->m_pFunc)( pThis->m_pContext );
	pThis->m_eState = eState_Stopped;
}

uint32 CCoroutine::_Resume()
{
	s_pTempThis = this;
	(*m_resumeState)();
	return m_nReturnValue;
}

//暂时返回
void CCoroutine::Yield( uint32 nReturnValue )
{
	m_nReturnValue = nReturnValue;
	m_eState = eState_Pending;

	(*m_yieldState)();
}

#endif

//创建对象
ICoroutine* ICoroutine::CreateCoroutine( uint32 nStackSize )
{
	return new CCoroutine( nStackSize );
}
