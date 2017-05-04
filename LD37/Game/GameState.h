#pragma once
#include "UICommon/UIManager.h"
#include "World.h"

class IGameState
{
public:
	virtual void EnterState() {}
	virtual void ExitState() {}
	virtual void UpdateInput() {}
	virtual void UpdateFrame() {}
	virtual void HandleResize( const CVector2& size ) {}
	virtual void HandleMouseDown( const CVector2& pos ) {}
	virtual void HandleMouseUp( const CVector2& pos ) {}
	virtual void HandleMouseMove( const CVector2& mousePos ) {}
	virtual void HandleChar( uint32 nChar ) {}
	virtual void BeforeRender() {}
};

class CUIMgrGameState : public IGameState
{
public:
	CUIMgrGameState();

	virtual void EnterState() override;
	virtual void ExitState() override;
	virtual void HandleResize( const CVector2& size ) override;
	virtual void HandleMouseDown( const CVector2& pos ) override;
	virtual void HandleMouseUp( const CVector2& pos ) override;
	virtual void HandleMouseMove( const CVector2& mousePos ) override;
	virtual void HandleChar( uint32 nChar ) override;
	virtual void BeforeRender() { m_pUIMgr->UpdateLayout(); }
protected:
	CReference<CUIManager> m_pUIMgr;
	CCamera2D m_camera;
};

class CMainGameState : public CUIMgrGameState
{
public:
	CMainGameState();

	virtual void EnterState() override;
	virtual void ExitState() override;
	virtual void UpdateInput() override;
	virtual void UpdateFrame() override;

	CWorld* GetWorld() { return m_pWorld; }
	void SetStageName( const char* szStage ) { m_strStage = szStage; }

	void DelayResetStage();

	DECLARE_GLOBAL_INST_REFERENCE( CMainGameState );
private:
	CWorld* m_pWorld;
	string m_strStage;
};

class CLevelDesignGameState : public CUIMgrGameState
{
public:
	CLevelDesignGameState();

	virtual void EnterState() override;
	virtual void ExitState() override;
	virtual void UpdateInput() override;
	virtual void UpdateFrame() override;

	CEntity* GetDesignLevel() { return m_pDesignLevel; }

	DECLARE_GLOBAL_INST_REFERENCE( CLevelDesignGameState );
private:
	CReference<CEntity> m_pDesignLevel;
	CStage* m_pDesignStage;
};