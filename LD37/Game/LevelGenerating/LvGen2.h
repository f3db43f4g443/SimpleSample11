#pragma once
#include "LevelGenerate.h"

class CLevelGenNode2_1_0 : public CLevelGenerateNode
{
public:
	virtual void Load( TiXmlElement* pXml, struct SLevelGenerateNodeLoadContext& context ) override;
	virtual void Generate( SLevelBuildContext& context, const TRectangle<int32>& region ) override;
private:
	void GenRoad();
	void GenSubArea( const TRectangle<int32>& rect );
	void GenBase();
	void GenChunks();
	enum
	{
		eType_None,
		eType_Road,
		eType_Block,
		eType_Block1,
		eType_Block2,
		eType_Obj,
		eType_Chunk,
		eType_Temp,
	};
	enum
	{
		eType1_None,
		eType1_Block,
		eType1_Obj,
	};
	
	SLevelBuildContext* m_pContext;
	TRectangle<int32> m_region;
	vector<int8> m_gendata;
	vector<int8> m_gendata1;

	struct SRoad
	{
		SRoad() {}
		SRoad( const TRectangle<int32>& rect, uint8 nType ) : rect( rect ), nType( nType ) {}
		TRectangle<int32> rect;
		uint8 nType;
	};
	vector<SRoad> m_vecRoads;
	vector<TRectangle<int32> > m_vecCargoSmall;
	vector<TRectangle<int32> > m_vecCargoLarge;
	TRectangle<int32> m_road1;

	CReference<CLevelGenerateNode> m_pWallNode;
	CReference<CLevelGenerateNode> m_pRoadNode;
	CReference<CLevelGenerateNode> m_pRoadNode1;
	CReference<CLevelGenerateNode> m_pBlockNode;
	CReference<CLevelGenerateNode> m_pBlock1Node;
	CReference<CLevelGenerateNode> m_pBlock2Node;
	CReference<CLevelGenerateNode> m_pObjNode;
	CReference<CLevelGenerateNode> m_pCargoNode;
	CReference<CLevelGenerateNode> m_pCargo2Node;
};

class CLevelGenNode2_2_0 : public CLevelGenerateNode
{
public:
	virtual void Load( TiXmlElement* pXml, struct SLevelGenerateNodeLoadContext& context ) override;
	virtual void Generate( SLevelBuildContext& context, const TRectangle<int32>& region ) override;
private:
	void GenAreas();
	enum
	{
		eType_None,
		eType_Temp0,
		eType_Temp0_0,
		eType_Temp1,
		eType_Road,

		eType_Room,
		eType_House,
		eType_House_Exit1,
		eType_House_Exit2,
	};
	enum
	{
		eType1_None,
		eType1_Block,
		eType1_Obj,
	};

	SLevelBuildContext* m_pContext;
	TRectangle<int32> m_region;
	vector<int8> m_gendata;
	vector<int8> m_gendata1;
	vector<TVector2<int32> > m_par;
	uint8 m_nType;
	vector<TRectangle<int32> > m_vecRoads;
	vector<TRectangle<int32> > m_vecArea1;
	vector<TRectangle<int32> > m_vecArea2;
	vector<TRectangle<int32> > m_vecRooms;

	struct SHouse
	{
		SHouse() {}
		SHouse( int32 x, int32 y, int32 w, int32 h ) : rect( x, y, w, h ) { memset( exit, 0, sizeof( exit ) ); memset( nExitType, 0, sizeof( nExitType ) ); }
		SHouse( const TRectangle<int32>& r ) : rect( r ) { memset( exit, 0, sizeof( exit ) ); memset( nExitType, 0, sizeof( nExitType ) ); }
		TRectangle<int32> rect;
		TVector2<int32> exit[4];
		uint8 nExitType[4];
	};

	vector<SHouse> m_vecHouses;
};