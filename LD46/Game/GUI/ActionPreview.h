#pragma once
#include "Entities/UtilEntities.h"
#include "MyLevel.h"

class CActionPreview : public CEntity
{
	friend void RegisterGameClasses_ActionPreview();
public:
	CActionPreview( const SClassCreateContext& context ) : CEntity( context ) { SET_BASEOBJECT_ID( CActionPreview ); }
	virtual void OnAddedToStage() override { m_pInputRoot->SetRenderObject( NULL ); }
	void Show( bool bShow );
	void Update();
private:
	void CreateLevel();
	void Clear();
	void RefreshText();
	TResourceRef<CPrefab> m_pLevelPrefab;
	int32 m_nStartX, m_nStartY;
	CReference<CEntity> m_pInputRoot;
	CReference<CEntity> m_pTip;

	CReference<CMyLevel> m_pPreviewLevel;
	CReference<CPlayer> m_pPreviewPlayer;
	bool m_bWaitingInput;
	bool m_bFailed;
	vector<CReference<CRenderObject2D> > m_vecText[ePlayerStateSource_Count];
	int32 m_nSelectedType;
	int32 m_nSelectedIndex;
};