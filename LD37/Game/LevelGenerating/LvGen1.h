#pragma once
#include "LevelGenerate.h"

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
		eType_Block1x,
		eType_Block2x,
		eType_Block1y,
		eType_Block2y,
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

	CReference<CLevelGenerateNode> m_pWallNode;
	CReference<CLevelGenerateNode> m_pStoneNode;
	CReference<CLevelGenerateNode> m_pBlock1xNode;
	CReference<CLevelGenerateNode> m_pBlock2xNode;
	CReference<CLevelGenerateNode> m_pBlock1yNode;
	CReference<CLevelGenerateNode> m_pBlock2yNode;
	CReference<CLevelGenerateNode> m_pBarNode;
	CReference<CLevelGenerateNode> m_pBar2Node;
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
	void GenRooms1();
	void LinkRoom( int8 nRoomPosType );
	void AddMoreBars();
	void GenObjs();
	void GenBlocks();
	void GenBonus();

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
		eType_Block1y,
		eType_Block2y,
		eType_Obj,
		eType_Bonus,
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
	vector<TVector2<int32> > m_path;
	vector<TVector2<int32> > m_pathFindingTarget;
	vector<TVector2<int32> > m_par;

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
	CReference<CLevelGenerateNode> m_pObjNode;
	CReference<CLevelGenerateNode> m_pBonusNode;
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
	void ConnRooms();
	void GenConnAreas();
	void GenDoors();
	void GenEmptyArea();
	void FillBlockArea();
	void GenObjects();

	CReference<CLevelGenerateNode> m_pWallNode;
	CReference<CLevelGenerateNode> m_pWallChunkNode;
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

	enum
	{
		eType_None,
		eType_BlockRed,
		eType_BlockBlue,
		eType_Room1,
		eType_Room2,
		eType_WallChunk,
		eType_Bar,
		eType_Door,
		eType_Path,
		eType_Object,
		eType_Crate1,
		eType_Crate2,

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
	CReference<CLevelGenerateNode> m_pWindowNode;
	CReference<CLevelGenerateNode> m_pShopNode;

	enum
	{
		eType_None,
		eType_Wall,
		eType_Room,
		eType_Door,
		eType_WallChunk0,
		eType_WallChunk,
		eType_WallChunk1,
		eType_BlockRed,
		eType_BlockBlue,
		eType_Crate1,
		eType_Crate2,

		eType_Temp,
		eType_Temp1,

		eType_Count,
	};
	SLevelBuildContext* m_pContext;
	TRectangle<int32> m_region;
	vector<int8> m_gendata;

	vector<TRectangle<int32> > m_rooms;
	vector<TRectangle<int32> > m_wallChunks0;
	vector<TRectangle<int32> > m_wallChunks;
	vector<TRectangle<int32> > m_wallChunks1;
	vector<TRectangle<int32> > m_windows;
	vector<TRectangle<int32> > m_shops;
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