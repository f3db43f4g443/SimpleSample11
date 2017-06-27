#pragma once
#include "Entity.h"

class CAIObject : public CEntity
{
public:
	CAIObject() : m_onTick( this, &CAIObject::OnTick ) {}
	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;
	void Yield();
	void Yield( float fTime, bool bAfterHitTest );
	bool IsRunning();

	template<typename T>
	void Throw( T t )
	{
		m_resumeFunc = [=] () { throw( t ); };
		ResumeThrow();
	}

	struct SExceptionRemoved {};
protected:
	virtual void AIFunc() {}
private:
	void OnTick();

	static uint32 CoroutineFunc( void* pThis );

	void OnResumeRet( uint32 nRet );
	void ResumeThrow();

	class ICoroutine* m_pCoroutine;
	float m_fTime;
	bool m_bAfterHitTest;

	function<void()> m_resumeFunc;

	TClassTrigger<CAIObject> m_onTick;
};

class CMessagePump
{
public:
	CMessagePump( CAIObject* pContext ) : m_pContext( pContext ), m_pItems( NULL ) {}
	~CMessagePump() { UnRegisterAll(); }
	void UnRegisterAll();

	template<typename T>
	CTrigger* Register()
	{
		auto pItem = new TItem<T>( m_pContext );
		Insert_Item( pItem );
		return &pItem->trigger;
	}
private:
	struct SItem
	{
		virtual ~SItem() {}
		LINK_LIST( SItem, Item );
	};
	template<typename T>
	struct TItem : public SItem
	{
		TItem( CAIObject* pContext ) : trigger( this, &TItem::OnTrigger ), m_pContext( pContext ) {}
		void OnTrigger( T t )
		{
			m_pContext->Throw( t );
		}
		TClassTrigger1<TItem, T> trigger;
		CAIObject* m_pContext;
	};

	CAIObject* m_pContext;
	LINK_LIST_HEAD( m_pItems, SItem, Item );
};