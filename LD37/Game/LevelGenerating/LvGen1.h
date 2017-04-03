#pragma once
#include "LevelGenerate.h"

class CLevelGenNode1_1 : public CLevelGenerateNode
{
public:
	virtual void Load( TiXmlElement* pXml, struct SLevelGenerateNodeLoadContext& context ) override;
	virtual void Generate( SLevelBuildContext& context, const TRectangle<int32>& region ) override;
private:
	void GenRooms();
	void PutHBars();
	void ConnRooms();
	void GenConnAreas();
	void GenDoors();
	void GenEmptyArea();
	void FillBlockArea();
	void GenObjects();

	CReference<CLevelGenerateNode> m_pWallNode;
	CReference<CLevelGenerateNode> m_pBlock1Node;
	CReference<CLevelGenerateNode> m_pBlock2Node;
	CReference<CLevelGenerateNode> m_pRoom1Node;
	CReference<CLevelGenerateNode> m_pRoom2Node;
	CReference<CLevelGenerateNode> m_pBar1Node;
	CReference<CLevelGenerateNode> m_pBar2Node;
	CReference<CLevelGenerateNode> m_pObjNode;

	enum
	{
		eType_None,
		eType_BlockRed,
		eType_BlockBlue,
		eType_Room1,
		eType_Room2,
		eType_Bar,
		eType_Door,
		eType_Path,
		eType_Object,

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