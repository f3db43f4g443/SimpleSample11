#pragma once
#include <string>
#include "Common/Math3D.h"
using namespace std;

enum EPlayerActionType
{
	ePlayerActionType_Common,
	ePlayerActionType_Channel,
};

class CPlayer;
class CPlayerAction
{
public:
	CPlayerAction( const char* szName, EPlayerActionType eType );
	virtual ~CPlayerAction() {}

	const char* GetName() { return m_strName.c_str(); }
	bool IsDoing() { return m_nState > 0; }

	virtual bool Do( CPlayer* pPlayer ) = 0;
	virtual bool Stop( CPlayer* pPlayer ) = 0;
	virtual void Update( CPlayer* pPlayer, const CVector2& moveAxis, const CVector2& pushForce, float fDeltaTime ) {}

	virtual void OnEnterHR( CPlayer* pPlayer ) {}
	virtual void OnLeaveHR( CPlayer* pPlayer ) {}
protected:
	string m_strName;

	EPlayerActionType m_eType;
	uint8 m_nState;
};

class CPlayerActionCommon : public CPlayerAction
{
public:
	CPlayerActionCommon( const char* szName )
		: CPlayerAction( szName, ePlayerActionType_Common ) {}

	virtual bool Do( CPlayer* pPlayer ) override;
	virtual bool Stop( CPlayer* pPlayer ) override;
protected:
	virtual bool OnDo( CPlayer* pPlayer ) { return false; }
};