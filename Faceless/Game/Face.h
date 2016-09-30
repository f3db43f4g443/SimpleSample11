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
	CFace( const SClassCreateContext& context ) : CEntity( context ) { SET_BASEOBJECT_ID( CFace ); }

	struct SGrid
	{
		SGrid() : bEnabled( false ), pOrgan( NULL ), pSkin( NULL ), pMask( NULL ) {}
		bool bEnabled;
		COrgan* pOrgan;

		CSkin* pSkin;
		SAttribute nSkinHp;
		CMask* pMask;
		SAttribute nMaskHp;

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

	bool SetSkin( CSkin* pSkin, uint32 x, uint32 y );

	void OnBeginEdit();
	void OnEndEdit();

	CTileMap2D* GetEditTile() { return m_pFaceEditTile; }
	const CVector2& GetBaseOffset() { return m_baseOffset; }
	const CVector2& GetGridScale() { return m_gridScale; }
	CRectangle GetFaceRect();
	CRectangle GetKillBound();
	
	SGrid* GetGrid( uint32 x, uint32 y ) { return x < m_nWidth && y < m_nHeight ? &m_grids[x + y * m_nWidth] : NULL; }
	void RefreshEditTile( uint32 x, uint32 y, uint8 nFlag );

	bool IsEditValid( CFaceEditItem* pItem, const TVector2<int32>& pos );
private:
	vector<SGrid> m_grids;
	CVector2 m_baseOffset;
	CVector2 m_gridScale;

	uint32 m_nWidth;
	uint32 m_nHeight;

	uint32 m_nDefaultSkinMaxHp;
	uint32 m_nDefaultSkinHp;

	CEntity* m_pOrganRoot;
	CTileMapSet* m_pSkinTile;

	CReference<CTileMap2D> m_pFaceEditTile;
};