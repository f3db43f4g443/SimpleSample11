#pragma once
#include "LinkList.h"
#include <map>
#include <functional>
using namespace std;

class CTrigger
{
public:
	CTrigger() : __pPrevTrigger( NULL ), m_bValidFlag( true ), bAutoDelete( false ) {}
	virtual ~CTrigger() { bAutoDelete = false; if( IsRegistered() ) Unregister(); }

	virtual void Run( void* pContext ) = 0;
	virtual void OnRemoved() { if( bAutoDelete ) delete this; }
	bool IsRegistered() { return __pPrevTrigger != NULL; }
	void OnTimer();

	void Unregister();
	bool m_bValidFlag;
	bool bAutoDelete;
	LINK_LIST( CTrigger, Trigger )
};

class CFunctionTrigger : public CTrigger
{
public:
	CFunctionTrigger() {}
	CFunctionTrigger( function<void()> func ) : m_func( func ) {}
	void Set( function<void()> func ) { m_func = func; }
	void Run( void* pContext ) { m_func(); }
private:
	function<void()> m_func;
};

template <typename TParam = void*>
class CFunctionTrigger1 : public CTrigger
{
public:
	CFunctionTrigger1() {}
	CFunctionTrigger1( function<void( TParam )> func ) : m_func( func ) {}
	void Set( function<void( TParam )> func ) { m_func = func; }
	void Run( void* pContext ) { m_func( (TParam)( pContext ) ); }
private:
	function<void( TParam )> m_func;
};

template <class T>
class TClassTrigger : public CTrigger
{
public:
	typedef void (T::*EventCallbackFunc)();
	TClassTrigger() {}
	TClassTrigger( T* pObj, EventCallbackFunc fun ) : m_pObj( pObj ), m_fun( fun ) {}
	void Set( T* pObj, EventCallbackFunc fun ) { m_pObj = pObj; m_fun = fun; }
	void Run( void* pContext ) { (m_pObj->*m_fun)(); }
private:
	T* m_pObj;
	EventCallbackFunc m_fun;
};

template <class T, typename TParam = void*>
class TClassTrigger1 : public CTrigger
{
public:
	typedef void (T::*EventCallbackFunc)( TParam pContext );
	TClassTrigger1() {}
	TClassTrigger1( T* pObj, EventCallbackFunc fun ) : m_pObj( pObj ), m_fun( fun ) {}
	void Set( T* pObj, EventCallbackFunc fun ) { m_pObj = pObj; m_fun = fun; }
	void Run( void* pContext ) { (m_pObj->*m_fun)( (TParam)( pContext ) ); }
private:
	T* m_pObj;
	EventCallbackFunc m_fun;
};

template <int iCount>
class CEventTrigger
{
public:
	CEventTrigger() { memset( m_triggers, 0, sizeof( m_triggers ) ); }
	~CEventTrigger() { Clear(); }

	void Trigger( uint32 iEvent, void* pContext )
	{
		if( iEvent >= iCount ) return;
		LINK_LIST_FOR_EACH_BEGIN( pTrigger, m_triggers[iEvent], CTrigger, Trigger )
			if( pTrigger->m_bValidFlag )
				pTrigger->Run( pContext );
		LINK_LIST_FOR_EACH_END( pTrigger, m_triggers[iEvent], CTrigger, Trigger )
	}
	void Clear()
	{
		for( int i = 0; i < iCount; i++ )
		{
			while( m_triggers[i] )
				m_triggers[i]->Unregister();
		}
	}
	void Register( uint32 iEvent, CTrigger* pTrigger )
	{
		if( pTrigger->IsRegistered() )
			pTrigger->Unregister();
		Insert_Trigger( pTrigger, iEvent );
	}
	LINK_LIST_HEAD_ARR( m_triggers, iCount, CTrigger, Trigger )
};

#define DECLARE_EVENT_TRIGGER( name ) \
	private: \
		CEventTrigger<1> m_trigger_##name; \
	public: \
		void Register_##name( CTrigger* pTrigger ) { m_trigger_##name.Register( 0, pTrigger ); } \
		void Trigger_##name( void* pContext ) { m_trigger_##name.Trigger( 0, pContext ); } \


template <int iCount>
class CTimedTrigger
{
public:
	CTimedTrigger() { m_nTime = 0; memset( m_entries, 0, sizeof( m_entries ) ); }

	int Register( uint32 nTime, CTrigger* pTrigger )
	{
		if( nTime <= 0 ) return 0;
		unsigned int triggerTime = nTime + m_nTime;
		CTimedTriggerEntry** ppEntry = m_entries + (triggerTime % iCount);
		int dTime;
		CTimedTriggerEntry* pEntry = *ppEntry;
		while( pEntry )
		{
			dTime = pEntry->nTime - triggerTime;
			if( dTime >= 0 )
				break;
			ppEntry = &( pEntry->NextEntry() );
			pEntry = *ppEntry;
		}
		if( !pEntry || dTime )
		{
			pEntry = new CTimedTriggerEntry( triggerTime );
			pEntry->InsertTo_Entry( *ppEntry );
		}
		pEntry->Register( 0, pTrigger );
		return triggerTime;
	}
	void Clear()
	{
		for( int i = 0; i < iCount; i++ )
		{
			CTimedTriggerEntry* pEntry = m_entries[i];
			while( pEntry )
			{
				pEntry->Clear();
				pEntry->RemoveFrom_Entry();
				delete pEntry;
				pEntry = m_entries[i];
			}
		}
	}
	int GetTimeStamp() { return m_nTime; }
	void UpdateTime()
	{
		m_nTime++;
		CTimedTriggerEntry* pEntry = m_entries[((unsigned int)m_nTime) % iCount];
		while( pEntry )
		{
			int dTime = pEntry->nTime - m_nTime;
			if( !dTime )
			{
				for( CTrigger* pTrigger = pEntry->m_triggers[0]; pTrigger; pTrigger = pEntry->m_triggers[0] )
				{
					pTrigger->OnTimer();
				}
				pEntry->RemoveFrom_Entry();
				delete pEntry;
				break;
			}
			else if( dTime > 0 )
				break;
			pEntry = pEntry->NextEntry();
		}
	}
private:
	class CTimedTriggerEntry : public CEventTrigger<1>
	{
		friend class CTimedTrigger<iCount>;
	public:
		CTimedTriggerEntry( int nTime ) : nTime( nTime ) {}
		int nTime;
		LINK_LIST( CTimedTriggerEntry, Entry );
	};

	int m_nTime;
	CTimedTriggerEntry* m_entries[iCount];
};

