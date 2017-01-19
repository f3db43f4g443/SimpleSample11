#pragma once
#include "Entity.h"
#include "Common/StringUtil.h"

class CBlockItem : public CEntity
{
	friend void RegisterGameClasses();
public:
	CBlockItem( const SClassCreateContext& context ) : CEntity( context ) { SET_BASEOBJECT_ID( CBlockItem );}

};

enum EBlockItemTriggerType
{
	eBlockItemTriggerType_Step,
	eBlockItemTriggerType_HitDeltaPos,
	eBlockItemTriggerType_HitVelocity,
};

class CCharacter;
class CBlockItemTrigger : public CBlockItem
{
	friend void RegisterGameClasses();
public:
	CBlockItemTrigger( const SClassCreateContext& context ) : CBlockItem( context ), m_nTriggerCDLeft( 0 ), m_onTick( this, &CBlockItemTrigger::OnTick ) { SET_BASEOBJECT_ID( CBlockItemTrigger ); }

	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;

	bool Trigger( CCharacter* pCharacter );
protected:
	virtual void OnTrigged( CCharacter* pCharacter, const CVector2& dir ) {}
	void OnTick();
	void CheckHit( CCharacter* pCharacter );

	uint32 m_nTriggerCD;
	uint32 m_nTriggerCDLeft;

	EBlockItemTriggerType m_eTriggerType;

	CReference<CEntity> m_pTriggerArea;
	vector<CReference<CCharacter> > m_vecHitCharacters;

	TClassTrigger<CBlockItemTrigger> m_onTick;
};

class CBlockItemTrigger1 : public CBlockItemTrigger
{
	friend void RegisterGameClasses();
public:
	CBlockItemTrigger1( const SClassCreateContext& context ) : CBlockItemTrigger( context ), m_strBullet( context ), m_strBullet1( context ) { SET_BASEOBJECT_ID( CBlockItemTrigger1 ); }

	virtual void OnAddedToStage() override;
protected:
	virtual void OnTrigged( CCharacter* pCharacter, const CVector2& dir ) override;

	uint32 m_nAmmoCount;
	uint32 m_nFireRate;
	uint32 m_nBulletCount;
	float m_fBulletSpeed;
	float m_fBulletAngle;

	CString m_strBullet;
	CString m_strBullet1;
	CReference<CPrefab> m_pBulletPrefab;
	CReference<CPrefab> m_pBulletPrefab1;
};