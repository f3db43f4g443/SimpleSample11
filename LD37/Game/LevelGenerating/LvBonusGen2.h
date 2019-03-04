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

class CLevelBonusGenNode2_1 : public CLevelGenerateNode
{
public:
	virtual void Load( TiXmlElement* pXml, struct SLevelGenerateNodeLoadContext& context ) override;
	virtual void Generate( SLevelBuildContext& context, const TRectangle<int32>& region ) override;
private:
	void GenChunks();
	void GenChunks1( int32 yBegin, int32 yEnd, int8 nFlag, int8 nNxtType, int8& nFlag1 );
	void GenChunks2( int32 yBegin, int32 yEnd, int8 nFlag, int8 nNxtType, int8& nFlag1 );

	void GenRoom( const TRectangle<int32>& rect, uint8 nType = 0 );
	void GenCargos( const TRectangle<int32>& rect, uint8 nType = 0 );
	void GenBars0( TRectangle<int32> rect, int8 nBaseType );
	void GenBars0Wall( TRectangle<int32> rect, const TRectangle<int32>& bound, uint8 nType = 0 );
	void AddChunk( const TRectangle<int32>& rect, int8 nType, vector<TRectangle<int32> >* pVec );
	void GenCargo( const TRectangle<int32>& rect, uint8 nType = 0 );
	void GenCargo0( const TRectangle<int32>& rect );
	enum
	{
		eType_None,
		eType_Road,

		eType_Room_1,
		eType_Room_2,
		eType_Room_Door,

		eType_Cargo1,
		eType_Cargo1_1,
		eType_Cargo1_2,
		eType_Cargo1_3,
		eType_Chunk,

		eType_Obj,
		eType_Obj1,
		eType_Obj2,
		eType_Temp,
		eType_Temp1,
		eType_Temp2,
	};

	SLevelBuildContext* m_pContext;
	TRectangle<int32> m_region;
	vector<int8> m_gendata;
	vector<int8> m_gendata1;
	vector<int8> m_gendata2;
	vector<TVector2<int32> > m_par;

	vector<TRectangle<int32> > m_vecRoads;
	vector<TRectangle<int32> > m_vecRooms;
	vector<TRectangle<int32> > m_vecCargos;
	vector<TRectangle<int32> > m_vecCargos1;
	vector<TVector2<int32> > m_vecBroken;
	vector<TVector2<int32> > m_vecBox;
	vector<TRectangle<int32> > m_vecBar0;
	vector<TRectangle<int32> > m_vecBar[2];
	vector<TRectangle<int32> > m_vecBar_a[2];
	vector<TRectangle<int32> > m_vecBar2;
	vector<TRectangle<int32> > m_vecBarrel[3];
	vector<TVector2<int32> > m_vecFuse;
	vector<TRectangle<int32> > m_vecChain;
	vector<TVector2<int32> > m_vecPt;

	CReference<CLevelGenerateNode> m_pWallNode;
	CReference<CLevelGenerateNode> m_pRoadNode;
	CReference<CLevelGenerateNode> m_pRoomNode;
	CReference<CLevelGenerateNode> m_pCargoNode;
	CReference<CLevelGenerateNode> m_pCargo1Node;
	CReference<CLevelGenerateNode> m_pBrokenNode;
	CReference<CLevelGenerateNode> m_pBoxNode;
	CReference<CLevelGenerateNode> m_pBar0Node;
	CReference<CLevelGenerateNode> m_pBarNode[2];
	CReference<CLevelGenerateNode> m_pBarNode_a[2];
	CReference<CLevelGenerateNode> m_pBar2Node;
	CReference<CLevelGenerateNode> m_pBarrelNode[3];
	CReference<CLevelGenerateNode> m_pFuseNode;
	CReference<CLevelGenerateNode> m_pChainNode;
	CReference<CLevelGenerateNode> m_pPtNode;
};