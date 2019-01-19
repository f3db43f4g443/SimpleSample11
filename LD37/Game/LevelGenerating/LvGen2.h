#pragma once
#include "LevelGenerate.h"

class CLevelGen2
{
public:
	static void GenHouse( vector<int8>& genData, int32 nWidth, int32 nHeight, const TRectangle<int32>& rect, int8 nTypeBegin, int32 la = 3, int32 lb = 5 );
	static void GenRoom( vector<int8>& genData, int32 nWidth, int32 nHeight, const TRectangle<int32>& rect, int8 nTypeBegin );
};

class CLevelGenNode2_1_0 : public CLevelGenerateNode
{
public:
	virtual void Load( TiXmlElement* pXml, struct SLevelGenerateNodeLoadContext& context ) override;
	virtual void Generate( SLevelBuildContext& context, const TRectangle<int32>& region ) override;
private:
	void GenChunks();
	void GenObj();
	void GenHouse();
	void GenWall1();

	void GenObjRect( const TRectangle<int32>& rect, int8 nDir );
	enum
	{
		eType_None,

		eType_House_0,
		eType_House_1,
		eType_House_2,
		eType_House_2_0,
		eType_House_2_1,
		eType_House_2_2,
		eType_House_2_3,
		eType_House_2_4,

		eType_Obj,
		eType_Temp,
		eType_Temp1,
	};
	
	SLevelBuildContext* m_pContext;
	TRectangle<int32> m_region;
	vector<int8> m_gendata;

	vector<TRectangle<int32> > m_vecWall1;
	vector<TRectangle<int32> > m_vecHouse;
	vector<TRectangle<int32> > m_vecCargo;
	vector<TRectangle<int32> > m_vecGarbageBin;
	vector<TRectangle<int32> > m_vecGarbageBin2;

	CReference<CLevelGenerateNode> m_pWallNode;
	CReference<CLevelGenerateNode> m_pWall1Node;
	CReference<CLevelGenerateNode> m_pHouseNode;
	CReference<CLevelGenerateNode> m_pCargoNode;
	CReference<CLevelGenerateNode> m_pGarbageBinNode;
	CReference<CLevelGenerateNode> m_pGarbageBin2Node;
};

class CLevelGenNode2_1_1 : public CLevelGenerateNode
{
public:
	virtual void Load( TiXmlElement* pXml, struct SLevelGenerateNodeLoadContext& context ) override;
	virtual void Generate( SLevelBuildContext& context, const TRectangle<int32>& region ) override;
private:
	void GenChunks();
	void GenChunks1();
	bool GenChunks2( const TRectangle<int32>& r, const TRectangle<int32>& r0, int8 nDir, bool b1 );
	TRectangle<int32> GenChunks3( const TVector2<int32>& p, int8 nDir, int32 xBegin, int32 xEnd );
	void GenRoads();
	void GenObjs();
	void GenHouse();

	void AddChunk( const TRectangle<int32>& rect, int8 nType, vector<TRectangle<int32> >* pVec );
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
	vector<TRectangle<int32> > m_vecCargo;
	vector<TRectangle<int32> > m_vecGarbageBin;
	vector<TRectangle<int32> > m_vecGarbageBin2;

	CReference<CLevelGenerateNode> m_pWallNode;
	CReference<CLevelGenerateNode> m_pWall1Node;
	CReference<CLevelGenerateNode> m_pRoadNode;
	CReference<CLevelGenerateNode> m_pHouseNode;
	CReference<CLevelGenerateNode> m_pRoomNode;
	CReference<CLevelGenerateNode> m_pCargoNode;
	CReference<CLevelGenerateNode> m_pGarbageBinNode;
	CReference<CLevelGenerateNode> m_pGarbageBin2Node;
};

class CTruckNode : public CLevelGenerateNode
{
public:
	virtual void Load( TiXmlElement* pXml, struct SLevelGenerateNodeLoadContext& context ) override;
	virtual void Generate( SLevelBuildContext& context, const TRectangle<int32>& region ) override;
private:
	CReference<CLevelGenerateNode> m_pBaseNode[2];
	CReference<CLevelGenerateNode> m_pControlRoomNode[2];
	CReference<CLevelGenerateNode> m_pCargoNode;
	CReference<CLevelGenerateNode> m_pCargo2Node;
};

class CLevelGenNode2_2_0 : public CLevelGenerateNode
{
public:
	virtual void Load( TiXmlElement* pXml, struct SLevelGenerateNodeLoadContext& context ) override;
	virtual void Generate( SLevelBuildContext& context, const TRectangle<int32>& region ) override;
private:
	void GenRoads();
	void GenRooms();
	void ConnRoads();
	void FillEmpty();
	void GenObjs();
	void GenHouses();

	enum
	{
		eType_None,
		eType_Road,

		eType_Greenbelt,
		eType_Greenbelt_1,
		eType_Greenbelt_1_1,
		eType_Greenbelt_1_2,

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

		eType_Cargo1,

		eType_Obj,
		eType_Obj1,
		eType_Temp,
		eType_Temp1,
		eType_Temp2,
	};

	SLevelBuildContext* m_pContext;
	TRectangle<int32> m_region;
	vector<int8> m_gendata;
	vector<int8> m_gendata1;
	vector<TRectangle<int32> > m_vecRoad;
	vector<TRectangle<int32> > m_vecGreenbelt;
	vector<TRectangle<int32> > m_vecHouse;
	vector<TRectangle<int32> > m_vecRoom;
	vector<TRectangle<int32> > m_vecCargo;
	vector<TRectangle<int32> > m_vecCargo1;
	vector<TVector2<int32> > m_vecBroken;
	vector<TVector2<int32> > m_vecBox;
	vector<TRectangle<int32> > m_vecBar[2];
	vector<TRectangle<int32> > m_vecBarrel[3];

	CReference<CLevelGenerateNode> m_pWallNode;
	CReference<CLevelGenerateNode> m_pRoadNode;
	CReference<CLevelGenerateNode> m_pGreenbeltNode;
	CReference<CLevelGenerateNode> m_pHouseNode;
	CReference<CLevelGenerateNode> m_pRoomNode;
	CReference<CLevelGenerateNode> m_pCargoNode;
	CReference<CLevelGenerateNode> m_pCargo1Node;
	CReference<CLevelGenerateNode> m_pBrokenNode;
	CReference<CLevelGenerateNode> m_pBoxNode;
	CReference<CLevelGenerateNode> m_pBarNode[2];
	CReference<CLevelGenerateNode> m_pBarrelNode[3];
};

class CLevelGenNode2_2_1 : public CLevelGenerateNode
{
public:
	virtual void Load( TiXmlElement* pXml, struct SLevelGenerateNodeLoadContext& context ) override;
	virtual void Generate( SLevelBuildContext& context, const TRectangle<int32>& region ) override;
private:
	void GenChunks();
	void GenHouses();
	void GenObjs();

	enum
	{
		eType_None,
		eType_Walkable_a,
		eType_Walkable_b,
		eType_Walkable_c,
		eType_Walkable_d,
		eType_Road,
		eType_Temp,
		eType_Temp1,
		eType_Chunk,
		eType_Chunk_Plant,

		eType_Room,
		eType_Door,
		eType_House,
		eType_House_1,
		eType_House_2 = eType_House_1 + 4,
		eType_House_Exit1,
		eType_House_Exit2,
	};
	
	SLevelBuildContext* m_pContext;
	TRectangle<int32> m_region;
	vector<int8> m_gendata;
	vector<int8> m_gendata1;
	vector<TVector2<int32> > m_par;

	vector<TRectangle<int32> > m_vecRoads;
	vector<TRectangle<int32> > m_vecRoads1;
	vector<TRectangle<int32> > m_vecFences;
	vector<TRectangle<int32> > m_vecFenceBlock;
	vector<TRectangle<int32> > m_vecRooms;

	vector<TRectangle<int32> > m_vecCargos;
	vector<TRectangle<int32> > m_vecControlRooms;
	vector<TRectangle<int32> > m_vecTrucks;
	vector<TRectangle<int32> > m_vecObjs;

	CReference<CLevelGenerateNode> m_pRoadNode;
	CReference<CLevelGenerateNode> m_pFenceNode;
	CReference<CLevelGenerateNode> m_pPlantNode;
	CReference<CLevelGenerateNode> m_pWalkableNodes[4];
	CReference<CLevelGenerateNode> m_pBlockNode;
	CReference<CLevelGenerateNode> m_pRoomNode;
	CReference<CLevelGenerateNode> m_pHouseNode;
	CReference<CLevelGenerateNode> m_pCargoNode;
	CReference<CLevelGenerateNode> m_pCargo2Node;
	CReference<CLevelGenerateNode> m_pCargoSmallNode;
	CReference<CLevelGenerateNode> m_pBarrelNode;
	CReference<CLevelGenerateNode> m_pBarrel1Node;
	CReference<CLevelGenerateNode> m_pControlRoomNode;
	CReference<CLevelGenerateNode> m_pTruckNode;
};

class CLevelGenNode2_2_2 : public CLevelGenerateNode
{
public:
	virtual void Load( TiXmlElement* pXml, struct SLevelGenerateNodeLoadContext& context ) override;
	virtual void Generate( SLevelBuildContext& context, const TRectangle<int32>& region ) override;
private:
	void GenPath();
	void GenObjs();

	void AddRoad( const TRectangle<int32>& r );
	void AddRoom( const TRectangle<int32>& r, bool bLeft, bool bRight, bool bTop, bool bBottom );
	void AddChunk( const TRectangle<int32>& r, vector<TRectangle<int32> >& chunks );
	void AddBigChunk( const TRectangle<int32>& r, int8 nType );
	void AddThruster( const TRectangle<int32>& r );
	void AddSupport( const TRectangle<int32>& r );

	void ProcessTempBar( const vector<TRectangle<int32> >& v );
	void ProcessTempChunks( const vector<TRectangle<int32> >& v );
	void AddThrusters( const TRectangle<int32>& r );
	void GenBigChunk( const TRectangle<int32>& r, int8 nType );

	enum
	{
		eType_None,
		eType_Road,
		eType_Blocked,
		eType_Chunk,
		eType_Chunk1,
		eType_Thruster,

		eType_Room,
		eType_Door,
	};

	SLevelBuildContext* m_pContext;
	TRectangle<int32> m_region;
	vector<int8> m_gendata;

	vector<TRectangle<int32> > m_vecChunks;
	vector<TRectangle<int32> > m_vecBars;
	vector<TRectangle<int32> > m_vecSupports;
	vector<TRectangle<int32> > m_vecRoads;
	vector<TRectangle<int32> > m_vecCargos;
	vector<TRectangle<int32> > m_vecBigChunk1;
	vector<TRectangle<int32> > m_vecBigChunk1_1;
	vector<TRectangle<int32> > m_vecBigChunk2;
	vector<TRectangle<int32> > m_vecRooms;
	vector<TRectangle<int32> > m_vecThrusters;

	CReference<CLevelGenerateNode> m_pChunkNode;
	CReference<CLevelGenerateNode> m_pBarNode;
	CReference<CLevelGenerateNode> m_pSupportANode;
	CReference<CLevelGenerateNode> m_pSupportBNode;
	CReference<CLevelGenerateNode> m_pRoadNode;
	CReference<CLevelGenerateNode> m_pCargoNode;
	CReference<CLevelGenerateNode> m_pBigChunk1Node;
	CReference<CLevelGenerateNode> m_pBigChunk2Node;
	CReference<CLevelGenerateNode> m_pRoomNode;
	CReference<CLevelGenerateNode> m_pThrusterNode;
	CReference<CLevelGenerateNode> m_pFill1Node;
	CReference<CLevelGenerateNode> m_pFill2Node;
};