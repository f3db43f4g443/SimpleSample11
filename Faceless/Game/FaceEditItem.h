#pragma once

#include <string>
#include <vector>
#include "Common/Math3D.h"
using namespace std;

enum
{
	eFaceEditType_Common,

	eFaceEditType_Organ,
	eFaceEditType_Skin,
	eFaceEditType_Mask,
};

class CCharacter;
class CFace;
class CFaceEditItem
{
public:
	string strName;
	uint8 nType;
	uint32 nCost;
	uint32 nWidth;
	uint32 nHeight;

	virtual uint32 GetCost( CFace* pFace, const TVector2<int32>& pos ) { return nCost; }
	virtual bool IsValidGrid( CFace* pFace, const TRectangle<int32>& editRect, const TVector2<int32>& pos ) { return true; }
	virtual void Edit( CCharacter* pCharacter, CFace* pFace, const TVector2<int32>& pos ) {}

	static const vector<CFaceEditItem*>& GetAllCommonEditItems();
};

class CFaceEditItemErase : public CFaceEditItem
{
public:
	CFaceEditItemErase();
	virtual uint32 GetCost( CFace* pFace, const TVector2<int32>& pos ) override;
	virtual bool IsValidGrid( CFace* pFace, const TRectangle<int32>& editRect, const TVector2<int32>& pos ) override;
	virtual void Edit( CCharacter* pCharacter, CFace* pFace, const TVector2<int32>& pos ) override;
};