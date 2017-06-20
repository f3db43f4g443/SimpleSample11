#pragma once
#include "Entities/Enemies.h"
#include "Block.h"
#include "LevelScrollObj.h"
#include "Render/DrawableGroup.h"

class CLv1Boss : public CLevelScrollObj
{
	friend void RegisterGameClasses();
public:
	CLv1Boss( const SClassCreateContext& context ) : CLevelScrollObj( context ), m_h1( -1 ), m_nLinkCount( 0 ), m_strTentacle( context ), m_strTentacleHole( context ) { SET_BASEOBJECT_ID( CLv1Boss ); }
	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;
	virtual void Update( uint32 nCur ) override;
	virtual void PostBuild( struct SLevelBuildContext& context ) override;
protected:
	void Active();

	virtual void OnTick() override;
private:
	void CreateTentacle( uint8 i, CChunkObject* pChunkObject );

	void AIFuncEye( uint8 nEye, CChunkObject* pChunkObject );
	class AIEye : public CAIObject
	{
	public:
		AIEye( uint8 nEye, CChunkObject* pChunkObject ) : m_nEye( nEye ), m_pChunkObject( pChunkObject ) {}
	protected:
		virtual void AIFunc() override { static_cast<CLv1Boss*>( GetParentEntity() )->AIFuncEye( m_nEye, m_pChunkObject ); }
		uint8 m_nEye;
		CChunkObject* m_pChunkObject;
	};
	AIEye* m_pAIEye[2];

	void AIFuncNose( CChunkObject* pChunkObject );
	class AINose : public CAIObject
	{
	public:
		AINose( CChunkObject* pChunkObject ) : m_pChunkObject( pChunkObject ) {}
	protected:
		virtual void AIFunc() override { static_cast<CLv1Boss*>( GetParentEntity() )->AIFuncNose( m_pChunkObject ); }
		CChunkObject* m_pChunkObject;
	};
	AINose* m_pAINose;

	void AIFuncTongue( CChunkObject* pChunkObject );
	class AITongue : public CAIObject
	{
	public:
		AITongue( CChunkObject* pChunkObject ) : m_pChunkObject( pChunkObject ) {}
	protected:
		virtual void AIFunc() override { static_cast<CLv1Boss*>( GetParentEntity() )->AIFuncTongue( m_pChunkObject ); }
		CChunkObject* m_pChunkObject;
	};
	AITongue* m_pAITongue;

	CString m_strTentacle;
	CString m_strTentacleHole;
	CReference<CDrawableGroup> m_pDrawableTentacle;
	CReference<CDrawableGroup> m_pDrawableTentacleHole;

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
	int32 m_nLinkCount;
	int32 m_h1;
};
