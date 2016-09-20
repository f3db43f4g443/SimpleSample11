#pragma once
#include "Render/RenderObject2D.h"
#include <map>
using namespace std;

class CHurtEffectMgr
{
public:
	class IFactory
	{
	public:
		virtual CRenderObject2D* Create( const CVector2& ofs ) = 0;
	};

	CHurtEffectMgr();
	void Register( const char* szEffect, IFactory* pFactory );
	CRenderObject2D* Create( const char* szEffect, const CVector2& ofs );
	
	DECLARE_GLOBAL_INST_REFERENCE( CHurtEffectMgr )
private:
	map<string, IFactory*> m_mapFactories;
};