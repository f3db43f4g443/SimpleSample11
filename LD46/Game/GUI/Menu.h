#pragma once
#include "MyLevel.h"
#include "Entities/UtilEntities.h"


enum
{
	eStartMenuResult_None,
	eStartMenuResult_Continue,
	eStartMenuResult_NewGame,
	eStartMenuResult_Quit,
};

class CStartMenu : public CEntity
{
	friend void RegisterGameClasses_Menu();
public:
	CStartMenu( const SClassCreateContext& context ) : CEntity( context ) { SET_BASEOBJECT_ID( CStartMenu ); }
	void Init();
	int8 Update();
	virtual void Render( CRenderContext2D& context ) override;
private:
	int8 OnSelect();
	void UpdateEffect();
	TResourceRef<CPrefab> m_pBackLevelPrefab;
	CVector4 m_textSelected, m_textUnSelected;
	CReference<CSimpleText> m_pMenuItem[3];

	CReference<CMyLevel> m_pBackLevel;
	int32 m_nCurSelectedItem;
	vector<int32> m_vecValidMenuItem;
	vector<CElement2D> m_vecElems;
	vector<CVector4> m_vecElemParams;
};