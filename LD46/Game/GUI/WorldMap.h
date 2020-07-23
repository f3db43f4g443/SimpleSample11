#pragma once
#include "Entity.h"

class CWorldMap : public CEntity
{
	friend void RegisterGameClasses_WorldMap();
public:
	CWorldMap( const SClassCreateContext& context ) : CEntity( context ) { SET_BASEOBJECT_ID( CWorldMap ); }
	virtual void OnAddedToStage() override;
	void SetRegion( int32 nRegion );
	void OnCurLevelChanged();
	void SetViewArea( const CRectangle& rect );
	void AutoViewArea();
	void Move( const CVector2& ofs );
	void Scale( int32 nScale );
	void PickConsole();
	const char* GetPickedConsole( TVector2<int32>& ofs );
	virtual void Render( CRenderContext2D& context ) override;
private:
	enum
	{
		eMapElemType_Console,
		eMapElemType_Fall,
		eMapElemType_Climb,
		eMapElemType_NxtEft,
		eMapElemType_CurEft,
		eMapElemType_VisitEft,
		eMapElemType_Count,
	};
	CRectangle m_clipRect;
	CRectangle m_iconTexRect[eMapElemType_Count];

	struct SMapElem
	{
		bool bVisible;
		CElement2D elem;
		CRectangle rect0, texRect0;
		int32 nRegion;
		int32 nLevel;
		TVector2<int32> ofs;
	};
	SMapElem& AddElem( const CVector2& p, int8 nType, int32 nRegion, int32 nLevel, const TVector2<int32>& ofs );
	CRectangle m_viewArea;
	int32 m_nCurRegion;
	CReference<CEntity> m_pMap;
	CRectangle m_mapRect0, m_mapTexRect0;
	vector<SMapElem> m_vecMapElems[eMapElemType_Count];
	int32 m_nPickedConsole;
};

class CWorldMapUI : public CEntity
{
	friend void RegisterGameClasses_WorldMap();
public:
	CWorldMapUI( const SClassCreateContext& context ) : CEntity( context ) { SET_BASEOBJECT_ID( CWorldMapUI ); }

	void Show( int8 nType );
	void Update();
	int8 GetShowType() { return m_nShowType; }
	const char* GetPickedConsole( TVector2<int32>& ofs ) { return m_pMap->GetPickedConsole( ofs ); }
private:
	CReference<CWorldMap> m_pMap;
	CReference<CEntity> m_pNoData;
	TArray<int32> m_arrScale;

	int32 m_nCurRegion;
	int32 m_nScale;
	int8 m_nShowType;
};