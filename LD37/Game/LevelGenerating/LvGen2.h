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
	void GenCargo1Room( const TRectangle<int32>& rect, const TRectangle<int32>& room );
	void GenObjs();

	void GenTruck1( const TRectangle<int32>& rect, vector<TRectangle<int32> >& vec1, vector<TRectangle<int32> >& vec2 );
	void GenTruck2( const TRectangle<int32>& rect );
	void GenStack( const TRectangle<int32>& rect );
	void AddChunk( const TRectangle<int32>& rect, int8 nType, vector<TRectangle<int32> >* pVec );
	void GenLimbs( const TRectangle<int32>& rect );
	void GenCargo( const TRectangle<int32>& rect );
	void GenCargo0( const TRectangle<int32>& rect );
	bool GenBar0( const TRectangle<int32>& rect );
	bool CheckBar( TRectangle<int32> rect, TRectangle<int32> r1 );
	bool CheckCargo( TRectangle<int32> rect, TRectangle<int32> r1 );

	enum
	{
		eType_None,
		eType_Road,

		eType_Room_1,
		eType_Room_2,
		eType_Room_Door,

		eType_Control_Room,
		eType_Control_Room_1,
		eType_Control_Room_2,
		eType_Control_Room_3,
		eType_Control_Room_4,
		eType_Control_Room_5,

		eType_Billboard,
		eType_Billboard_1,

		eType_Cargo1,
		eType_Cargo1_1,
		eType_Cargo1_2,
		eType_Cargo1_3,
		eType_Chunk,

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
	vector<int8> m_gendata2;
	vector<TVector2<int32> > m_par;

	vector<TRectangle<int32> > m_vecRoads;
	vector<TRectangle<int32> > m_vecRooms;
	vector<TRectangle<int32> > m_vecBillboards;
	vector<TRectangle<int32> > m_vecCargos;
	vector<TRectangle<int32> > m_vecCargos1;
	struct SControlRoom
	{
		TRectangle<int32> rect;
		uint8 b[4];
	};
	vector<SControlRoom> m_vecControlRooms;
	vector<TVector2<int32> > m_vecBroken;
	vector<TVector2<int32> > m_vecBox;
	vector<TRectangle<int32> > m_vecBar0;
	vector<TRectangle<int32> > m_vecBar[2];
	vector<TRectangle<int32> > m_vecBar_a[2];
	vector<TRectangle<int32> > m_vecBar2;
	vector<TRectangle<int32> > m_vecBarrel[5];
	vector<TVector2<int32> > m_vecFuse;
	vector<TRectangle<int32> > m_vecChain;

	CReference<CLevelGenerateNode> m_pWallNode;
	CReference<CLevelGenerateNode> m_pRoadNode;
	CReference<CLevelGenerateNode> m_pRoomNode;
	CReference<CLevelGenerateNode> m_pBillboardNode;
	CReference<CLevelGenerateNode> m_pCargoNode;
	CReference<CLevelGenerateNode> m_pCargo1Node;
	CReference<CLevelGenerateNode> m_pBrokenNode;
	CReference<CLevelGenerateNode> m_pBoxNode;
	CReference<CLevelGenerateNode> m_pBar0Node;
	CReference<CLevelGenerateNode> m_pBarNode[2];
	CReference<CLevelGenerateNode> m_pBarNode_a[2];
	CReference<CLevelGenerateNode> m_pBar2Node;
	CReference<CLevelGenerateNode> m_pBarrelNode[5];
	CReference<CLevelGenerateNode> m_pControlRoomNode;
	CReference<CLevelGenerateNode> m_pFuseNode;
	CReference<CLevelGenerateNode> m_pChainNode;
};

class CLv2WallNode : public CLevelGenerateNode
{
public:
	virtual void Load( TiXmlElement* pXml, struct SLevelGenerateNodeLoadContext& context ) override;
	virtual void Generate( SLevelBuildContext& context, const TRectangle<int32>& region ) override;
private:
	string m_strMask;
	CReference<CLevelGenerateNode> m_pNode;
	CReference<CLevelGenerateNode> m_pNode0;
	CReference<CLevelGenerateNode> m_pBar0;
};

class CLevelGenNode2_3_0 : public CLevelGenerateNode
{
public:
	virtual void Load( TiXmlElement* pXml, struct SLevelGenerateNodeLoadContext& context ) override;
	virtual void Generate( SLevelBuildContext& context, const TRectangle<int32>& region ) override;
private:
	void GenBase();
	void GenArea( const TRectangle<int32>& rect );
	void AddRooms( const TRectangle<int32>& r, TVector4<int8> doors, const TVector2<int32>& ofs, int32 nCount, bool bFlip );
	void AddConn1( const TRectangle<int32>& r );
	void AddConn2( const TRectangle<int32>& r );
	void AddChunk( const TRectangle<int32>& rect, int8 nType, vector<TRectangle<int32> >* pVec, bool b = false );
	void GenArea1( const TRectangle<int32>& rect1, const TRectangle<int32>& rect2, const TVector2<int32>& ofs, int32 nCount );
	void AddWall1( const TRectangle<int32>& rect );
	enum
	{
		eType_None,
		eType_Wall,

		eType_Wall1,
		eType_Room_1,
		eType_Room_2,
		eType_Room_Door,

		eType_Control_Room,
		eType_Control_Room_1,
		eType_Control_Room_2,
		eType_Control_Room_3,
		eType_Control_Room_4,
		eType_Control_Room_5,

		eType_Billboard,
		eType_Billboard_0_0,
		eType_Billboard_0_1,
		eType_Billboard_1,
		eType_Billboard_2,

		eType_Cargo1,
		eType_Cargo1_1,
		eType_Cargo1_2,
		eType_Cargo1_3,
		eType_Cargo1_4,
		eType_Cargo1_5,
		eType_Cargo1_6,

		eType_Cargo2,
		eType_Cargo2_1,

		eType_Building,
		eType_Building_1,
		eType_Building_2,
		eType_Building_3,
		eType_Chunk,

		eType_Obj,
		eType_Obj1,
		eType_Temp0,
		eType_Temp1,
		eType_Temp2,
		eType_Temp3,
	};

	SLevelBuildContext* m_pContext;
	TRectangle<int32> m_region;
	vector<int8> m_gendata;
	vector<int8> m_gendata1;

	vector<TRectangle<int32> > m_vecWall1;
	vector<TVector2<int32> > m_vecBlocks[4];
	vector<TRectangle<int32> > m_vecChunk1[2];
	vector<TRectangle<int32> > m_vecChunk2[2];
	vector<TRectangle<int32> > m_vecChunk3[2];
	vector<TRectangle<int32> > m_vecChunk4[2];
	vector<TRectangle<int32> > m_vecChunk5;

	vector<TRectangle<int32> > m_vecBuildings;
	vector<TRectangle<int32> > m_vecRooms;
	vector<TRectangle<int32> > m_vecBillboards;
	vector<TRectangle<int32> > m_vecCargos;
	vector<TRectangle<int32> > m_vecCargos1;
	vector<TRectangle<int32> > m_vecCargos2;
	struct SControlRoom
	{
		TRectangle<int32> rect;
		uint8 b[4];
	};
	vector<SControlRoom> m_vecControlRooms;
	vector<TVector2<int32> > m_vecBroken;
	vector<TVector2<int32> > m_vecBox;
	vector<TRectangle<int32> > m_vecBar0[4];
	vector<TRectangle<int32> > m_vecBar[2];
	vector<TRectangle<int32> > m_vecBar_a[2];
	vector<TRectangle<int32> > m_vecSawBlade[4];

	CReference<CLevelGenerateNode> m_pWallNode;
	CReference<CLevelGenerateNode> m_pWall1Node;
	CReference<CLevelGenerateNode> m_pBlockNode[4];
	CReference<CLevelGenerateNode> m_pChunkNode1[2];
	CReference<CLevelGenerateNode> m_pChunkNode2[2];
	CReference<CLevelGenerateNode> m_pChunkNode3[2];
	CReference<CLevelGenerateNode> m_pChunkNode4[2];
	CReference<CLevelGenerateNode> m_pChunkNode5;
	CReference<CLevelGenerateNode> m_pBuildingNode;

	CReference<CLevelGenerateNode> m_pRoomNode;
	CReference<CLevelGenerateNode> m_pBillboardNode;
	CReference<CLevelGenerateNode> m_pCargoNode;
	CReference<CLevelGenerateNode> m_pCargo1Node;
	CReference<CLevelGenerateNode> m_pCargo2Node;
	CReference<CLevelGenerateNode> m_pBrokenNode;
	CReference<CLevelGenerateNode> m_pBoxNode;
	CReference<CLevelGenerateNode> m_pBar0Node[4];
	CReference<CLevelGenerateNode> m_pBarNode[2];
	CReference<CLevelGenerateNode> m_pBarNode_a[2];
	CReference<CLevelGenerateNode> m_pControlRoomNode;
	CReference<CLevelGenerateNode> m_pSawBladeNode[4];
};

class CLevelGenNode2_3_1 : public CLevelGenerateNode
{
public:
	virtual void Load( TiXmlElement* pXml, struct SLevelGenerateNodeLoadContext& context ) override;
	virtual void Generate( SLevelBuildContext& context, const TRectangle<int32>& region ) override;
private:
	void GenBase();

	void GenBillboard( const TRectangle<int32>& rect );
	int32 GenTrap( const TRectangle<int32>& bar, const TRectangle<int32>& r1, int8 nDir, int8 nDir1 );
	void GenWorkshop( const TRectangle<int32>& r, const TVector2<int32>& p0 );
	void GenWorkshop1( const TRectangle<int32>& r, const TVector2<int32>& p0 );
	void GenWorkshopRooms( const TRectangle<int32>& r, const TVector2<int32>& ofs, int8 nDir, int32 nCount );
	void AddChunk( const TRectangle<int32>& rect, int8 nType, vector<TRectangle<int32> >* pVec );
	enum
	{
		eType_None,
		eType_Wall,

		eType_Wall1,
		eType_Room_1,
		eType_Room_2,
		eType_Room_Door,

		eType_Control_Room,
		eType_Control_Room_1,
		eType_Control_Room_2,
		eType_Control_Room_3,
		eType_Control_Room_4,
		eType_Control_Room_5,

		eType_Billboard,
		eType_Billboard_1,
		eType_Billboard_2,

		eType_Cargo1,
		eType_Cargo1_1,
		eType_Cargo1_2,
		eType_Cargo1_3,
		eType_Cargo1_4,
		eType_Cargo1_5,
		eType_Cargo1_6,

		eType_Cargo2,
		eType_Cargo2_1,
		eType_Cargo2_2,
		eType_Cargo2_3,
		eType_Cargo2_4,
		eType_Chunk,

		eType_Obj,
		eType_Obj1,
		eType_Temp0,
		eType_Temp1,
		eType_Temp2,
		eType_Temp3,
	};

	SLevelBuildContext* m_pContext;
	TRectangle<int32> m_region;
	vector<int8> m_gendata;

	vector<TVector2<int32> > m_vecBlocks[4];
	vector<TRectangle<int32> > m_vecChunk1[2];
	vector<TRectangle<int32> > m_vecChunk2[2];
	vector<TRectangle<int32> > m_vecChunk3[2];
	vector<TRectangle<int32> > m_vecChunk4[2];

	vector<TRectangle<int32> > m_vecBuildings;
	vector<TRectangle<int32> > m_vecRooms;
	vector<TRectangle<int32> > m_vecBillboards;
	vector<TRectangle<int32> > m_vecCargos;
	vector<TRectangle<int32> > m_vecCargos1;
	vector<TRectangle<int32> > m_vecCargos2;
	struct SControlRoom
	{
		TRectangle<int32> rect;
		uint8 b[4];
	};
	vector<SControlRoom> m_vecControlRooms;
	vector<TVector2<int32> > m_vecBroken;
	vector<TVector2<int32> > m_vecBox;
	vector<TRectangle<int32> > m_vecBar0[3];
	vector<TRectangle<int32> > m_vecBar[2];
	vector<TRectangle<int32> > m_vecBar_a[2];
	vector<TRectangle<int32> > m_vecSawBlade[4];

	CReference<CLevelGenerateNode> m_pWallNode;
	CReference<CLevelGenerateNode> m_pBlockNode[4];
	CReference<CLevelGenerateNode> m_pChunkNode1[2];
	CReference<CLevelGenerateNode> m_pChunkNode2[2];
	CReference<CLevelGenerateNode> m_pChunkNode3[2];
	CReference<CLevelGenerateNode> m_pChunkNode4[2];
	CReference<CLevelGenerateNode> m_pBuildingNode;

	CReference<CLevelGenerateNode> m_pRoomNode;
	CReference<CLevelGenerateNode> m_pBillboardNode;
	CReference<CLevelGenerateNode> m_pCargoNode;
	CReference<CLevelGenerateNode> m_pCargo1Node;
	CReference<CLevelGenerateNode> m_pCargo2Node;
	CReference<CLevelGenerateNode> m_pBrokenNode;
	CReference<CLevelGenerateNode> m_pBoxNode;
	CReference<CLevelGenerateNode> m_pBar0Node[3];
	CReference<CLevelGenerateNode> m_pBarNode[2];
	CReference<CLevelGenerateNode> m_pBarNode_a[2];
	CReference<CLevelGenerateNode> m_pControlRoomNode;
	CReference<CLevelGenerateNode> m_pSawBladeNode[4];

	vector<pair<TRectangle<int32>, TVector2<int32> > > m_vecTempWorkshop1;
};