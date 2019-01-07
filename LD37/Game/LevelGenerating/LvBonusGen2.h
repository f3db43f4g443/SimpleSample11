#pragma once
#include "LevelGenerate.h"


class CLevelBonusGenNode2_0 : public CLevelGenerateNode
{
public:
	virtual void Load( TiXmlElement* pXml, struct SLevelGenerateNodeLoadContext& context ) override;
	virtual void Generate( SLevelBuildContext& context, const TRectangle<int32>& region ) override;
private:
	void GenAreas();
	void GenAreas1( int32 y, int32 y1, int32 xBegin1, int32 xEnd1, int32& xBegin2, int32& xEnd2 );
	void GenAreas2( int32 y, int32 y1, int32& xBegin2, int32& xEnd2 );
	void AddCargos0( const TRectangle<int32>& rect );
	void AddCargos( const TRectangle<int32>& rect, int8 nType, int8 nType1 );
	void AddChunk( const TRectangle<int32>& rect, int8 nType, vector<TRectangle<int32> >* pVec );
	int32 CreateCarPath( const TRectangle<int32>& car, int8 nDir, int32 y, int32 h1, bool bType );
	void GenObj( int32 y, int32 y1 );
	void GenObjRect( const TRectangle<int32>& rect, int8 nDir, int32 y, int32 y1 );
	void GenSplitRoad( const TRectangle<int32>& rect, int32 xBegin1, int32 xEnd1, int8 nType, int8 nType1 );

	void GenHouses();
	enum
	{
		eType_None,
		eType_Wall1,

		eType_Road,
		eType_Road_1,
		eType_Road_1_1,
		eType_Road_1_2,

		eType_House_0,
		eType_House_1,
		eType_House_2,
		eType_House_2_0,
		eType_House_2_1,
		eType_House_2_2,
		eType_House_2_3,
		eType_House_2_4,

		eType_Room,
		eType_Room_1,
		eType_Room_2,
		eType_Room_Door,
		eType_Room_Car_0,
		eType_Room_Car_2,
		eType_Room_Car_3,

		eType_Obj,
		eType_Temp,
		eType_Temp1,
	};

	SLevelBuildContext* m_pContext;
	TRectangle<int32> m_region;
	vector<int8> m_gendata;
	vector<TRectangle<int32> > m_vecWall1;
	vector<TRectangle<int32> > m_vecRoad;
	vector<TRectangle<int32> > m_vecHouse;
	vector<TRectangle<int32> > m_vecRoom;
	vector<TRectangle<int32> > m_vecRoom1;
	vector<TRectangle<int32> > m_vecCargo;

	CReference<CLevelGenerateNode> m_pWallNode;
	CReference<CLevelGenerateNode> m_pWall1Node;
	CReference<CLevelGenerateNode> m_pRoadNode;
	CReference<CLevelGenerateNode> m_pHouseNode;
	CReference<CLevelGenerateNode> m_pRoomNode;
	CReference<CLevelGenerateNode> m_pCargoNode;
};
