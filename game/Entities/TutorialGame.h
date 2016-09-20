#pragma once
#include "Entity.h"
#include "Common/StringUtil.h"
#include "Render/Canvas.h"
#include "PlayerActions/PlayerActionAttack.h"
#include "CommonBullet.h"

class CTutorialGameElement : public CEntity
{
	friend void RegisterGameClasses();
public:
	CTutorialGameElement( const SClassCreateContext& context ) : CEntity( context ), m_pos( 0, 0, 0 ), m_nLevel( 0 ) {}

	const CVector3& GetPos() { return m_pos; }
	void SetPos( const CVector3& pos ) { m_pos = pos; SetPosition( CVector2( pos.x, pos.y ) ); }
	int32 GetLevel() { return m_nLevel; }
	void SetLevel( int32 nLevel ) { m_nLevel = nLevel; }
	void Update( class CTutorialGame* pGame, class CPlayer* pPlayer, float fTime );
	void UpdateObj( CTutorialGame* pGame );
protected:
	virtual void OnUpdate( class CTutorialGame* pGame, class CPlayer* pPlayer, float fTime ) {}

	CTutorialGame* m_pGame;
	CVector3 m_pos;
	int32 m_nLevel;
	CReference<CRenderObject2D> m_pObj;
};

class CTutorialGame : public CEntity
{
	friend void RegisterGameClasses();
	friend class CTutorialGameElement;
public:
	CTutorialGame( const SClassCreateContext& context );
	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;

	virtual void AddGameElement( CTutorialGameElement* pElement ) { pElement->SetParentEntity( m_pElementRoot ); pElement->UpdateObj( this ); }
	
	virtual void Render( CRenderContext2D& context ) override
	{
		if( context.eRenderPass == eRenderPass_Color )
		{
			m_dynamicTexture.Render( context );
		}
	}
	
	virtual void Damage( uint32 nHp, uint32 nMp, uint32 nSp ) {}

	const CVector2& GetCamCenter() { return m_camCenter; }
	float GetCamZNear() { return m_fCamZNear; }
	float GetCamDist() { return m_fCamDist; }
	CEntity* GetElementRoot() { return m_pElementRoot; }
	CEntity* GetGUIRoot() { return m_pGUIRoot; }
	CEntity* GetCrosshair() { return m_pCrosshair; }

	static CTutorialGame* GetCurGame() { return CurGame(); }
protected:
	virtual void StartGame() {}
	virtual void OnPlayerUpdated();

	CVector2 m_camCenter;
	float m_fCamZNear;
	float m_fCamDist;
	CDynamicTexture m_dynamicTexture;

	CReference<CEntity> m_pElementRoot;
	CReference<CEntity> m_pGUIRoot;

	CString m_strCrosshair;
	CReference<CEntity> m_pCrosshair;

	TClassTrigger<CTutorialGame> m_onStartGame;
	TClassTrigger<CTutorialGame> m_onPlayerUpdated;

	static CTutorialGame*& CurGame()
	{
		static CTutorialGame* g_pGame = NULL;
		return g_pGame;
	}
};

class CTutorialGameBullet : public CCommonBullet
{
	friend void RegisterGameClasses();
public:
	CTutorialGameBullet( const SClassCreateContext& context ) : CCommonBullet( context ) {}
protected:
	virtual void OnHitPlayer( CPlayer* pPlayer ) override;
};

class CTutorialGameHpBar : public CEntity
{
	friend void RegisterGameClasses();
public:
	CTutorialGameHpBar( const SClassCreateContext& context );
	
	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;
	void SetHp( uint32 nCur, uint32 nMax );
private:
	float m_fHeightPerHp;
	CString m_strElem;
	CString m_strElem1;
	CString m_strEffect;
	CReference<CPrefab> m_pElem;
	CReference<CPrefab> m_pElem1;
	CReference<CPrefab> m_pEffect;

	uint32 m_nCurHp;
	uint32 m_nMaxHp;
	vector<CRenderObject2D*> m_vecImages;
};

class CTutorialGameFall : public CTutorialGame
{
	friend void RegisterGameClasses();
public:
	CTutorialGameFall( const SClassCreateContext& context );
	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;

	void AddGameElement( CTutorialGameElement* pElement ) override;

	CReference<CPrefab> pBackgroundPrefab;
	CReference<CPrefab> pCoinPrefab;
	CReference<CPrefab> pChestPrefab;
	CReference<CPrefab> pParticle0;
protected:
	virtual void StartGame() override;
	virtual void OnPlayerUpdated() override;
private:
	CString m_strBackground;
	CString m_strCoin;
	CString m_strChest;
	CString m_strParticle0;

	CReference<CTutorialGameElement> m_pLastAddedElement;
	float m_fBackgroundGenTime;
	float m_fGenTime;
};

class CTutorialGameFlyingObject : public CTutorialGameElement
{
	friend void RegisterGameClasses();
public:
	CTutorialGameFlyingObject( const SClassCreateContext& context ) : CTutorialGameElement( context ) {}
	float GetSpeed() { return m_fFlyingSpeed; }
protected:
	virtual void OnUpdate( class CTutorialGame* pGame, class CPlayer* pPlayer, float fTime ) override;

	float m_fFlyingSpeed;
	float m_fLife;
};

class CTutorialGameHitObject : public CTutorialGameFlyingObject
{
	friend void RegisterGameClasses();
public:
	CTutorialGameHitObject( const SClassCreateContext& context ) : CTutorialGameFlyingObject( context ) {}
protected:
	virtual void OnUpdate( class CTutorialGame* pGame, class CPlayer* pPlayer, float fTime ) override;
};

class CTutorialGamePlayerActionAttack : public CPlayerActionAttack
{
public:
	CTutorialGamePlayerActionAttack( CTutorialGame* pGame, const char* szName, uint32 nDmg, float fHitAreaRadius, float fMoveSpeed, float fMoveInnerRadius, float fMoveOuterRadius, float fCD  )
		: CPlayerActionAttack( szName, nDmg, fHitAreaRadius, fMoveSpeed, fMoveInnerRadius, fMoveOuterRadius, fCD ), m_pGame( pGame ) {}
	virtual void OnEnterHR( CPlayer* pPlayer ) override;
private:
	CTutorialGame* m_pGame;
};