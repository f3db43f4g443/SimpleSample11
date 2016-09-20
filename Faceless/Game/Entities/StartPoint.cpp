#include "stdafx.h"
#include "StartPoint.h"
#include "Stage.h"

void CStartPoint::OnAddedToStage()
{
	GetStage()->AddStartPoint( this );
}