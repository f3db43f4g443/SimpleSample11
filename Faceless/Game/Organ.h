#pragma once
#include "Entity.h"
#include "GameUtil.h"
#include "FaceEditItem.h"
#include "Common/StringUtil.h"

#include "Entities/OrganHpBar.h"

enum ETargetType
{
	eTargetType_None,
	eTergetType_Pos,
	eTargetType_Character
};

class CTurnBasedContext;
class CCharacter;
struct SOrganActionContext
{
	SOrganActionContext() : pCharacter( NULL ), pOrgan( NULL ), pCurTarget( NULL ), bSucceed( false ) {}
	CCharacter* pCharacter;
	class COrgan* pOrgan;
	class COrganAction* pOrganAction;
	class COrganTargetor* pOrganTargetor;
	TVector2<int> target;
	uint8 nCharge;

	bool bSucceed;

	vector<CCharacter*> targetCharacters;
	CCharacter* pCurTarget;

	void AddObject( const char* szName, CRenderObject2D* pObject )
	{
		RemoveObject( szName );
		mapTempObjects[szName] = pObject;
	}
	CRenderObject2D* FindObject( const char* szName ) const
	{
		auto itr = mapTempObjects.find( szName );
		if( itr == mapTempObjects.end() )
			return NULL;
		return itr->second;
	}
	void RemoveObject( const char* szName )
	{
		auto itr = mapTempObjects.find( szName );
		if( itr == mapTempObjects.end() )
			return;
		itr->second->RemoveThis();
		mapTempObjects.erase( itr );
	}

	map<string, CReference<CRenderObject2D> > mapTempObjects;
};

class COrganAction : public CEntity
{
	friend void RegisterGameClasses();
public:
	COrganAction( const SClassCreateContext& context ) : CEntity( context ) { SET_BASEOBJECT_ID( COrganAction ); }
	virtual void Action( CTurnBasedContext* pContext ) {}
	
	virtual void OnBeginFaceSelectTarget( SOrganActionContext& actionContext ) {}
	virtual void OnFaceSelectTargetMove( SOrganActionContext& actionContext, TVector2<int32> grid ) {}
	virtual void OnEndFaceSelectTarget( SOrganActionContext& actionContext ) {}
};

class COrganTargetor : public CEntity
{
	friend void RegisterGameClasses();
public:
	COrganTargetor( const SClassCreateContext& context ) : CEntity( context ) { SET_BASEOBJECT_ID( COrganTargetor ); }
	virtual void FindTargets( CTurnBasedContext* pContext ) {}

	typedef function<bool( CCharacter* pChar, CTurnBasedContext* pContext )> FuncOnFindTarget;
	void SetFindTargetFunc( FuncOnFindTarget func ) { m_onFindTarget = func; }

	virtual void OnBeginSelectTarget( SOrganActionContext& actionContext, TVector2<int32> grid ) { m_selectGrid = grid; ShowSelectTarget( actionContext, true ); }
	virtual void OnSelectTargetMove( SOrganActionContext& actionContext, TVector2<int32> grid ) { OnEndSelectTarget( actionContext ); OnBeginSelectTarget( actionContext, grid ); }
	virtual void OnEndSelectTarget( SOrganActionContext& actionContext ) { ShowSelectTarget( actionContext, false ); }

	virtual void ShowSelectTarget( SOrganActionContext& actionContext, bool bShow ) {}
protected:
	void FindTarget( CCharacter* pChar, CTurnBasedContext* pContext );
	
	TVector2<int32> m_selectGrid;
private:
	FuncOnFindTarget m_onFindTarget;
};

class COrgan : public CEntity
{
	friend class CFace;
	friend class COrganEditItem;
	friend void RegisterGameClasses();
public:
	COrgan( const SClassCreateContext& context ) : CEntity( context ), m_pEditItem( NULL ), m_nHp( m_nMaxHp ), m_pFace( NULL ), m_pos( 0, 0 ), m_strOrganAction( context ), m_strOrganTargetor( context ), m_nVisitFlag( 0 )
	{ SET_BASEOBJECT_ID( COrgan ); }

	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;

	CFace* GetFace() const { return m_pFace; }
	uint32 GetWidth() const { return m_nWidth; }
	uint32 GetHeight() const { return m_nHeight; }
	uint32 GetInnerX() const { return m_nInnerX; }
	uint32 GetInnerY() const { return m_nInnerY; }
	uint32 GetInnerWidth() const { return m_nInnerWidth; }
	uint32 GetInnerHeight() const { return m_nInnerHeight; }
	TVector2<int32> GetGridPos() const { return m_pos; }
	COrganEditItem* GetEditItem() { return m_pEditItem; }

	uint32 GetCurHp() const { return m_nHp; }
	uint32 GetMaxHp() const { return m_nMaxHp; }

	void GetRange( vector<TVector2<int32> >& result );
	uint32 GetCost() { return m_nCost; }
	bool IsInRange( const TVector2<int32>& pos );
	bool CanAction( SOrganActionContext& actionContext );
	bool CheckActionTarget( SOrganActionContext& actionContext );

	void Action( CTurnBasedContext* pContext, SOrganActionContext& actionContext );
	void ActionSelectTarget( CTurnBasedContext* pContext );
	void ActionSelectTarget( CTurnBasedContext* pContext, COrganTargetor::FuncOnFindTarget func );

	void SetHp( uint32 nHp );
	void Damage( uint32 nDmg );

	void ShowHpBar( bool bShown );

	CPrefab* GetActionPrefab() { return m_pOrganActionPrefab; }
	CPrefab* GetTargetorPrefab() { return m_pOrganTargetorPrefab; }

	void OnBeginSelectTarget( SOrganActionContext& actionContext, TVector2<int32> grid );
	void OnSelectTargetMove( SOrganActionContext& actionContext, TVector2<int32> grid );
	void OnEndSelectTarget( SOrganActionContext& actionContext );

	DECLARE_EVENT_TRIGGER( OnHpChanged )

	uint8 m_nVisitFlag;
private:
	CFace* m_pFace;
	TVector2<int32> m_pos;
	COrganEditItem* m_pEditItem;

	uint32 m_nWidth, m_nHeight;
	uint32 m_nInnerX, m_nInnerY, m_nInnerWidth, m_nInnerHeight;
	uint32 m_nHp, m_nMaxHp;

	uint32 m_nCost;
	ERangeType m_nRangeType;
	ETargetType m_nTargetType;
	uint32 m_nRange, m_nRange1;
	bool m_bRangeExcludeSelf;

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
	uint32 nInnerX, nInnerY, nInnerWidth, nInnerHeight;
	virtual bool IsValidGrid( CFace* pFace, const TRectangle<int32>& editRect, const TVector2<int32>& pos ) override;
	virtual void Edit( CCharacter* pCharacter, CFace* pFace, const TVector2<int32>& pos ) override;
};

class COrganCfg
{
public:
	map<string, COrganEditItem> mapOrganEditItems;
	void Load();
	void Unload();

	COrganEditItem* GetOrganEditItem( const char* szName )
	{
		auto itr = mapOrganEditItems.find( szName );
		if( itr == mapOrganEditItems.end() )
			return NULL;
		return &itr->second;
	}

	DECLARE_GLOBAL_INST_REFERENCE( COrganCfg );
};