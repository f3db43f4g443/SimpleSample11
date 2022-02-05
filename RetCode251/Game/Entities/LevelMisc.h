#pragma once
#include "Entity.h"
#include "Interfaces.h"

class CLevelRepScrollLayer : public CEntity, public ILevelObjLayer
{
	friend void RegisterGameClasses_LevelMisc();
public:
	CLevelRepScrollLayer( const SClassCreateContext& context ) : CEntity( context ) { SET_BASEOBJECT_ID( CLevelRepScrollLayer ); }
	virtual bool GetBound( CRectangle& rect ) const override { return false; }
	virtual void Init() override;
	virtual void InitFromTemplate( CEntity* p, const CRectangle& rect ) override {}
	virtual bool IsPreview() { return true; }
	virtual void OnPreview();
	virtual void Update() override;
	virtual void UpdateScroll( const CVector4& camTrans ) override;
private:
	float m_fScrollSpeed;
	CVector2 m_scrollDir;
	float m_fContentShowDist;
	float m_fContentLen;
	float m_fOfsScale;

	struct SScrollItem
	{
		CReference<CEntity> pRoot;
		CReference<CPrefabNode> pPrefabNode;
		float x, y, r, s;
	};
	vector<SScrollItem> m_vecScrollItems;
	float m_fCurScrollOfs;
	vector<CReference<CEntity> > m_vecRepItems;
	int32 m_nBegin, m_nEnd;
};