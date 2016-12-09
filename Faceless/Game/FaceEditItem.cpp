#include "stdafx.h"
#include "FaceEditItem.h"
#include "Face.h"

CFaceEditItemErase::CFaceEditItemErase()
{
	strName = "Erase";
	nType = eFaceEditType_Common;
	nCost = 1;
	nWidth = 1;
	nHeight = 1;
}

uint32 CFaceEditItemErase::GetCost( CFace * pFace, const TVector2<int32>& pos )
{
	auto* pGrid = pFace->GetGrid( pos.x, pos.y );
	if( !pGrid && !pGrid->pOrgan )
		return 0;
	return pGrid->pOrgan->GetEditItem()->GetCost( pFace, pos );
}

bool CFaceEditItemErase::IsValidGrid( CFace * pFace, const TRectangle<int32>& editRect, const TVector2<int32>& pos )
{
	auto* pGrid = pFace->GetGrid( pos.x, pos.y );
	if( !pGrid || !pGrid->pOrgan )
		return false;
	return true;
}

void CFaceEditItemErase::Edit( CCharacter * pCharacter, CFace * pFace, const TVector2<int32>& pos )
{
	auto* pGrid = pFace->GetGrid( pos.x, pos.y );
	if( pGrid && pGrid->pOrgan )
	{
		pFace->RemoveOrgan( pGrid->pOrgan );
	}
}

const vector<CFaceEditItem*>& CFaceEditItem::GetAllCommonEditItems()
{
	static vector<CFaceEditItem*> g_items;
	if( !g_items.size() )
	{
		g_items.push_back( new CFaceEditItemErase );
	}
	return g_items;
}
