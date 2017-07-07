#pragma once
#include "LevelGenerate.h"
#include "Entities/Blocks/RandomBlocks.h"

class CMainMenuGenerateNode : public CLevelGenerateSimpleNode
{
public:
	virtual void Load( TiXmlElement* pXml, struct SLevelGenerateNodeLoadContext& context ) override;
	virtual void Generate( SLevelBuildContext& context, const TRectangle<int32>& region ) override;
private:
	CReference<CLevelGenerateNode> m_pButtonValidNode;
	CReference<CLevelGenerateNode> m_pButtonInvalidNode;
	vector<TVector2<int32> > m_vecButtonPos;
};

class CMainMenu : public CRandomChunkTiled
{
	friend void RegisterGameClasses();
public:
	CMainMenu( const SClassCreateContext& context ) : CRandomChunkTiled( context ), m_strButton( context ), m_onKill( this, &CMainMenu::OnKill ) { SET_BASEOBJECT_ID( CMainMenu ); }
	virtual void OnCreateComplete( class CMyLevel* pLevel ) override;
private:
	void OnButton( int32 i );
	void OnKill();

	CString m_strButton;
	vector<CReference<CChunkObject> > m_vecButtons;
	vector<CFunctionTrigger> m_triggers;
	TClassTrigger<CMainMenu> m_onKill;
};