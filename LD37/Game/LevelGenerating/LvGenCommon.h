#pragma once
#include "LevelGenerate.h"

class CBrickTileNode : public CLevelGenerateNode
{
public:
	virtual void Load( TiXmlElement* pXml, struct SLevelGenerateNodeLoadContext& context ) override;
	virtual void Generate( SLevelBuildContext& context, const TRectangle<int32>& region ) override;
private:
	CReference<CLevelGenerateNode> m_pBrick;
	CReference<CLevelGenerateNode> m_pBrick1;
	bool m_bVertical;
};

class CRoom1Node : public CLevelGenerateSimpleNode
{
public:
	virtual void Load( TiXmlElement* pXml, struct SLevelGenerateNodeLoadContext& context ) override;
	virtual void Generate( SLevelBuildContext& context, const TRectangle<int32>& region ) override;
private:
	CReference<CLevelGenerateNode> m_pWallBroken;
	CReference<CLevelGenerateNode> m_pHBar;
	CReference<CLevelGenerateNode> m_pVBar;
	CReference<CLevelGenerateNode> m_pDoor1[4];
	CReference<CLevelGenerateNode> m_pDoor2[4];
};

class CRoom2Node : public CLevelGenerateSimpleNode
{
public:
	virtual void Load( TiXmlElement* pXml, struct SLevelGenerateNodeLoadContext& context ) override;
	virtual void Generate( SLevelBuildContext& context, const TRectangle<int32>& region ) override;
private:
	CReference<CLevelGenerateNode> m_pWallBroken;
	CReference<CLevelGenerateNode> m_pHBar;
	CReference<CLevelGenerateNode> m_pVBar;
	CReference<CLevelGenerateNode> m_pCorner;
	CReference<CLevelGenerateNode> m_pDoor1[4];
	CReference<CLevelGenerateNode> m_pDoor2[4];
};

class CPipeNode : public CLevelGenerateNode
{
public:
	virtual void Load( TiXmlElement* pXml, struct SLevelGenerateNodeLoadContext& context ) override;
	virtual void Generate( SLevelBuildContext& context, const TRectangle<int32>& region ) override;
private:
	void GenPipe( TVector2<int32> beginPoint );

	CReference<CLevelGenerateNode> m_pPipes[15];

	float m_fBeginPointCountPercent;
	float m_fBeginPointHeightPercent;
	float m_fEndPointHeightPercent;
	uint32 m_nBeginClipLen;
	uint32 m_nMinHLength, m_nMaxHLength;
	uint32 m_nMinVLength, m_nMaxVLength;
	float m_fIntersectStopChance;

	SLevelBuildContext* m_pContext;
	TRectangle<int32> m_region;
	vector<int8> m_gendata;
};