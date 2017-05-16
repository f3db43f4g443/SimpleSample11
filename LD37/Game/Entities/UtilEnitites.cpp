#include "stdafx.h"
#include "UtilEntities.h"
#include "Render/Image2D.h"
#include "Common/Rand.h"
#include "Stage.h"

void CTexRectRandomModifier::OnAddedToStage()
{
	auto pImage2D = static_cast<CImage2D*>( GetParentEntity()->GetRenderObject() );
	CRectangle texRect = pImage2D->GetElem().texRect;

	uint32 nCol = SRand::Inst().Rand( 0u, m_nCols );
	uint32 nRow = SRand::Inst().Rand( 0u, m_nRows );
	texRect = texRect.Offset( CVector2( nCol * m_fWidth, nRow * m_fHeight ) );

	pImage2D->SetTexRect( texRect );
	GetStage()->RegisterAfterHitTest( 1, &m_onTick );
}