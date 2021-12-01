#pragma once
#include "Character.h"
#include "Interfaces.h"

enum EBugFixType
{
	eBugFixType_Common,
	eBugFixType_Melee,
	eBugFixType_Range,
	eBugFixType_System,
	eBugFixType_None,
};

class CBug : public CCharacter
{
	friend void RegisterGameClasses();
	friend class CMyLevel;
	friend class CLevelBugsTool;
public:
	CBug( const SClassCreateContext& context ) : CCharacter( context ), m_onTick( this, &CBug::OnTick ) { SET_BASEOBJECT_ID( CBug ) };

	virtual bool IsPreview() { return true; }
	virtual void OnPreview() { UpdateImg( true ); }
	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;
	virtual bool Damage( SDamageContext& context );
	virtual bool IsResetable() { return true; }
	bool IsActivated();
	bool IsFixed() { return m_bFixed; }
	bool IsCaught() { return false; }
	void Fix();
	void Clear( bool bFirst );
	int32 GetExp() { return m_nExp; }
	virtual void OnTickAfterHitTest() override;

	static CVector4 GetGroupColor( int32 nGroup );
private:
	void OnTick() { UpdateImg( false ); }
	void UpdateImg( bool bPreview );
	int32 m_nBugID;
	int32 m_nGroup;
	int32 m_nExp;
	EBugFixType m_nFixType;
	CReference<CRenderObject2D> m_pFixedEft;
	TResourceRef<CSoundFile> m_pSoundFixed;
	TResourceRef<CSoundFile> m_pSoundCleared;

	bool m_bDetected;
	bool m_bFixed;
	bool m_bCurActivated;

	TClassTrigger<CBug> m_onTick;
};
