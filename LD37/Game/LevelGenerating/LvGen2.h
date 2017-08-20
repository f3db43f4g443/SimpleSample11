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