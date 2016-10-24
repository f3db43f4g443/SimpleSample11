#pragma once
#include "Attribute.h"
#include "Organ.h"
#include "SkinNMask.h"
#include "Render/TileMap2D.h"

enum
{
	eGridEdit_Disabled,
	eGridEdit_Empty,
	eGridEdit_NonEmpty,
	eGridEdit_Broken,

	eGridEdit_Valid,
	eGridEdit_Invalid = eGridEdit_Valid + eGridEdit_Valid
};

class CFace : public CEntity
{
	friend void RegisterGameClasses();
public:
	CFace( const SClassCreateContext& context ) : CEntity( context ), m_strDefaultSkinName( context ), m_tickAfterHitTest( this, &CFace::OnTickAfterHitTest ) { SET_BASEOBJECT_ID( CFace ); }

	struct SGrid
	{
		SGrid() : bEnabled( false ), pOrgan( NULL ), pSkin( NULL ), pMask( NULL ) {}
		bool bEnabled;
		COrgan* pOrgan;

		CSkin* pSkin;
		uint32 nSkinHp;
		uint32 nSkinMaxHp;
		CMask* pMask;
		SAttribute nMaskHp;

		CReference<CMultiFrameImage2D> pEffect;

		uint8 GetEditType()
		{
			if( !bEnabled )
				return eGridEdit_Disabled;
			if( pOrgan )
				return eGridEdit_NonEmpty;
			if( nSkinHp <= 0 )
				return eGridEdit_Broken;
			return eGridEdit_Empty;
		}
	};
	
	virtual void OnAddedToStage();
	virtual void OnRemovedFromStage();

	bool AddOrgan( COrgan* pOrgan, uint32 x, uint32 y );
	bool RemoveOrgan( COrgan* pOrgan );
	bool KillOrgan( COrgan* pOrgan );

	bool SetSkin( CSkin* pSkin, uint32 x, uint32 y );
	void SetSkinHp( uint32 nHp, uint32 x, uint32 y );
	void DamageSkin( uint32 nDmg, uint32 x, uint32 y );

	bool IsAwake() { return m_nAwakeFrames > 0; }
	void KeepAwake( uint32 nFrames ) { m_nAwakeFrames = Max( m_nAwakeFrames, nFrames ); }

	void OnBeginEdit();
	void OnEndEdit();
	void OnBeginSelectTarget();
	void OnEndSelectTarget();
	void UpdateSelectGrid( TVector2<int32> grid );

	CTileMap2D* GetEditTile() { return m_pFaceEditTile; }
	CTileMap2D* GetSelectTile() { return m_pFaceSelectTile; }
	CEntity* GetGUIRoot() { return m_pGUIRoot; }
	const CVector2& GetBaseOffset() { return m_baseOffset; }
	const CVector2& GetGridScale() { return m_gridScale; }
	CRectangle GetFaceRect();
	CRectangle GetKillBound();
	
	SGrid* GetGrid( uint32 x, uint32 y ) { return x < m_nWidth && y < m_nHeight ? &m_grids[x + y * m_nWidth] : NULL; }
	void RefreshEditTile( uint32 x, uint32 y, uint8 nFlag );
	void RefreshSelectTile( uint32 x, uint32 y, uint8 nType );

	bool IsEditValid( CFaceEditItem* pItem, const TVector2<int32>& pos );
private:
	void OnTickAfterHitTest();

	vector<SGrid> m_grids;
	CVector2 m_baseOffset;
	CVector2 m_gridScale;

	uint32 m_nWidth;
	uint32 m_nHeight;

	CString m_strDefaultSkinName;
	CSkin* m_pDefaultSkin;

	CEntity* m_pOrganRoot;
	CTileMapSet* m_pSkinTile;

	CReference<CTileMap2D> m_pFaceEditTile;
	CReference<CTileMap2D> m_pFaceSelectTile;
	CReference<CEntity> m_pGUIRoot;
	CReference<CRenderObject2D> m_pSelectEffect;

	uint32 m_nAwakeFrames;

	TClassTrigger<CFace> m_tickAfterHitTest;
};