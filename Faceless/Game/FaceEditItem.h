#pragma once

enum
{
	eFaceEditType_Organ,
	eFaceEditType_Skin,
	eFaceEditType_Mask
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

	virtual bool IsValidGrid( CFace* pFace, const TRectangle<int32>& editRect, const TVector2<int32>& pos ) { return true; }
	virtual void Edit( CCharacter* pCharacter, CFace* pFace, const TVector2<int32>& pos ) {}
};