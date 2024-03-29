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
	bool m_bOfs;
};

class CRandomTileNode1 : public CLevelGenerateNode
{
public:
	virtual void Load( TiXmlElement* pXml, struct SLevelGenerateNodeLoadContext& context ) override;
	virtual void Generate( SLevelBuildContext& context, const TRectangle<int32>& region ) override;
private:
	CReference<CLevelGenerateNode> m_pNodes[16];
};

class CBarFillNode : public CLevelGenerateNode
{
public:
	virtual void Load( TiXmlElement* pXml, struct SLevelGenerateNodeLoadContext& context ) override;
	virtual void Generate( SLevelBuildContext& context, const TRectangle<int32>& region ) override;
private:
	CReference<CLevelGenerateNode> m_pNode;
	string m_strName;
	bool m_bVertical;
};

class CCommonRoomNode : public CLevelGenerateSimpleNode
{
public:
	virtual void Load( TiXmlElement* pXml, struct SLevelGenerateNodeLoadContext& context ) override;
	virtual void Generate( SLevelBuildContext& context, const TRectangle<int32>& region ) override;
private:
	CReference<CLevelGenerateNode> m_pWallBroken;
	CReference<CLevelGenerateNode> m_pWallBroken1;
};

class CRoom0Node : public CLevelGenerateSimpleNode
{
public:
	virtual void Load( TiXmlElement* pXml, struct SLevelGenerateNodeLoadContext& context ) override;
	virtual void Generate( SLevelBuildContext& context, const TRectangle<int32>& region ) override;
private:
	CReference<CLevelGenerateNode> m_pWallBroken;
	CReference<CLevelGenerateNode> m_pDoor1[4];
	CReference<CLevelGenerateNode> m_pDoor2[4];
	CReference<CLevelGenerateNode> m_pObj[4];
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
	string m_strMask;
	CReference<CLevelGenerateNode> m_pWallBroken;
	CReference<CLevelGenerateNode> m_pHBar;
	CReference<CLevelGenerateNode> m_pVBar;
	CReference<CLevelGenerateNode> m_pCorner;
	CReference<CLevelGenerateNode> m_pDoor1[4];
	CReference<CLevelGenerateNode> m_pDoor2[4];
};

class CBillboardNode : public CLevelGenerateSimpleNode
{
public:
	virtual void Load( TiXmlElement* pXml, struct SLevelGenerateNodeLoadContext& context ) override;
	virtual void Generate( SLevelBuildContext& context, const TRectangle<int32>& region ) override;
private:
	vector<CReference<CLevelGenerateNode> > m_vecSubNodes;
};

class CPipeNode : public CLevelGenerateNode
{
public:
	virtual void Load( TiXmlElement* pXml, struct SLevelGenerateNodeLoadContext& context ) override;
	virtual void Generate( SLevelBuildContext& context, const TRectangle<int32>& region ) override;
private:
	void GenPipe( TVector2<int32> beginPoint );

	CReference<CLevelGenerateNode> m_pPipes[15];
	CReference<CLevelGenerateNode> m_pNode1;

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

class CSplitNode : public CLevelGenerateNode
{
public:
	virtual void Load( TiXmlElement* pXml, struct SLevelGenerateNodeLoadContext& context ) override;
	virtual void Generate( SLevelBuildContext& context, const TRectangle<int32>& region ) override;
private:
	bool m_bVertical;
	uint32 m_nMinWidth;
	uint32 m_nMaxWidth;
	uint32 m_nSpaceWidth;
	CReference<CLevelGenerateNode> m_pSplitNode;
	CReference<CLevelGenerateNode> m_pSpaceNode;
};

class CHouseNode : public CLevelGenerateSimpleNode
{
public:
	virtual void Load( TiXmlElement* pXml, struct SLevelGenerateNodeLoadContext& context ) override;
	virtual void Generate( SLevelBuildContext& context, const TRectangle<int32>& region ) override;
private:
	CReference<CLevelGenerateNode> m_pBrokenNode;
	CReference<CLevelGenerateNode> m_pSubChunkNode;
	CReference<CLevelGenerateNode> m_pCarSpawnerNode[4];
	CReference<CLevelGenerateNode> m_pEntranceNode[4];
};

class CFenceNode : public CLevelGenerateSimpleNode
{
public:
	virtual void Load( TiXmlElement* pXml, struct SLevelGenerateNodeLoadContext& context ) override;
	virtual void Generate( SLevelBuildContext& context, const TRectangle<int32>& region ) override;
private:
	uint8 m_nType0;
	uint8 m_nType1;
	CReference<CLevelGenerateNode> m_pFenceNode;
	CReference<CLevelGenerateNode> m_pTileNode;
};


class CFiberNode : public CLevelGenerateNode
{
public:
	virtual void Load( TiXmlElement* pXml, struct SLevelGenerateNodeLoadContext& context ) override;
	virtual void Generate( SLevelBuildContext& context, const TRectangle<int32>& region ) override;
private:
	vector<string> m_vecBlockTypes;
	CReference<CLevelGenerateNode> m_pNode;
	uint8 m_nType;
};

class CControlRoomNode : public CLevelGenerateSimpleNode
{
public:
	virtual void Load( TiXmlElement* pXml, struct SLevelGenerateNodeLoadContext& context ) override;
	virtual void Generate( SLevelBuildContext& context, const TRectangle<int32>& region ) override;
private:
	CReference<CLevelGenerateNode> m_pNode0[4];
	CReference<CLevelGenerateNode> m_pNode1[8];
	CReference<CLevelGenerateNode> m_pNode2[8];
	CReference<CLevelGenerateNode> m_pNode3[4];
};