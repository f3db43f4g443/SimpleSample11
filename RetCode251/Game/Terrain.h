#pragma once
#include "Entity.h"
#include "Interfaces.h"

class CTerrain : public CEntity, public IEditorTiled, public ILevelObjLayer
{
	friend void RegisterGameClasses();
	friend class CLevelEditObjectToolTerrain;
public:
	CTerrain( const SClassCreateContext& context ) : CEntity( context ) { SET_BASEOBJECT_ID( CTerrain ); }
	virtual bool IsPreview() { return true; }
	virtual void OnPreview();
	virtual void OnAddedToStage() override;

	virtual bool GetBound( CRectangle& rect ) const override
	{ rect = CRectangle( m_ofs.x, m_ofs.y, m_nTileX * m_tileSize.x, m_nTileY * m_tileSize.y ); return true; }
	virtual void InitFromTemplate( CEntity* p, const CRectangle& rect ) override;
	virtual CVector2 GetTileSize() const override { return m_tileSize; }
	virtual TVector2<int32> GetSize() const override { return TVector2<int32>( m_nTileX, m_nTileY ); }
	virtual TVector2<int32> GetMinSize() const override { return TVector2<int32>( 4, 4 ); }
	virtual CVector2 GetBaseOfs() const override { return m_ofs; }
	virtual void Resize( const TRectangle<int32>& size ) override
	{ m_ofs = m_ofs + CVector2( size.x, size.y ) * m_tileSize; m_nTileX = size.width; m_nTileY = size.height; }
	virtual void SetBaseOfs( const CVector2& ofs ) override { m_ofs = ofs; }
private:
	uint32 m_nBlockIndexBegin;
	int32 m_nBorder;
	CVector2 m_tileSize;
	int32 m_nTileX, m_nTileY;
	CVector2 m_ofs;

	bool m_bInited;
};