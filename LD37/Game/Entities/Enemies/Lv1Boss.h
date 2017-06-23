#pragma once
#include "Entities/Enemies.h"
#include "Block.h"
#include "LevelScrollObj.h"
#include "Render/DrawableGroup.h"

class CLv1Boss : public CLevelScrollObj
{
	friend class CLv1BossWorm1;
	friend void RegisterGameClasses();
public:
	CLv1Boss( const SClassCreateContext& context ) : CLevelScrollObj( context ), m_h1( -1 ), m_nLinkCount( 0 )
		, m_strBullet( context ), m_strBullet1( context ), m_strBullet2( context ), m_strBullet3( context ), m_strTentacle( context ), m_strTentacleHole( context )
		, m_strExpKnockbackName( context ), m_strTransparentChunkName( context ), m_strWorm1( context ), m_strTentacleName1( context ) { SET_BASEOBJECT_ID( CLv1Boss ); }
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
	class AIMain : public CAIObject
	{
	protected:
		virtual void AIFunc() override { static_cast<CLv1Boss*>( GetParentEntity() )->AIFuncMain(); }
	};
	AIMain* m_pAIMain;

	bool CheckBlocked( const TRectangle<int32>& rect );

	CString m_strBullet;
	CString m_strBullet1;
	CString m_strBullet2;
	CString m_strBullet3;
	CString m_strTentacle;
	CString m_strTentacleHole;
	CString m_strWorm1;
	CString m_strTransparentChunkName;
	CReference<CPrefab> m_pPrefabBullet;
	CReference<CPrefab> m_pPrefabBullet1;
	CReference<CPrefab> m_pPrefabBullet2;
	CReference<CPrefab> m_pPrefabBullet3;
	CReference<CDrawableGroup> m_pDrawableTentacle;
	CReference<CDrawableGroup> m_pDrawableTentacleHole;
	CReference<CPrefab> m_pWorm1;
	CReference<CPrefab> m_pPrefabExpKnockback;

	CString m_strExpKnockbackName;
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
