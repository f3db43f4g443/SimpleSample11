#pragma once
#include "Entities/Enemies.h"
#include "CharacterMove.h"
#include "Block.h"
#include "Entities/Bullets.h"
#include "Entities/Decorator.h"
#include "Interfaces.h"

class CLimbs : public CEnemy, public IOperateable
{
	friend void RegisterGameClasses();
public:
	CLimbs( const SClassCreateContext& context ) : CEnemy( context ), m_onChunkKilled( this, &CLimbs::OnChunkKilled ) { SET_BASEOBJECT_ID( CLimbs ); }

	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;
	virtual void Damage( SDamageContext& context ) override;
	virtual bool IsOwner( CEntity* pEntity ) override;
	virtual void Kill() override;
	virtual int8 IsOperateable( const CVector2& pos ) override;
	virtual void Operate( const CVector2& pos ) override;
	virtual void OnHitPlayer( class CPlayer* pPlayer, const CVector2& normal ) override;
	void SetMask( uint8 nMask );
protected:
	void OnChunkKilled();
	void KillAttackEft();
	virtual void OnTickBeforeHitTest() override;
	virtual void OnTickAfterHitTest() override;

	bool m_bAuto;
	CRectangle m_detectRect;
	int32 m_nAITick;
	CRectangle m_attackRect;
	int32 m_nAttackTime;
	int32 m_nAttackTime1;
	float m_fStretchLenPerFrame;
	float m_fBackLenPerFrame;
	float m_fBackLenPerDamage;
	float m_fMaxLen;
	int8 m_nAttackDir;
	float m_fKillEftDist;
	uint32 m_nMaxSpawnCount;
	CVector2 m_killSpawnVelMin1;
	CVector2 m_killSpawnVelMax1;
	CVector2 m_killSpawnVelMin2;
	CVector2 m_killSpawnVelMax2;
	CReference<CEntity> m_pLimbsEft;
	CReference<CEntity> m_pLimbsAttackEft;
	TResourceRef<CPrefab> m_pKillSpawn;
	TResourceRef<CPrefab> m_pBullet;
	uint32 m_nAmmoCount;
	uint32 m_nBulletCount;
	uint32 m_nFireCD;
	float m_fBulletSpeed;

	float m_fEftLen;
	uint8 m_nState;
	bool m_bIgnoreHit;
	bool m_bKilled;
	CVector2 m_vel1, m_vel2;
	uint32 m_nKillSpawnLeft;
	uint32 m_nDamage;
	int32 m_nAmmoLeft;
	int32 m_nFireCDLeft;
	TClassTrigger<CLimbs> m_onChunkKilled;
};

class CLimbsController : public CEnemy
{
	friend void RegisterGameClasses();
public:
	CLimbsController( const SClassCreateContext& context ) : CEnemy( context ) { SET_BASEOBJECT_ID( CLimbsController ); }
	virtual void OnAddedToStage() override;
	virtual void Kill() override;
private:
	virtual void OnTickAfterHitTest() override;
	void KillEft();
	int8 m_nDir;
	TResourceRef<CPrefab> m_pLimbPrefab;
	CRectangle m_attackRect;
	TResourceRef<CPrefab> m_pExpEft;

	vector<CReference<CLimbs> > m_vecLimbs;
	int32 m_nWidth, m_nHeight;
	int32 m_nKillEftCD, m_nKillEftCount;
	bool m_bKilled;
};

class CLimbs1 : public CDecorator
{
	friend void RegisterGameClasses();
public:
	CLimbs1( const SClassCreateContext& context ) : CDecorator( context ), m_onTick( this, &CLimbs1::OnTick ), m_onChunkKilled( this, &CLimbs1::Kill ) { SET_BASEOBJECT_ID( CLimbs1 ); }
	virtual void Init( const CVector2& size, SChunk* pPreParent ) override;
	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;
	virtual void UpdateRendered( double dTime ) override;
	virtual void Render( CRenderContext2D& context ) override;
	void Kill();
private:
	void OnTick();

	uint32 m_nAITick;
	CRectangle m_attackRect;
	uint32 m_nAttackCD;
	float m_fBulletSpeed;
	TResourceRef<CPrefab> m_pBullet;
	TResourceRef<CPrefab> m_pKillSpawn;

	bool m_bKilled;
	int32 m_nWidth;
	int32 m_nHeight;
	vector<int8> m_vecMask;
	vector<pair<CVector2, uint8> > m_vecKillSpawn;
	TClassTrigger<CLimbs1> m_onTick;
	TClassTrigger<CLimbs1> m_onChunkKilled;
	struct SElem
	{
		SElem()
		{
			m_element2D.worldMat.Identity();
		}
		CElement2D m_element2D;
		int8 nX, nY, nFrame;
	};
	vector<SElem> m_elems;
	CReference<CRenderObject2D> m_pImg;
	float m_fTime;
};

class CLimbsHook : public CEnemy, public IHook
{
	friend void RegisterGameClasses();
public:
	CLimbsHook( const SClassCreateContext& context ) : CEnemy( context ), m_onChunkKilled( this, &CLimbsHook::Kill ) { SET_BASEOBJECT_ID( CLimbsHook ); }

	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;
	virtual void Kill() override;
	virtual void OnDetach() override;
protected:
	void OnEndHook();
	void KillAttackEft();
	virtual void OnTickBeforeHitTest() override;
	virtual void OnTickAfterHitTest() override;

	float m_fDetectDist;
	float m_fAttackDist;
	int32 m_nAITick;
	int32 m_nAttackTime;
	int32 m_nAttackTime1;
	float m_fStretchSpeed;
	float m_fBackSpeed;
	float m_fMaxLen;
	float m_fKillEftDist;
	CRectangle m_rectHook;
	CRectangle m_rectDetach;
	CReference<CEntity> m_pLimbsAttackEft;
	TResourceRef<CPrefab> m_pBullet;

	float m_fEftLen;
	uint8 m_nState;
	CMatrix2D m_lastMatrix;
	CReference<CCharacter> m_pHooked;
	TClassTrigger<CLimbsHook> m_onChunkKilled;
};

class CManChunk1 : public CEnemy
{
	friend void RegisterGameClasses();
public:
	CManChunk1( const SClassCreateContext& context ) : CEnemy( context ), m_moveData( context ), m_onChunkKilled( this, &CManChunk1::Kill ) { SET_BASEOBJECT_ID( CManChunk1 ); }

	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;
	virtual void Damage( SDamageContext& context ) override;
	virtual void Kill() override;
	virtual void Crush() override;
private:
	virtual void OnTickBeforeHitTest() override;
	virtual void OnTickAfterHitTest() override;

	SCharacterPhysicsMovementData m_moveData;
	float m_fAttackDist;
	float m_fStopDist;
	float m_fSpeed;
	float m_fSpeed1;
	float m_fMaxRadius;
	int32 m_nAttackCD;
	int32 m_nSelfDmg;
	float m_fKillY;
	CReference<CEntity> m_pManChunkEft;
	CReference<CEnemyPart> m_pEnemyPart;
	TResourceRef<CPrefab> m_pBullet;
	TResourceRef<CPrefab> m_pBullet1;

	uint8 m_nType;
	uint8 m_nState;
	int32 m_nCDLeft;
	float m_fHit0;
	TClassTrigger<CManChunk1> m_onChunkKilled;
};

class CManChunkEye : public CEnemyTemplate
{
	friend void RegisterGameClasses();
public:
	CManChunkEye( const SClassCreateContext& context ) : CEnemyTemplate( context ) { SET_BASEOBJECT_ID( CManChunkEye ); }
protected:
	virtual void AIFunc() override;

	float m_fTraceSpeed;
	float m_fRange, m_fRange1;
	float m_fDist, m_fDist1;
	int32 m_nCD;
	int32 m_nBullet;
	int32 m_nTraceTime, m_nTraceTime1;
	int32 m_nFireCD;
	float m_fAngle;
	float m_v, m_a;
	CReference<CEntity> m_pEft;
	TResourceRef<CPrefab> m_pBullet;
	TResourceRef<CPrefab> m_pLightning;
	TResourceRef<CPrefab> m_pLightning0;
};

class CManChunkEyeBouns : public CEnemy
{
	friend void RegisterGameClasses();
public:
	CManChunkEyeBouns( const SClassCreateContext& context ) : CEnemy( context ) { SET_BASEOBJECT_ID( CManChunkEyeBouns ); }
	virtual void OnAddedToStage() override;
	virtual void Damage( SDamageContext& context ) override;
protected:
	virtual void OnTickAfterHitTest() override;

	int32 m_nSpawn;
	float m_fSpawnSpeed;
	float m_fTraceSpeed;
	int32 m_nSpawnCount;
	float m_fSpawnAngle;
	CReference<CEntity> m_pEft;
	TResourceRef<CPrefab> m_pSpawn;

	CVector2 m_target;
};

class CArmRotator : public CEnemy
{
	friend void RegisterGameClasses();
public:
	CArmRotator( const SClassCreateContext& context ) : CEnemy( context ), m_onChunkKilled( this, &CArmRotator::Kill ) { SET_BASEOBJECT_ID( CArmRotator ); }
	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;
	virtual void Kill() override;
protected:
	virtual void OnTickBeforeHitTest() override;

	CReference<CEntity> m_pEft;
	CReference<CEntity> m_pEnd;
	float m_fRad;
	float m_fASpeed;
	float m_fWidth;
	float m_fKillEftDist;

	float m_fAngle;
	float m_dAngle;
	TClassTrigger<CArmRotator> m_onChunkKilled;
};

class CManChunkEgg : public CEnemy
{
	friend void RegisterGameClasses();
public:
	CManChunkEgg( const SClassCreateContext& context ) : CEnemy( context ), m_flyData( context ), m_onChunkKilled( this, &CManChunkEgg::Kill ) { SET_BASEOBJECT_ID( CManChunkEgg ); }

	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;
	virtual void Damage( SDamageContext& context ) override;
	virtual void Kill() override;
	bool Hatch();
private:
	void AIFunc();
	class AI : public CAIObject
	{
	protected:
		virtual void AIFunc() override { static_cast<CManChunkEgg*>( GetParentEntity() )->AIFunc(); }
	};
	AI* m_pAI;

	float m_fSpeed;
	float m_fMaxRadius;
	CReference<CEntity> m_pManChunkEft;
	TResourceRef<CPrefab> m_pBullet;
	SCharacterPhysicsFlyData m_flyData;

	TClassTrigger<CManChunkEgg> m_onChunkKilled;
};

class CArmAdvanced : public CEnemyTemplate, public IAttachable
{
	friend void RegisterGameClasses();
public:
	CArmAdvanced( const SClassCreateContext& context ) : CEnemyTemplate( context ), m_onChunkKilled( this, &CArmAdvanced::Kill ) { SET_BASEOBJECT_ID( CArmAdvanced ); }
	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;
	virtual void Damage( SDamageContext& context ) override;
	virtual void Kill() override;
protected:
	virtual void AIFunc() override;
	void Step( CVector2& moveTarget );
	void Spawn();
	void Hatch( int32 n, int8 nType );
	void Attach( CEntity* pEntity, const CVector2& ofs );
	void KillItem();
	virtual void OnSlotDetach( CEntity* pTarget ) override;
	float m_fArmLen;
	float m_fArmLenSpeed;
	float m_fArmWidth;
	float m_fKillEftDist;
	int32 m_nWaitTime;
	CReference<CEntity> m_pBloodConsumer;
	TResourceRef<CPrefab> m_pPrefabArm;
	TResourceRef<CPrefab> m_pPrefabEgg;
	TResourceRef<CPrefab> m_pPrefabComponent;

	struct SItem
	{
		uint8 nType;
		uint8 nState;
		CReference<CEntity> pArm;
		CReference<CEntity> pArmEft;
		CReference<CEntity> pComponent;
		float fArmLen;
		CVector2 targetPos;
		CReference<CEntity> pTarget;
	};
	vector<SItem> m_vecItems;

	uint8 m_nState;
	int32 m_nDamage;
	TClassTrigger<CArmAdvanced> m_onChunkKilled;
};

class CCar : public CEnemy
{
	friend void RegisterGameClasses();
public:
	CCar( const SClassCreateContext& context ) : CEnemy( context ), m_moveData( context ) { SET_BASEOBJECT_ID( CCar ); m_bHasHitFilter = true; }

	void SetExcludeChunk( CChunkObject * pChunkObject, const TRectangle<int32>& rect )
	{
		m_pExcludeChunkObject = pChunkObject;
		m_excludeRect = rect;
	}

	virtual bool CanHit( CEntity* pEntity ) override;
	virtual bool CanHit1( CEntity* pEntity, SRaycastResult& result ) override;
	virtual void OnRemovedFromStage() override;

	virtual void Damage( SDamageContext& context ) override;
	virtual void Kill() override;
protected:
	virtual void OnTickAfterHitTest() override;

	SCharacterVehicleMovementData m_moveData;
	float m_fAcc;
	float m_fHitDamage;
	float m_fHitDmg1Coef;
	uint32 m_nBurnHp;
	uint32 m_nBurnDamage;
	uint32 m_nBurnDamageInterval;
	TResourceRef<CPrefab> m_strBurnEffect;
	TResourceRef<CPrefab> m_strBullet;
	TResourceRef<CPrefab> m_strExplosion;
	TResourceRef<CPrefab> m_strMan;
	CReference<CRenderObject2D> m_pDoors[4];
	CVector2 m_spawnManOfs[4];
	float m_spawnManRadius;
	uint32 m_nSpawnManTime;
	float m_fSpawnManSpeed;
	uint8 m_nExpType;

	CReference<CChunkObject> m_pExcludeChunkObject;
	TRectangle<int32> m_excludeRect;
	CReference<CEntity> m_pBurnEffect;
	uint32 m_nBurnDamageCD;
	uint32 m_nSpawnManTimeLeft;
	bool m_bDoorOpen;
	bool m_bSpawned;
	bool m_bHitExcludeChunk;
};

class CThrowBox : public CThrowObj
{
	friend void RegisterGameClasses();
public:
	CThrowBox( const SClassCreateContext& context ) : CThrowObj( context ) { SET_BASEOBJECT_ID( CThrowBox ); }

	virtual void Kill() override;
private:
	TResourceRef<CPrefab> m_pBullet;
};