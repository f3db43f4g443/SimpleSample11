#pragma once
#include "TutorialGame.h"
#include "AIObject.h"

class CTutorialGame0Enemy : public CTutorialGameElement
{
	friend void RegisterGameClasses();
public:
	CTutorialGame0Enemy( const SClassCreateContext& context ) : CTutorialGameElement( context ), m_strHREffect( context ) {}

	virtual void OnAddedToStage() override;
	void OnHitPlayer();

	class AI : public CAIObject
	{
	protected:
		virtual void AIFunc() override { static_cast<CTutorialGame0Enemy*>( GetParentEntity() )->AIFunc(); }
	};
protected:
	void AIFunc();
	virtual void OnUpdate( class CTutorialGame* pGame, class CPlayer* pPlayer, float fTime ) override;
private:
	CString m_strHREffect;
	CReference<CPrefab> m_pHREffectPrefab;
	CReference<AI> m_pAI;
};

class CTutorialGame0 : public CTutorialGame
{
	friend void RegisterGameClasses();
public:
	CTutorialGame0( const SClassCreateContext& context );
	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;
	CDynamicTexture& GetEffectTexture() { return m_effectTexture; }

	virtual void Render( CRenderContext2D& context ) override
	{
		if( context.eRenderPass == eRenderPass_Color )
			m_effectTexture.Render( context );
		CTutorialGame::Render( context );
	}

	virtual void Damage( uint32 nHp, uint32 nMp, uint32 nSp ) override;
	
	CReference<CPrefab> pBackgroundPrefab;
	CReference<CPrefab> pBackgroundEffectPrefab;
	CReference<CPrefab> pBackgroundEffect1Prefab;
	CReference<CPrefab> pEnemyPrefab;

	CReference<CPrefab> pBulletSmallPrefab;
	CReference<CPrefab> pBulletBigPrefab;
	CReference<CPrefab> pTargetEffectPrefab;
protected:
	virtual void StartGame() override;
	virtual void OnPlayerUpdated() override;
private:
	CString m_strBackground;
	CString m_strBackgroundEffect;
	CString m_strBackgroundEffect1;
	CString m_strEnemyPrefab;
	CString m_strHpBar;
	CString m_strBulletSmall;
	CString m_strBulletBig;
	CString m_strTargetEffect;
	CReference<CRenderObject2D> m_pHREffect;
	CReference<CTutorialGameHpBar> m_pHpBar;
	CDynamicTexture m_effectTexture;
	uint32 m_nCurHp;
	uint32 m_nMaxHp;

	CTutorialGamePlayerActionAttack m_actionAttack;
};