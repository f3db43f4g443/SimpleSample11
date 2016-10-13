#pragma once
#include "Entity.h"
#include "FaceEditItem.h"
#include "Common/StringUtil.h"

#include "Entities/OrganHpBar.h"

class CTurnBasedContext;
class CCharacter;
struct SOrganActionContext
{
	SOrganActionContext() : pCharacter( NULL ), pOrgan( NULL ), pCurTarget( NULL ), bSucceed( false ) {}
	CCharacter* pCharacter;
	class COrgan* pOrgan;
	TVector2<int> target;
	uint8 nCharge;

	bool bSucceed;

	vector<CCharacter*> targetCharacters;
	CCharacter* pCurTarget;
};

enum
{
	eRangeType_Normal,

};

enum
{
	eTargetType_None,
	eTergetType_Pos,
	eTargetType_Character
};

class COrganAction : public CEntity
{
	friend void RegisterGameClasses();
public:
	COrganAction( const SClassCreateContext& context ) : CEntity( context ) { SET_BASEOBJECT_ID( COrganAction ); }
	virtual void Action( CTurnBasedContext* pContext, SOrganActionContext& actionContext ) {}

};

class COrganTargetor : public CEntity
{
	friend void RegisterGameClasses();
public:
	COrganTargetor( const SClassCreateContext& context ) : CEntity( context ) { SET_BASEOBJECT_ID( COrganTargetor ); }
	virtual void FindTargets( CTurnBasedContext* pContext, SOrganActionContext& actionContext ) {}

	typedef function<bool( CCharacter* pChar, CTurnBasedContext* pContext, SOrganActionContext& actionContext )> FuncOnFindTarget;
	void SetFindTargetFunc( FuncOnFindTarget func ) { m_onFindTarget = func; }
protected:
	void FindTarget( CCharacter* pChar, CTurnBasedContext* pContext, SOrganActionContext& actionContext )
	{
		if( m_onFindTarget && !m_onFindTarget( pChar, pContext, actionContext ) )
			return;
		actionContext.targetCharacters.push_back( pChar );
	}
private:
	FuncOnFindTarget m_onFindTarget;
};

class COrgan : public CEntity
{
	friend class CFace;
	friend class COrganEditItem;
	friend void RegisterGameClasses();
public:
	COrgan( const SClassCreateContext& context ) : CEntity( context ), m_nHp( m_nMaxHp ), m_pFace( NULL ), m_pos( 0, 0 ), m_strOrganAction( context ), m_strOrganTargetor( context )
	{ SET_BASEOBJECT_ID( COrgan ); }

	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;

	CFace* GetFace() { return m_pFace; }
	uint32 GetWidth() { return m_nWidth; }
	uint32 GetHeight() { return m_nHeight; }
	TVector2<int32> GetGridPos() { return m_pos; }

	uint32 GetCurHp() { return m_nHp; }
	uint32 GetMaxHp() { return m_nMaxHp; }

	void GetRange( vector<TVector2<int32> >& result );
	uint32 GetCost() { return m_nCost; }
	bool IsInRange( const TVector2<int32>& pos );
	bool CanAction( SOrganActionContext& actionContext );
	bool CheckActionTarget( SOrganActionContext& actionContext );

	void Action( CTurnBasedContext* pContext, SOrganActionContext& actionContext );
	void ActionSelectTarget( CTurnBasedContext* pContext, SOrganActionContext& actionContext );
	void ActionSelectTarget( CTurnBasedContext* pContext, SOrganActionContext& actionContext, COrganTargetor::FuncOnFindTarget func );

	void SetHp( uint32 nHp );
	void Damage( uint32 nDmg );

	void ShowHpBar( bool bShown );

	CPrefab* GetActionPrefab() { return m_pOrganActionPrefab; }
	CPrefab* GetTargetorPrefab() { return m_pOrganTargetorPrefab; }

	DECLARE_EVENT_TRIGGER( OnHpChanged )
private:
	CFace* m_pFace;
	TVector2<int32> m_pos;

	uint32 m_nWidth, m_nHeight;
	uint32 m_nHp, m_nMaxHp;

	uint32 m_nCost;
	uint8 m_nRangeType;
	uint8 m_nTargetType;
	uint32 m_nRange, m_nRange1;

	uint32 m_nFramesRowCount, m_nFramesColumnCount;

	CString m_strOrganAction;
	CString m_strOrganTargetor;
	CReference<CPrefab> m_pOrganActionPrefab;
	CReference<CPrefab> m_pOrganTargetorPrefab;

	CReference<COrganHpBar> m_pHpBar;
};

class COrganEditItem : public CFaceEditItem
{
public:
	COrganEditItem() { nType = eFaceEditType_Organ; }

	CReference<CPrefab> pPrefab;
	virtual bool IsValidGrid( CFace* pFace, const TVector2<int32>& pos ) override;
	virtual void Edit( CCharacter* pCharacter, CFace* pFace, const TVector2<int32>& pos ) override;
};