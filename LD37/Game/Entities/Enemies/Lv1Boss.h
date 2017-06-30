#pragma once
#include "Entities/Enemies.h"
#include "Block.h"
#include "LevelScrollObj.h"
#include "Render/DrawableGroup.h"
#include "CharacterMove.h"

class CLv1Boss : public CLevelScrollObj
{
	friend class CLv1BossWorm1;
	friend class CLv1BossWorm2;
	friend void RegisterGameClasses();
public:
	CLv1Boss( const SClassCreateContext& context ) : CLevelScrollObj( context ), m_bActive( false ), m_h1( -1 ), m_nLinkCount( 0 )
		, m_strBullet( context ), m_strBullet1( context ), m_strBullet2( context ), m_strBullet3( context )
		, m_strLaser( context ), m_strBulletEye( context ), m_strBulletShockwave( context ), m_strTentacle( context ), m_strTentacleHole( context )
		, m_strExpKnockbackName( context ), m_strExpKnockback1( context ), m_strTransparentChunkName( context ), m_strWorm1( context ), m_strWorm2( context )
		, m_strTentacleName1( context ), m_strExplosive0( context ), m_strExplosive1( context ), m_strExplosive2( context ), m_strExplosive3( context ) { SET_BASEOBJECT_ID( CLv1Boss ); }
	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;
	virtual void Update( uint32 nCur ) override;
	virtual void PostBuild( struct SLevelBuildContext& context ) override;
protected:
	void Active();

	virtual void OnTick() override;
private:
	void CreateTentacle( uint8 i, CChunkObject* pChunkObject );
	void SpawnWorm1( CEntity* pTentacle );
	CEntity* FindWorm1ReturnPoint();

	void UpdateEyePos( CEntity* pEye, CRenderObject2D* pEyeLink, const CVector2& target, float fSpeed );
	void AIFuncEye( uint8 nEye );
	void AIFuncEye1( uint8 nEye );
	void AIFuncEye2( uint8 nEye, CChunkObject* pChunkObject );
	void AIFuncEye3( uint8 nEye );
	class AIEye : public CAIObject
	{
	public:
		AIEye( uint8 nEye ) : m_nEye( nEye ) {}
	protected:
		virtual void AIFunc() override { static_cast<CLv1Boss*>( GetParentEntity() )->AIFuncEye( m_nEye ); }
		uint8 m_nEye;
	};
	AIEye* m_pAIEye[2];

	void AIFuncNose();
	void AIFuncNose1();
	void AIFuncNose2( CChunkObject* pChunkObject );
	void AIFuncNose3();
	class AINose : public CAIObject
	{
	protected:
		virtual void AIFunc() override { static_cast<CLv1Boss*>( GetParentEntity() )->AIFuncNose(); }
	};
	AINose* m_pAINose;

	void AIFuncMouth();
	void AIFuncMouth1();
	void AIFuncTongue( CChunkObject* pChunkObject );
	void AIFuncMouth3();
	class AIMouth : public CAIObject
	{
	protected:
		virtual void AIFunc() override { static_cast<CLv1Boss*>( GetParentEntity() )->AIFuncMouth(); }
	};
	AIMouth* m_pAIMouth;

	void AIFuncMain();
	void AIFuncMain1();
	void AIFuncMainFinal();
	class AIMain : public CAIObject
	{
	protected:
		virtual void AIFunc() override { static_cast<CLv1Boss*>( GetParentEntity() )->AIFuncMain(); }
	};
	AIMain* m_pAIMain;
	bool m_bActive;

	void BeginFinalPhase();

	bool CheckBlocked( const TRectangle<int32>& rect );

	TResourceRef<CPrefab> m_strBullet;
	TResourceRef<CPrefab> m_strBullet1;
	TResourceRef<CPrefab> m_strBullet2;
	TResourceRef<CPrefab> m_strBullet3;
	TResourceRef<CPrefab> m_strLaser;
	TResourceRef<CPrefab> m_strBulletEye;
	TResourceRef<CPrefab> m_strBulletShockwave;
	TResourceRef<CDrawableGroup> m_strTentacle;
	TResourceRef<CDrawableGroup> m_strTentacleHole;
	TResourceRef<CPrefab> m_strWorm1;
	TResourceRef<CPrefab> m_strWorm2;
	TResourceRef<CPrefab> m_strExpKnockbackName;
	TResourceRef<CPrefab> m_strExpKnockback1;
	TResourceRef<CPrefab> m_strExplosive0;
	TResourceRef<CPrefab> m_strExplosive1;
	TResourceRef<CPrefab> m_strExplosive2;
	TResourceRef<CPrefab> m_strExplosive3;
	CString m_strTransparentChunkName;
	CString m_strTentacleName1;

	CReference<CEntity> m_pBoss;
	CReference<CEntity> m_pFaceEye[2];
	CReference<CEntity> m_pFaceNose;
	CReference<CEntity> m_pFaceMouth;
	CReference<CEntity> m_pEyeHole[2];
	CReference<CEnemy> m_pEye[2];
	CReference<CRenderObject2D> m_pEyeLink[2];
	CReference<CEntity> m_pNose;
	CReference<CEntity> m_pTongueHole;
	CReference<CEnemy> m_pTongue;
	vector<CReference<CEntity> > m_vecTongueSegs;
	CReference<CDrawableGroup> m_pDrawableGroup;
	vector<CReference<CEntity> > m_vecTentacles;
	vector<CReference<CEntity> > m_vecWorms1;
	int32 m_nLinkCount;
	int32 m_h1;
};

class CLv1BossWorm1 : public CEnemyTemplate
{
	friend void RegisterGameClasses();
public:
	CLv1BossWorm1( const SClassCreateContext& context ) : CEnemyTemplate( context ), m_bKilled( false ), m_bReturn( false ), m_pAIFire( NULL ) { SET_BASEOBJECT_ID( CLv1BossWorm1 ); }

	void Set( CLv1Boss* pOwner, CEntity* pSpawner );
	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;
	virtual void Kill() override;
	void Return();
protected:
	virtual void AIFunc() override;

	void AIFuncFire();
	class AIFire : public CAIObject
	{
	protected:
		virtual void AIFunc() override { static_cast<CLv1BossWorm1*>( GetParentEntity() )->AIFuncFire(); }
	};
	AIFire* m_pAIFire;

	vector<CReference<CEntity> > m_parts;
	CReference<CLv1Boss> m_pOwner;
	CReference<CEntity> m_pSpawner;
	bool m_bKilled;
	bool m_bReturn;
};

class CLv1BossWorm2Seg : public CEnemyPart
{
	friend void RegisterGameClasses();
public:
	CLv1BossWorm2Seg( const SClassCreateContext& context ) : CEnemyPart( context ) { SET_BASEOBJECT_ID( CLv1BossWorm2Seg ); }

	CRenderObject2D* GetLayer( int8 i ) { return m_pLayers[i]; }
private:
	CReference<CRenderObject2D> m_pLayers[3];
};

class CLv1BossWorm2 : public CEnemyTemplate
{
	friend void RegisterGameClasses();
	typedef int32 SKilled;
	typedef void* SKilled1;
public:
	CLv1BossWorm2( const SClassCreateContext& context ) : CEnemyTemplate( context ), m_bSpawned( false ), m_bKilled( false ), m_strSeg( context )
		, m_strBullet1( context ), m_strBullet2( context ), m_strBullet3( context ) { SET_BASEOBJECT_ID( CLv1BossWorm2 ); }

	bool IsSpawned() { return m_bSpawned; }
	bool IsKilled() { return m_bKilled; }
	void Set( CLv1Boss* pOwner, uint32 nSegs, const CVector2& src, const CVector2& target, int8 nMoveType ) { m_pOwner = pOwner; m_vecSegs.resize( nSegs ); m_src = src; m_target = target; m_nMoveType = nMoveType; }
	virtual void OnAddedToStage() override;
	virtual void Kill() override;

	void Fire1();
	void Fire2();
protected:
	virtual void AIFunc() override;
	void Move1();
	void Move2();
	void Move3();
	void Destroy();

	bool m_bSpawned;
	bool m_bKilled;
	TResourceRef<CPrefab> m_strSeg;
	TResourceRef<CPrefab> m_strBullet1;
	TResourceRef<CPrefab> m_strBullet2;
	TResourceRef<CPrefab> m_strBullet3;

	vector<CCharacter*> m_vecSegs;
	CReference<CLv1Boss> m_pOwner;
	CVector2 m_src;
	CVector2 m_target;
	int8 m_nMoveType;
	CReference<CEntity> m_pLayers[3];
};

class CLv1BossBullet1 : public CEnemyTemplate
{
	friend void RegisterGameClasses();
public:
	CLv1BossBullet1( const SClassCreateContext& context ) : CEnemyTemplate( context ), m_flyData( context ), m_strBullet( context ) { SET_BASEOBJECT_ID( CLv1BossBullet1 ); }

	virtual void Kill() override;
protected:
	virtual void AIFunc() override;
	virtual void OnTickAfterHitTest() override;

	TResourceRef<CPrefab> m_strBullet;

	SCharacterPhysicsFlyData m_flyData;
	CVector2 m_moveTarget;
};