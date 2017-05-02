#include "stdafx.h"
#include "AIObject.h"
#include "Common/Coroutine.h"
#include "Stage.h"

#define STACK_SIZE 0x10000

void CAIObject::OnAddedToStage()
{
	m_pCoroutine = TCoroutinePool<STACK_SIZE>::Inst().Alloc();
	m_pCoroutine->Create( &CAIObject::CoroutineFunc, this );
	m_resumeFunc = nullptr;
	uint32 nRet = m_pCoroutine->Resume();
	OnResumeRet( nRet );
}

void CAIObject::OnRemovedFromStage()
{
	if( m_onTick.IsRegistered() )
		m_onTick.Unregister();
	if( !m_pCoroutine || m_pCoroutine->GetState() == ICoroutine::eState_Running )
		return;

	Throw( SExceptionRemoved() );
	TCoroutinePool<STACK_SIZE>::Inst().Free( m_pCoroutine );
	m_pCoroutine = NULL;
}

void CAIObject::Yield()
{
	m_pCoroutine->Yield( 1 );

	if( m_resumeFunc )
	{
		auto func = m_resumeFunc;
		m_resumeFunc = nullptr;
		func();
	}
}

void CAIObject::Yield( float fTime, bool bAfterHitTest )
{
	m_fTime = fTime;
	m_bAfterHitTest = bAfterHitTest;
	m_pCoroutine->Yield( 0 );

	if( m_resumeFunc )
	{
		auto func = m_resumeFunc;
		m_resumeFunc = nullptr;
		func();
	}
}
	
void CAIObject::OnTick()
{
	float dTime = GetStage()->GetElapsedTimePerTick();
	m_fTime -= dTime;
	if( m_fTime > 0 )
	{
		if( m_bAfterHitTest )
			GetStage()->RegisterAfterHitTest( 1, &m_onTick );
		else
			GetStage()->RegisterBeforeHitTest( 1, &m_onTick );
		return;
	}

	CReference<CAIObject> temp = this;
	uint32 nRet = m_pCoroutine->Resume();
	OnResumeRet( nRet );
}

uint32 CAIObject::CoroutineFunc( void* pThis )
{
	CAIObject* pObject = (CAIObject*)pThis;

	try
	{
		pObject->AIFunc();
	}
	catch( SExceptionRemoved e )
	{
		return 1;
	}
	return 1;
}

void CAIObject::OnResumeRet( uint32 nRet )
{
	if( !GetStage() )
	{
		OnRemovedFromStage();
		return;
	}

	if( nRet == 0 )
	{
		if( m_bAfterHitTest )
			GetStage()->RegisterAfterHitTest( 1, &m_onTick );
		else
			GetStage()->RegisterBeforeHitTest( 1, &m_onTick );
	}
}

void CAIObject::ResumeThrow()
{
	if( m_onTick.IsRegistered() )
		m_onTick.Unregister();
	if( m_pCoroutine->GetState() == ICoroutine::eState_Running )
		m_resumeFunc();
	else
	{
		auto pStage = GetStage();
		CReference<CAIObject> temp = this;
		uint32 nRet = m_pCoroutine->Resume();
		if( pStage )
			OnResumeRet( nRet );
	}
}

void CMessagePump::UnRegisterAll()
{
	while( m_pItems )
	{
		auto pItem = m_pItems;
		pItem->RemoveFrom_Item();
		delete pItem;
	}
}