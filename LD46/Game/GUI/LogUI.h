#pragma once
#include "Entities/UtilEntities.h"

class CLogUI : public CEntity
{
	friend void RegisterGameClasses_LogUI();
public:
	CLogUI( const SClassCreateContext& context ) : CEntity( context ) { SET_BASEOBJECT_ID( CLogUI ); }

	virtual void OnAddedToStage() override;
	void Show( int8 nPage = -1, int32 nIndex = 0 );
	void Refresh( bool bInit );
	void Update();
	void AddDoc( bool bUnlocked, const char* szName, const char* szContent );
private:
	CString m_strScriptInit;
	float m_fPageItemWidth;
	float m_fDocTitleHeight;
	float m_fRecordTexLeft;
	float m_fRecordTexRight;
	CReference<CEntity> m_pPages[2];
	CReference<CSimpleText> m_pPageItemText[2];
	CReference<CEntity> m_pPageSelectEffect;
	CReference<CSimpleText> m_pDocTitleText[8];
	CReference<CEntity> m_pDocSelectEffect;
	CReference<CSimpleText> m_pDocContentText;
	CReference<CSimpleText> m_pRecordItemText[16];
	CReference<CEntity> m_pRecordScroll;
	
	CRectangle m_recordScrollOrigRect;
	int8 m_nSelectedPage;
	int32 m_nSelectedIndex;
	int32 m_nShowBeginIndex;
	struct SDocument
	{
		bool bUnlocked;
		string strName;
		string strContent;
	};
	vector<SDocument> m_vecDocuments;
	struct SRecord
	{
		int8 nType;
		CVector4 color;
		string strText;
	};
	vector<SRecord> m_vecRecords;
};