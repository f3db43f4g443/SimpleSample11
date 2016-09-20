#pragma once
#include "Entity.h"
#include "Common/StringUtil.h"
#include "Render/DrawableGroup.h"
#include "Render/Image2D.h"

class CGlitchEffect : public CEntity
{
	friend void RegisterGameClasses();
public:
	CGlitchEffect( const SClassCreateContext& context );
	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;

	void SetMaxOfs( uint8 nIndex, float fValue ) { m_fMaxTexOfs[nIndex] = fValue; }
	void SetP( uint32 p[4] )
	{
		m_nP = 0;
		for( int i = 0; i < m_nPrefabCount; i++ )
		{
			m_p[i] = p[i];
			m_nP += p[i];
		}
	}
private:
	void OnTickBeforeHitTest();

	void AddNode();
	void RemoveNode();
	void ChangeNode();

	uint8 m_nPrefabCount;
	uint32 m_nMaxNodes;
	CRectangle m_effectRect;
	CString m_strMaterial0;
	CString m_strMaterial1;
	CString m_strMaterial2;
	CString m_strMaterial3;
	uint32 m_p[4];
	uint32 m_nP;
	CVector2 m_baseTex[4];
	float m_texSize[4];
	float m_fMaxTexOfs[4];

	CReference<CDrawableGroup> m_pMaterials[4];
	TClassTrigger<CGlitchEffect> m_onTickBeforeHitTest;

	struct SNode
	{
		bool bVertical;
		uint8 nType;
		float fSplitPercent;
		CRectangle rect;
		CReference<CImage2D> pImg;
		SNode* pParent;
		SNode* pLeft;
		SNode* pRight;
		
		void UpdateImg( CGlitchEffect* pOwner );
		void UpdateChildren( CGlitchEffect* pOwner );
		LINK_LIST( SNode, Node );
	};
	uint32 m_nNodes;
	LINK_LIST_HEAD( m_pNodes, SNode, Node );
};