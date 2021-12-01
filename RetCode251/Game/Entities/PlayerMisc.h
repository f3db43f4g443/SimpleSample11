#pragma once
#include "Character.h"

class CKick : public CCharacter
{
	friend void RegisterGameClasses_PlayerMisc();
public:
	CKick( const SClassCreateContext& context ) : CCharacter( context ) { SET_BASEOBJECT_ID( CKick ); }
	virtual void OnAddedToStage() override;

	enum
	{
		eType_0,
		eType_1,
		eType_2,

		eType_3,

		eType_Count,
	};
	void SetKickType( int8 nType ) { m_nKickType = nType; }
	void OnHit( CEntity* pEntity );
	void Extent( int32 nTime ) { m_nExtentTime = nTime; }
	void Morph( CEntity* pEntity );
	void Cancel();
protected:
	virtual void OnTickAfterHitTest() override;
	void HitTest( const CRectangle& rect, const CMatrix2D& g, float fDmg );
private:
	void UpdateImage();
	void OnFirstHit();
	float m_fDamage0[eType_Count];
	float m_fHitForce[eType_Count];
	int32 m_nDeathTime;
	int32 m_nHitFrames;
	CRectangle m_hitBegin, m_hitDelta;
	int32 m_nReleaseFrame;
	TResourceRef<CPrefab> m_pDmgEft;

	int32 m_nAnimTick;
	int32 m_nTick;
	int32 m_nDeathTime0;
	bool m_bHit;
	int8 m_nKickType;
	int32 m_nExtentTime;
	CRectangle m_hitRect0;
	CRectangle m_hitRect1;
	CRectangle m_hitRect;
	CVector2 m_lastGlobalPos;
	CRectangle m_origImgRect;
	CRectangle m_origImgTexRect;
	struct SHit
	{
		SHit() : n( 0 ), hitPos( 0, 0 ), hitDir( 0, 0 ) {}
		int32 n;
		CVector2 hitPos;
		CVector2 hitDir;
	};
	map<CReference<CEntity>, SHit> m_hit;
};