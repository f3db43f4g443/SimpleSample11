#pragma once
#include "Entity.h"
#include "Common/StringUtil.h"

class CUseable : public CEntity
{
	friend void RegisterGameClasses();
public:
	CUseable( const char* szText, float fTime, float fCircleSize );
	CUseable( const SClassCreateContext& context );

	bool GetEnabled() { return m_bEnabled; }
	void SetText( const char* szText );
	void SetEnabled( bool bEnabled ) { m_bEnabled = bEnabled; }
	void SetUsing( bool bUsing, float fTime );
	void ResetUsing();
	void Use() { m_trigger.Trigger( eEntityEvent_PlayerUse, this ); }
private:
	float m_fTime;
	float m_fCurTime;
	bool m_bEnabled;
	CString m_strText;
	float m_fCircleSize;

	CReference<CEntity> m_pPercentCircle;
	CReference<CRenderObject2D> m_pText;
};

class CFastUseable : public CUseable
{
	friend void RegisterGameClasses();
public:
	CFastUseable( const char* szText, float fTime, float fCircleSize );
	CFastUseable( const SClassCreateContext& context );

	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;
protected:
	virtual void OnUse() {}
private:
	TClassTrigger<CFastUseable> m_onUse;
};