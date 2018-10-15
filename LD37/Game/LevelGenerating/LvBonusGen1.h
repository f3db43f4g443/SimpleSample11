#pragma once
#include "LevelGenerate.h"


class CLevelBonusGenNode1_0 : public CLevelGenerateNode
{
public:
	virtual void Load( TiXmlElement* pXml, struct SLevelGenerateNodeLoadContext& context ) override;
	virtual void Generate( SLevelBuildContext& context, const TRectangle<int32>& region ) override;
private:
	void GenChunks();
	void GenObjs();

	void AddRect( uint8 nType, vector<TRectangle<int32> >& vec, const TRectangle<int32>& rect );
	void FillStone( const TRectangle<int32>& rect );

	enum
	{
		eType_None,
		eType_WallChunk,
		eType_Web,
		eType_Temp,

		eType_Bar,
		eType_Stone,
		eType_Block1x,
		eType_Block2x,
		eType_Obj,
		eType_Bonus,
		eType_Temp1,
	};

	int32 m_nSeed;
	SLevelBuildContext* m_pContext;
	TRectangle<int32> m_region;
	vector<int8> m_gendata;

	vector<TRectangle<int32> > m_bars;
	vector<TRectangle<int32> > m_stones;
	vector<TRectangle<int32> > m_wallChunks;
	vector<TRectangle<int32> > m_wallChunkBonuses;

	CReference<CLevelGenerateNode> m_pWallNode;
	CReference<CLevelGenerateNode> m_pStoneNode;
	CReference<CLevelGenerateNode> m_pBlock1xNode;
	CReference<CLevelGenerateNode> m_pBlock2xNode;
	CReference<CLevelGenerateNode> m_pBarNode;
	CReference<CLevelGenerateNode> m_pWallChunkNode;
	CReference<CLevelGenerateNode> m_pObjNode;
	CReference<CLevelGenerateNode> m_pBonusNode;
	CReference<CLevelGenerateNode> m_pBonusPickUpNode;
};

class CLevelBonusGenNode1_1 : public CLevelGenerateNode
{
public:
	virtual void Load( TiXmlElement* pXml, struct SLevelGenerateNodeLoadContext& context ) override;
	virtual void Generate( SLevelBuildContext& context, const TRectangle<int32>& region ) override;
private:
	void GenRooms();
	void FixBlocks();

	void AddRect( uint8 nType, vector<TRectangle<int32> >& vec, const TRectangle<int32>& rect );
	void AddRoom( uint8 nType, const TRectangle<int32>& rect );
	void FillRoomGap( TRectangle<int32> rect, bool bUp, bool bDown, bool bCheckDoor = true );
	void CopyData( const TRectangle<int32>& dst, const TVector2<int32>& src, bool bFlipX );

	enum
	{
		eType_None,
		eType_Door,
		eType_WallChunk,
		eType_Temp,

		eType_Bar,
		eType_Stone,
		eType_Room,
		eType_Block1x,
		eType_Block2x,
		eType_Web,
		eType_Obj,
		eType_Obj_Max = eType_Obj + 3,
		eType_Bonus = eType_Obj_Max,
		eType_Temp1,
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
	vector<TRectangle<int32> > m_bars;
	vector<TRectangle<int32> > m_stones;
	vector<TRectangle<int32> > m_wallChunks;

	CReference<CLevelGenerateNode> m_pWallNode;
	CReference<CLevelGenerateNode> m_pStoneNode;
	CReference<CLevelGenerateNode> m_pBlock1xNode;
	CReference<CLevelGenerateNode> m_pBlock2xNode;
	CReference<CLevelGenerateNode> m_pBarNode;
	CReference<CLevelGenerateNode> m_pBar2Node;
	CReference<CLevelGenerateNode> m_pRoom1Node;
	CReference<CLevelGenerateNode> m_pRoom2Node;
	CReference<CLevelGenerateNode> m_pWallChunkNode;
	CReference<CLevelGenerateNode> m_pObjNode[3];
	CReference<CLevelGenerateNode> m_pBonusNode;
	CReference<CLevelGenerateNode> m_pBonusPickUpNode;
	CReference<CLevelGenerateNode> m_pWebNode;
};

class CLevelBonusGenNode1_2 : public CLevelGenerateNode
{
public:
	virtual void Load( TiXmlElement* pXml, struct SLevelGenerateNodeLoadContext& context ) override;
	virtual void Generate( SLevelBuildContext& context, const TRectangle<int32>& region ) override;

private:
	void GenPath();
	void GenChunks();
	void GenObjs();

	void GenWallChunk0( const TRectangle<int32>& rect );
	void GenRoom( const TRectangle<int32>& rect );
	void GenWallChunk( const TRectangle<int32>& rect );

	void GenWallChunk0Rect( const TRectangle<int32>& rect, bool bVertical, bool bSplit = false );
	void GenWindows( const TRectangle<int32>& rect, vector<int8>& data1 );

	CReference<CLevelGenerateNode> m_pWallNode;
	CReference<CLevelGenerateNode> m_pWallChunkNode;
	CReference<CLevelGenerateNode> m_pWallChunk0Node;
	CReference<CLevelGenerateNode> m_pWallChunk1Node;
	CReference<CLevelGenerateNode> m_pBlock1Node;
	CReference<CLevelGenerateNode> m_pBlock2Node;
	CReference<CLevelGenerateNode> m_pRoomNode;
	CReference<CLevelGenerateNode> m_pCrate1Node;
	CReference<CLevelGenerateNode> m_pScrapNode;
	CReference<CLevelGenerateNode> m_pWindow1Node[4];
	CReference<CLevelGenerateNode> m_pBonusPickUpNode;

	enum
	{
		eType_None,
		eType_Temp,
		eType_Wall,
		eType_Room,
		eType_Room1,
		eType_Door,
		eType_WallChunk0,
		eType_WallChunk0_0,
		eType_WallChunk0_0x,
		eType_WallChunk0_0y,
		eType_WallChunk,
		eType_WallChunk1,
		eType_WallChunk1_1,
		eType_WallChunk1_2,
		eType_WallChunk2,
		eType_BlockRed,
		eType_BlockBlue,
		eType_Crate,
		eType_Web,

		eType_WallChunk_0,
		eType_WallChunk_0_1,
		eType_WallChunk_0_1a,
		eType_WallChunk_0_1b,
		eType_WallChunk_1,
		eType_WallChunk_2,
		eType_WallChunk_3,
		eType_Temp1,
		eType_Temp2,

		eType_Count,
	};
	SLevelBuildContext* m_pContext;
	TRectangle<int32> m_region;
	vector<int8> m_gendata;

	vector<TRectangle<int32> > m_rooms;
	vector<TRectangle<int32> > m_wallChunks0;
	vector<TRectangle<int32> > m_wallChunks;
	vector<TRectangle<int32> > m_wallChunks1;
	vector<TRectangle<int32> > m_wallChunks1_1;
	vector<TRectangle<int32> > m_scraps;
	vector<TRectangle<int32> > m_windows1[4];
	vector<TRectangle<int32> > m_wallChunkBonuses;
};