#pragma once
#include "CommonBlockItems.h"
#include "Entities/AIObject.h"
#include "Entities/EffectObject.h"
#include "Enemy.h"
#include "Entities/Decorator.h"
#include "Render/DrawableGroup.h"

//class CPipe0 : public CDetectTrigger
//{
//	friend void RegisterGameClasses();
//public:
//	CPipe0( const SClassCreateContext& context ) : CDetectTrigger( context ) { SET_BASEOBJECT_ID( CPipe0 ); }
//protected:
//	virtual void Trigger() override;
//};

class CPipe1 : public CEntity
{
	friend void RegisterGameClasses();
public:
	CPipe1( const SClassCreateContext& context ) : CEntity( context ), m_onDamaged( this, &CPipe1::OnDamaged ), m_onTick( this, &CPipe1::OnTick ) { SET_BASEOBJECT_ID( CPipe1 ); }
	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;
private:
	void OnDamaged();
	void OnTick();
	float m_fBulletSpeed;
	uint32 m_nBulletLife;
	uint32 m_nBulletCD;
	TResourceRef<CPrefab> m_pBullet;

	class CChunkObject* m_pChunkObject;
	CVector2 m_dir;
	TClassTrigger<CPipe1> m_onDamaged;
	TClassTrigger<CPipe1> m_onTick;
};

class CWindow0 : public CDecorator
{
	friend void RegisterGameClasses();
public:
	CWindow0( const SClassCreateContext& context ) : CDecorator( context ), m_onDamaged( this, &CWindow0::OnDamaged ), m_pChunkObject( NULL ) { SET_BASEOBJECT_ID( CWindow0 ); }
	virtual void Init( const CVector2& size, SChunk* pPreParent ) override;
	virtual void OnRemovedFromStage() override;
private:
	void OnDamaged();
	void CreateBullets( int32 nWindow, int32 h, int32 nType );
	void Refresh( int32 nWindow, float fPercent );
	struct SWindow
	{
		TRectangle<int32> rect;
		CReference<CRenderObject2D> p1;
		CReference<CRenderObject2D> p2;
		float fHpPercent;
		float fHpPercent0;
		int32 h1, h2, h3;
		float fCurPercent;
	};
	vector<SWindow> m_vecWindows;

	CReference<CEntity> m_pWood;
	CReference<CEntity> m_pFlesh;
	TResourceRef<CPrefab> m_pEft;
	TResourceRef<CPrefab> m_pBullet;
	TResourceRef<CPrefab> m_pBullet1;
	int8 m_nTag;

	class CChunkObject* m_pChunkObject;
	TClassTrigger<CWindow0> m_onDamaged;
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
	void AIFunc();
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

class CWindow1 : public CDecorator
{
	friend void RegisterGameClasses();
public:
	CWindow1( const SClassCreateContext& context ) : CDecorator( context ) { SET_BASEOBJECT_ID( CWindow1 ); }
	virtual void Init( const CVector2& size, SChunk* pPreParent ) override;
protected:
	void AIFunc();
	void AIFunc1( class CChunkObject* pChunkObject );
	void AIFunc2( class CChunkObject* pChunkObject );
	void AIFunc3( class CChunkObject* pChunkObject );
	class AI : public CAIObject
	{
	protected:
		virtual void AIFunc() override { static_cast<CWindow1*>( GetParentEntity() )->AIFunc(); }
	};
	AI* m_pAI;
	CVector2 m_size;
	vector<CReference<CRenderObject2D> > m_vecRenderObject;

	CReference<CEntity> m_p1;
	uint8 m_nType;
	uint32 m_nLayer2Rows;
	uint32 m_nLayer2Cols;
	TResourceRef<CDrawableGroup> m_pDrawable1;
	uint32 m_nDecoTexSize;
	TResourceRef<CPrefab> m_pPrefab;
};

class CWindow2 : public CEntity
{
	friend void RegisterGameClasses();
public:
	CWindow2( const SClassCreateContext& context ) : CEntity( context ), m_strBullet( context ), m_strBullet1( context ) { SET_BASEOBJECT_ID( CWindow2 ); }
	virtual void OnAddedToStage() override;
protected:
	void AIFunc();
	class AI : public CAIObject
	{
	protected:
		virtual void AIFunc() override { static_cast<CWindow2*>( GetParentEntity() )->AIFunc(); }
	};
	AI* m_pAI;

	void AIFuncEye( uint8 nEye );
	void AIFuncEye1( uint8 nEye );
	void AIFuncEye2( uint8 nEye );
	void AIFuncEye3( uint8 nEye );
	void UpdateLink( uint8 nEye );
	class AIEye : public CAIObject
	{
	public:
		AIEye( uint8 nEye ) : m_nEye( nEye ) {}
	protected:
		virtual void AIFunc() override { static_cast<CWindow2*>( GetParentEntity() )->AIFuncEye( m_nEye ); }
		uint8 m_nEye;
	};
	AIEye* m_pAIEye[2];

	CRectangle m_openRect;
	CRectangle m_closeRect;
	CReference<CRenderObject2D> m_pWindow;
	CReference<CEntity> m_pMan;
	CReference<CRenderObject2D> m_pEye[2];
	CReference<CEnemy> m_pHead[2];
	CReference<CRenderObject2D> m_pLinks[2];
	int32 m_nLinkCount;
	CString m_strBullet;
	CString m_strBullet1;
	CReference<CPrefab> m_pBullet;
	CReference<CPrefab> m_pBullet1;
};

class CHouse0Deco : public CDecorator
{
	friend void RegisterGameClasses();
public:
	CHouse0Deco( const SClassCreateContext& context ) : CDecorator( context ) { SET_BASEOBJECT_ID( CHouse0Deco ); }
	virtual void Init( const CVector2& size, SChunk* pPreParent ) override;
};