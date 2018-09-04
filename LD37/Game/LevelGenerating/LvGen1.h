#pragma once
#include "LevelGenerate.h"

class CLevelGenNode1_1 : public CLevelGenerateNode
{
public:
	virtual void Load( TiXmlElement* pXml, struct SLevelGenerateNodeLoadContext& context ) override;
	virtual void Generate( SLevelBuildContext& context, const TRectangle<int32>& region ) override;
private:
	void GenRegions();
	void FillRegion( const TRectangle<int32>& rect );
	void CalcB( int8* b, const TRectangle<int32>& rect );
	void FillEmptyArea();
	void GenBars();
	void GenChunks();
	void GenWallChunks();
	void GenBlocks();
	void GenWallChunks1();
	enum
	{
		eType_None,
		eType_Temp,
		eType_Temp1,
		eType_Temp2,
		eType_Temp3,
		eType_WallChunk,
		eType_WallChunk1,
		eType_Obj,
		eType_Bonus,

		eType_Bar,
		eType_Bar1,
		eType_Stone,
		eType_Block1x,
		eType_Block2x,
		eType_Web,
	};

	SLevelBuildContext* m_pContext;
	TRectangle<int32> m_region;
	vector<int8> m_gendata;
	vector<TRectangle<int32> > m_bars;
	vector<TRectangle<int32> > m_stones;
	vector<TRectangle<int32> > m_wallChunks;

	CReference<CLevelGenerateNode> m_pWallNode;
	CReference<CLevelGenerateNode> m_pStoneNode;
	CReference<CLevelGenerateNode> m_pBlock1xNode;
	CReference<CLevelGenerateNode> m_pBlock2xNode;
	CReference<CLevelGenerateNode> m_pBarNode;
	CReference<CLevelGenerateNode> m_pBar2Node;
	CReference<CLevelGenerateNode> m_pWallChunkNode;
	CReference<CLevelGenerateNode> m_pObjNode;
	CReference<CLevelGenerateNode> m_pBonusNode;
};

class CLevelGenNode1_1_0 : public CLevelGenerateNode
{
public:
	virtual void Load( TiXmlElement* pXml, struct SLevelGenerateNodeLoadContext& context ) override;
	virtual void Generate( SLevelBuildContext& context, const TRectangle<int32>& region ) override;
private:
	void GenAreas();
	void MakeHoles();

	enum
	{
		eType_None,
		eType_Wall,
		eType_Obj,
		eType_Bonus,
		eType_Block1,
		eType_Block2,
		eType_Stone,
	};

	SLevelBuildContext* m_pContext;
	TRectangle<int32> m_region;
	vector<int8> m_gendata;
	vector<TRectangle<int32> > m_vecStones;

	CReference<CLevelGenerateNode> m_pWallNode;
	CReference<CLevelGenerateNode> m_pStoneNode;
	CReference<CLevelGenerateNode> m_pBlock1Node;
	CReference<CLevelGenerateNode> m_pBlock2Node;
	CReference<CLevelGenerateNode> m_pObjNode;
	CReference<CLevelGenerateNode> m_pBonusNode;
};

class CLevelGenNode1_1_1 : public CLevelGenerateNode
{
public:
	virtual void Load( TiXmlElement* pXml, struct SLevelGenerateNodeLoadContext& context ) override;
	virtual void Generate( SLevelBuildContext& context, const TRectangle<int32>& region ) override;
private:
	void GenMainPath();
	void Flatten();
	void GenRooms();
	void GenObstacles();
	void GenObjsBig();
	void GenObjsSmall();
	void GenBlocks();

	enum
	{
		eType_None,
		eType_Path,
		eType_Bar,
		eType_Stone,
		eType_Room,
		eType_Door,
		eType_Block1x,
		eType_Block2x,
		eType_WallChunk,
		eType_Web,
		eType_Obj,
		eType_Bonus,
		eType_Temp,
		eType_Temp1,
	};

	SLevelBuildContext* m_pContext;
	TRectangle<int32> m_region;
	vector<int8> m_gendata;
	vector<TRectangle<int32> > m_bars;
	vector<TRectangle<int32> > m_stones;
	vector<TRectangle<int32> > m_rooms;
	vector<TRectangle<int32> > m_wallChunks;

	CReference<CLevelGenerateNode> m_pWallNode;
	CReference<CLevelGenerateNode> m_pStoneNode;
	CReference<CLevelGenerateNode> m_pBlock1xNode;
	CReference<CLevelGenerateNode> m_pBlock2xNode;
	CReference<CLevelGenerateNode> m_pBarNode;
	CReference<CLevelGenerateNode> m_pBar2Node;
	CReference<CLevelGenerateNode> m_pRoomNode;
	CReference<CLevelGenerateNode> m_pWallChunkNode;
	CReference<CLevelGenerateNode> m_pObjNode;
	CReference<CLevelGenerateNode> m_pBonusNode;
};

class CLevelGenNode1_1_2 : public CLevelGenerateNode
{
public:
	virtual void Load( TiXmlElement* pXml, struct SLevelGenerateNodeLoadContext& context ) override;
	virtual void Generate( SLevelBuildContext& context, const TRectangle<int32>& region ) override;
private:
	void GenRooms();
	void GenRooms1( int32 nDist );
	void LinkRoom( int8 nRoomPosType );
	void AddMoreBars();
	void AddBar( TRectangle<int32>& r, vector<TVector4<int8> >& vecWeight );
	void AddBar1( TRectangle<int32>& r, vector<TVector4<int8> >& vecWeight );
	void AddBar2( TRectangle<int32>& r );
	void UpdateWeight( const TRectangle<int32>& rect, vector<TVector4<int8> >& vecWeight, int8 n = 1 );
	void FixBars();
	void GenWallChunks();
	void GenWallChunk( const TRectangle<int32>& rect, vector<int8>& vecData );
	void GenObjs();
	void GenBlocks();
	void GenBonus();

	enum
	{
		eType_None,
		eType_Path,
		eType_Stone,
		eType_WallChunk,
		eType_WallChunk1,
		eType_Bar,
		eType_Room,
		eType_Door,
		eType_Block1x,
		eType_Block2x,
		eType_Obj,
		eType_Bonus,
		eType_Web,
		eType_Web1,
		eType_Temp,
		eType_Temp1,
		eType_Temp2,
	};

	SLevelBuildContext* m_pContext;
	TRectangle<int32> m_region;
	vector<int8> m_gendata;
	vector<int32> m_vecHeight;

	struct SRoom
	{
		uint8 nType;
		TRectangle<int32> rect;
	};
	vector<SRoom> m_rooms;
	vector<TRectangle<int32> > m_bars;
	vector<TRectangle<int32> > m_stones;
	vector<TRectangle<int32> > m_wallChunks;
	vector<TVector2<int32> > m_path;
	vector<TVector2<int32> > m_pathFindingTarget;
	vector<TVector2<int32> > m_par;

	CReference<CLevelGenerateNode> m_pWallNode;
	CReference<CLevelGenerateNode> m_pStoneNode;
	CReference<CLevelGenerateNode> m_pBlock1xNode;
	CReference<CLevelGenerateNode> m_pBlock2xNode;
	CReference<CLevelGenerateNode> m_pBarNode;
	CReference<CLevelGenerateNode> m_pBar2Node;
	CReference<CLevelGenerateNode> m_pRoom1Node;
	CReference<CLevelGenerateNode> m_pRoom2Node;
	CReference<CLevelGenerateNode> m_pWallChunkNode;
	CReference<CLevelGenerateNode> m_pObjNode;
	CReference<CLevelGenerateNode> m_pBonusNode;
	CReference<CLevelGenerateNode> m_pWebNode;
};

class CLevelGenNode1_1_3 : public CLevelGenerateNode
{
public:
	virtual void Load( TiXmlElement* pXml, struct SLevelGenerateNodeLoadContext& context ) override;
	virtual void Generate( SLevelBuildContext& context, const TRectangle<int32>& region ) override;
private:
	void GenAreas();
	void GenObstacles();
	void GenShops();

	enum
	{
		eType_None,
		eType_Bar,
		eType_Stone,
		eType_Room,
		eType_Door,
		eType_Block1x,
		eType_Block2x,
		eType_Block1y,
		eType_Block2y,
		eType_WallChunk,
		eType_Obj,
		eType_Bonus,
		eType_Web,

		eType_Temp,
		eType_Temp1,
	};

	SLevelBuildContext* m_pContext;
	TRectangle<int32> m_region;
	vector<int8> m_gendata;
	vector<int8> m_gendata1;

	struct SArea
	{
		int8 nType;
		TRectangle<int32> rect;
	};
	struct SRoom
	{
		uint8 nType;
		TRectangle<int32> rect;
	};
	vector<SArea> m_areas;
	vector<SRoom> m_rooms;
	vector<TRectangle<int32> > m_bars;
	vector<TRectangle<int32> > m_stones;
	vector<TRectangle<int32> > m_wallChunks;
	int32 m_nShop;

	CReference<CLevelGenerateNode> m_pWallNode;
	CReference<CLevelGenerateNode> m_pStoneNode;
	CReference<CLevelGenerateNode> m_pBlock1xNode;
	CReference<CLevelGenerateNode> m_pBlock2xNode;
	CReference<CLevelGenerateNode> m_pBlock1yNode;
	CReference<CLevelGenerateNode> m_pBlock2yNode;
	CReference<CLevelGenerateNode> m_pBarNode;
	CReference<CLevelGenerateNode> m_pBar2Node;
	CReference<CLevelGenerateNode> m_pRoom1Node;
	CReference<CLevelGenerateNode> m_pRoom2Node;
	CReference<CLevelGenerateNode> m_pWallChunkNode;
	CReference<CLevelGenerateNode> m_pObjNode;
	CReference<CLevelGenerateNode> m_pBonusNode;
	CReference<CLevelGenerateNode> m_pWebNode;
	CReference<CLevelGenerateNode> m_pShopNode;
};

class CLevelGenNode1_2 : public CLevelGenerateNode
{
public:
	virtual void Load( TiXmlElement* pXml, struct SLevelGenerateNodeLoadContext& context ) override;
	virtual void Generate( SLevelBuildContext& context, const TRectangle<int32>& region ) override;
private:
	void GenWallChunks();
	void GenRooms();
	void PutHBars();
	bool ConnRooms();
	void GenConnAreas();
	void GenDoors( bool b );
	void GenEmptyArea();
	void FillBlockArea();
	void GenObjects();
	void GenSpiders( SLevelBuildContext& context, const TRectangle<int32>& region );

	CReference<CLevelGenerateNode> m_pWallNode;
	CReference<CLevelGenerateNode> m_pWallChunkNode;
	CReference<CLevelGenerateNode> m_pWallChunk0Node;
	CReference<CLevelGenerateNode> m_pBlock1Node;
	CReference<CLevelGenerateNode> m_pBlock2Node;
	CReference<CLevelGenerateNode> m_pRoom1Node;
	CReference<CLevelGenerateNode> m_pRoom2Node;
	CReference<CLevelGenerateNode> m_pBar1Node;
	CReference<CLevelGenerateNode> m_pBar2Node;
	CReference<CLevelGenerateNode> m_pObjNode;
	CReference<CLevelGenerateNode> m_pCrate1Node;
	CReference<CLevelGenerateNode> m_pCrate2Node;
	CReference<CLevelGenerateNode> m_pShopNode;
	CReference<CLevelGenerateNode> m_pWebNode;
	CReference<CLevelGenerateNode> m_pSpiderNode;

	enum
	{
		eType_None,
		eType_BlockRed,
		eType_BlockBlue,
		eType_Room1,
		eType_Room2,
		eType_Bar,
		eType_WallChunk0,
		eType_WallChunk,
		eType_Door,
		eType_Path,
		eType_Object,
		eType_Crate1,
		eType_Crate2,
		eType_Web,

		eType_Count,
	};
	SLevelBuildContext* m_pContext;
	TRectangle<int32> m_region;
	vector<int8> m_gendata;
	struct SRoom
	{
		uint8 nType;
		bool bShop;
		TRectangle<int32> rect;
	};
	vector<SRoom> m_rooms;
	struct SBar
	{
		uint8 nType;
		TRectangle<int32> rect;
	};
	vector<SBar> m_bars;
};

class CLevelGenNode1_3 : public CLevelGenerateNode
{
public:
	virtual void Load( TiXmlElement* pXml, struct SLevelGenerateNodeLoadContext& context ) override;
	virtual void Generate( SLevelBuildContext& context, const TRectangle<int32>& region ) override;
private:
	void GenBase();
	void GenSubArea( const TRectangle<int32>& rect );
	void FillLine( TRectangle<int32>& rect );
	void GenRestWallChunks();
	void GenWallChunk();
	void GenWallChunk0();
	void GenWindows();
	void GenObjs();
	void GenShops();

	uint8 m_nType;
	CReference<CLevelGenerateNode> m_pWallNode;
	CReference<CLevelGenerateNode> m_pWallChunkNode;
	CReference<CLevelGenerateNode> m_pWallChunk0Node;
	CReference<CLevelGenerateNode> m_pWallChunk1Node;
	CReference<CLevelGenerateNode> m_pBlock1Node;
	CReference<CLevelGenerateNode> m_pBlock2Node;
	CReference<CLevelGenerateNode> m_pRoomNode;
	CReference<CLevelGenerateNode> m_pCrate1Node;
	CReference<CLevelGenerateNode> m_pCrate2Node;
	CReference<CLevelGenerateNode> m_pScrapNode;
	CReference<CLevelGenerateNode> m_pWindowNode;
	CReference<CLevelGenerateNode> m_pWindow1Node[4];
	CReference<CLevelGenerateNode> m_pSpiderNode;
	CReference<CLevelGenerateNode> m_pShopNode;

	enum
	{
		eType_None,
		eType_Wall,
		eType_Room,
		eType_Room1,
		eType_Door,
		eType_WallChunk0,
		eType_WallChunk,
		eType_WallChunk1,
		eType_WallChunk1_1,
		eType_WallChunk1_2,
		eType_BlockRed,
		eType_BlockBlue,
		eType_Crate1,
		eType_Crate2,
		eType_Crate3,
		eType_Web,

		eType_WallChunk_0,
		eType_WallChunk_0_1,
		eType_WallChunk_0_1a,
		eType_WallChunk_0_1b,
		eType_WallChunk_1,
		eType_WallChunk_2,
		eType_WallChunk_3,

		eType_Temp,
		eType_Temp1,

		eType_Count,
	};
	SLevelBuildContext* m_pContext;
	TRectangle<int32> m_region;
	vector<int8> m_gendata;

	struct SRoom
	{
		SRoom( const TRectangle<int32>& rect ) : rect( rect ), bRoomType( false ) {}
		TRectangle<int32> rect;
		bool bRoomType;
	};
	vector<SRoom> m_rooms;
	vector<TRectangle<int32> > m_wallChunks0;
	vector<TRectangle<int32> > m_wallChunks;
	vector<TRectangle<int32> > m_wallChunks1;
	vector<TRectangle<int32> > m_scraps;
	vector<TRectangle<int32> > m_windows;
	vector<TRectangle<int32> > m_windows1[4];
	vector<TRectangle<int32> > m_shops;
	vector<TRectangle<int32> > m_spiders;
};

class CLvBarrierNodeGen1 : public CLevelGenerateSimpleNode
{
public:
	virtual void Load( TiXmlElement* pXml, struct SLevelGenerateNodeLoadContext& context ) override;
	virtual void Generate( SLevelBuildContext& context, const TRectangle<int32>& region ) override;
private:
	void GenBase();
	void GenRooms();
	void GenWalls();
	void GenWindows();

	CReference<CLevelGenerateNode> m_pLabelNode;
	CReference<CLevelGenerateNode> m_pFillNode;
	CReference<CLevelGenerateNode> m_pBaseNode;
	CReference<CLevelGenerateNode> m_pCoreNode;
	CReference<CLevelGenerateNode> m_pRoomNode;
	CReference<CLevelGenerateNode> m_pWallHNode;
	CReference<CLevelGenerateNode> m_pWallVNode;
	CReference<CLevelGenerateNode> m_pWindowNode;

	enum
	{
		eType_None,
		eType_Base,
		eType_Core,
		eType_Room,
		eType_Wall,
		eType_WallH,
		eType_WallV,
	};

	SLevelBuildContext* m_pContext;
	TRectangle<int32> m_region;
	vector<int8> m_gendata;
	TRectangle<int32> m_labelRect;
	vector<TRectangle<int32> > m_rooms;
	vector<TRectangle<int32> > m_walls;
	vector<TRectangle<int32> > m_windows;
};