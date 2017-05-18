#pragma once
#include "LevelGenerate.h"

class CLevelGenNode1_1 : public CLevelGenerateNode
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