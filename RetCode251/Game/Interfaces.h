#pragma once
#include "Common/Math3D.h"

class ILevelObjLayer
{
public:
	virtual bool GetBound( CRectangle& rect ) const = 0;
	virtual void InitFromTemplate( class CEntity* p, const CRectangle& rect ) = 0;
};

class IEditorTiled
{
public:
	virtual CVector2 GetTileSize() const = 0;
	virtual TVector2<int32> GetSize() const = 0;
	virtual TVector2<int32> GetMinSize() const = 0;
	virtual CVector2 GetBaseOfs() const = 0;
	virtual void Resize( const TRectangle<int32>& size ) = 0;
	virtual void SetBaseOfs( const CVector2& ofs ) = 0;
};

class IAttackEft
{
public:
	virtual void AddInitHit( class CEntity* pHitEntity, const CVector2& hitPos, const CVector2& hitDir ) = 0;
};


struct SGrabDesc
{
	CVector2 grabOfs;
	float fDropThreshold;
	int8 nGrabDir;
	enum
	{
		eDetachType_None,
		eDetachType_Release,
		eDetachType_Walljump,
	};
	int8 nDetachType;

	bool bAttached;
};

class IGrabbable
{
public:
	virtual bool CheckGrab( class CPlayer* pPlayer, SGrabDesc& desc ) = 0;
	virtual void OnAttached( class CPlayer* pPlayer ) = 0;
	virtual void OnDetached( class CPlayer* pPlayer ) = 0;
};

class IBotModule
{
public:
	virtual void InitModule() = 0;
	virtual void UpdateModule( bool bActivated ) = 0;
};