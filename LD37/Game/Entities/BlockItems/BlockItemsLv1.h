#pragma once
#include "CommonBlockItems.h"
#include "Entities/AIObject.h"
#include "Entities/EffectObject.h"

class CPipe0 : public CDetectTrigger
{
	friend void RegisterGameClasses();
public:
	CPipe0( const SClassCreateContext& context ) : CDetectTrigger( context ) { SET_BASEOBJECT_ID( CPipe0 ); }
protected:
	virtual void Trigger() override;
};

class CWindow : public CEntity
{
	friend void RegisterGameClasses();
public:
	CWindow( const SClassCreateContext& context ) : CEntity( context ), m_pAI( NULL ), m_bHit( false )
		, m_strBullet( context ), m_strBullet1( context )
		, m_strHead( context ), m_strHead1( context ), m_strHead2( context ), m_strHead3( context ) { SET_BASEOBJECT_ID( CWindow ); }
	virtual void OnAddedToStage() override { if( m_pSpawner ) m_pSpawner->SetEnabled( false ); m_pAI = new AI(); m_pAI->SetParentEntity( this ); }
protected:
	virtual void AIFunc();
	class AI : public CAIObject
	{
	protected:
		virtual void AIFunc() override { static_cast<CWindow*>( GetParentEntity() )->AIFunc(); }
	};
	AI* m_pAI;

	CRectangle m_openRect;
	CRectangle m_closeRect;
	CReference<CRenderObject2D> m_pWindow;
	CReference<CRenderObject2D> m_pMan;
	CReference<CEffectObject> m_pDeathEffect;
	CReference<CSpawner> m_pSpawner;
	bool m_bHit;

	CString m_strBullet;
	CString m_strBullet1;
	CString m_strHead;
	CString m_strHead1;
	CString m_strHead2;
	CString m_strHead3;
	CReference<CPrefab> m_pBullet;
	CReference<CPrefab> m_pBullet1;
	CReference<CPrefab> m_pHead[4];
};