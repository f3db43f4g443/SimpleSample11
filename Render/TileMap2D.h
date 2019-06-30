#pragma once
#include "RenderObject2D.h"

struct STileMapInfo
{
	STileMapInfo() : nWidth( 0 ), nHeight( 0 ), nTileCount( 0 ), texSize( 0, 0 ), tileSize( 0, 0 ), tileStride( 0, 0 ), tileOffset( 0, 0 ), defaultTileSize( 0, 0 ) {}
	struct SEditInfo
	{
		uint32 nBegin;
		uint32 nCount;

		int32 nEditParent;
		uint32 nBlendBegin;
		uint32 nBlendCount;

		uint8 nType;
		uint8 nUserData;
	};

	uint32 nWidth;
	uint32 nHeight;
	uint32 nTileCount;
	CVector2 texSize;
	CVector2 tileSize;
	CVector2 tileStride;
	CVector2 tileOffset;

	CVector2 defaultTileSize;

	vector<CVector4> params;
	vector<SEditInfo> editInfos;
};

class CTileMap2D : public CRenderObject2D
{
public:
	CTileMap2D( CDrawable2D* pDrawable, CDrawable2D* pOcclusionDrawable, STileMapInfo* pInfo, const CVector2& tileSize, const CVector2& baseOffset, uint32 nWidth, uint32 nHeight, bool bGUI,
		uint16 nParamCount, uint16 nColorParamBeginIndex, uint16 nColorParamCount, uint16 nOcclusionParamBeginIndex,
		uint16 nOcclusionParamCount, uint16 nGUIParamBeginIndex, uint16 nGUIParamCount );

	STileMapInfo* GetInfo() { return m_pInfo; }
	CVector2 GetTileSize() { return m_tileSize; }
	CVector2 GetBaseOffset() { return m_baseOffset; }
	uint32 GetWidth() { return m_nWidth; }
	uint32 GetHeight() { return m_nHeight; }
	void SetTileSize( const CVector2& tileSize ) { Set( tileSize, m_baseOffset, m_nWidth, m_nHeight ); }
	void SetBaseOffset( const CVector2& baseOffset ) { Set( m_tileSize, baseOffset, m_nWidth, m_nHeight ); }
	void SetSize( uint32 nWidth, uint32 nHeight ) { Set( m_tileSize, m_baseOffset, nWidth, nHeight ); }
	void Resize( const TRectangle<int32>& rect );
	void Set( const CVector2& tileSize, const CVector2& baseOffset, uint32 nWidth, uint32 nHeight );

	void CopyData( CTileMap2D* pCopyFrom );
	void LoadData( IBufReader& buf );
	void SaveData( CBufFile& buf );
	
	struct STile
	{
		uint32 nLayers;
		uint16 nTiles[4];
		CElement2D elems[3][4];
	};
	const STile& GetTile( uint32 x, uint32 y ) { return m_tiles[x + y * m_nWidth]; }
	void SetTile( uint32 x, uint32 y, uint32 nLayers, const uint16* nValues );
	void AddTileLayer( uint32 x, uint32 y, uint16 nValue );
	void EditTile( uint32 x, uint32 y, uint32 nValue );
	uint32 GetEditData( uint32 x, uint32 y ) { return m_editData[x + y * ( m_nWidth + 1 )]; }
	uint8 GetUserData( uint32 x, uint32 y );
	void SetEditData( uint32 x, uint32 y, uint32 nValue ) { m_editData[x + y * ( m_nWidth + 1 )] = nValue; }
	void RefreshAll();

	virtual void Render( CRenderContext2D& context ) override;
private:
	void RefreshTile( uint32 x, uint32 y );
	CDrawable2D* m_pColorDrawable;
	CDrawable2D* m_pOcclusionDrawable;
	CDrawable2D* m_pGUIDrawable;
	STileMapInfo* m_pInfo;
	CVector2 m_tileSize;
	CVector2 m_baseOffset;
	uint32 m_nWidth;
	uint32 m_nHeight;
	vector<STile> m_tiles;
	vector<uint32> m_editData;

	uint16 m_nParamCount;
	uint16 m_nColorParamBeginIndex, m_nColorParamCount;
	uint16 m_nOcclusionParamBeginIndex, m_nOcclusionParamCount;
	uint16 m_nGUIParamBeginIndex, m_nGUIParamCount;
};

struct STileMapSetData
{
	struct SItem
	{
		string strName;
		string strResource;
	};
	map<string, SItem> items;
};

class CTileMapSet : public CRenderObject2D
{
public:
	CTileMapSet( const CVector2& tileSize, const CVector2& baseOffset, uint32 nWidth, uint32 nHeight, STileMapSetData* pData = NULL );

	CVector2 GetTileSize() { return m_tileSize; }
	CVector2 GetBaseOffset() { return m_baseOffset; }
	uint32 GetWidth() { return m_nWidth; }
	uint32 GetHeight() { return m_nHeight; }
	void SetTileSize( const CVector2& tileSize ) { Set( tileSize, m_baseOffset, m_nWidth, m_nHeight ); }
	void SetBaseOffset( const CVector2& baseOffset ) { Set( m_tileSize, baseOffset, m_nWidth, m_nHeight ); }
	void SetSize( uint32 nWidth, uint32 nHeight ) { Set( m_tileSize, m_baseOffset, nWidth, nHeight ); }
	void Set( const CVector2& tileSize, const CVector2& baseOffset, uint32 nWidth, uint32 nHeight );

	struct STileMapLayerInfo
	{
		STileMapLayerInfo() : nTileMapRefCount( 0 ) {}
		string strName;
		CReference<CResource> pResource;
		CReference<CTileMap2D> pTileMap;

		uint32 nTileMapRefCount;
	};

	struct SEditData
	{
		SEditData() : pInfo( NULL ) {}

		STileMapLayerInfo* pInfo;
	};

	void AddLayer( const char* szName, CResource* pResource );
	void EditTile( uint32 x, uint32 y, const char* szName, uint32 nValue );
private:
	CVector2 m_tileSize;
	CVector2 m_baseOffset;
	uint32 m_nWidth;
	uint32 m_nHeight;
	STileMapSetData* m_pData;

	map<string, STileMapLayerInfo> m_mapTileMaps;
	vector<SEditData> m_editData;
};