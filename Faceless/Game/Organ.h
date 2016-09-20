#pragma once
#include "Entity.h"
#include "FaceEditItem.h"
#include "Common/StringUtil.h"

class CTurnBasedContext;
class CCharacter;
struct SOrganActionContext
{
	CCharacter* pCharacter;
	TVector2<int> target;
	uint8 nCharge;

	vector<CCharacter*> targetCharacters;
};

enum
{
	eRangeType_Normal,

};

enum
{
	eTargetType_None,
	eTergetType_Pos,
	eTargetType_Cbaracter
};

class COrgan : public CEntity
{
	friend class CFace;
	friend class COrganEditItem;
	friend void RegisterGameClasses();
public:
	COrgan( const SClassCreateContext& context ) : CEntity( context ), m_pFace( NULL ), m_pos( 0, 0 ), m_strOrganAction( context ) { SET_BASEOBJECT_ID( COrgan ); }
	uint32 GetWidth() { return m_nWidth; }
	uint32 GetHeight() { return m_nHeight; }
	TVector2<int32> GetGridPos() { return m_pos; }

	void GetRange( vector<TVector2<int32> >& result );
	bool IsInRange( const TVector2<int32>& pos );
	bool CanAction( CTurnBasedContext* pContext, SOrganActionContext& actionContext );

	void Action( CTurnBasedContext* pContext, SOrganActionContext& actionContext );
private:
	CFace* m_pFace;
	TVector2<int32> m_pos;

	uint32 m_nWidth, m_nHeight;

	uint32 m_nCost;
	uint8 m_nRangeType;
	uint8 m_nTargetType;
	uint32 m_nRange, m_nRange1;

	CString m_strOrganAction;
	CReference<CPrefab> m_pOrganActionPrefab;
};

class COrganAction : public CEntity
{
	friend void RegisterGameClasses();
public:
	COrganAction( const SClassCreateContext& context ) : CEntity( context ) {}
	virtual void Action( CTurnBasedContext* pContext, SOrganActionContext& actionContext ) {}

};

class COrganTargetor : public CEntity
{
	friend void RegisterGameClasses();
public:
	COrganTargetor( const SClassCreateContext& context ) : CEntity( context ) {}
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

class COrganEditItem : public CFaceEditItem
{
public:
	COrganEditItem() { nType = eFaceEditType_Organ; }

	CReference<CPrefab> pPrefab;
	virtual bool IsValidGrid( CFace* pFace, const TVector2<int32>& pos ) override;
	virtual void Edit( CCharacter* pCharacter, CFace* pFace, const TVector2<int32>& pos ) override;
};