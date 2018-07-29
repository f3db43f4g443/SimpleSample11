#pragma once
#include "Entities/Blocks/SpecialBlocks.h"
#include "Entities/AIObject.h"

class CGarbageBinRed : public CTriggerChunk
{
	friend void RegisterGameClasses();
public:
	CGarbageBinRed( const SClassCreateContext& context ) : CTriggerChunk( context ) { SET_BASEOBJECT_ID( CGarbageBinRed ); }
	virtual void Trigger() override;
private:
	uint32 m_nBulletCount;
	float m_fMinSpeed, m_fMaxSpeed;
	float m_fShake;
};

class CGarbageBinYellow : public CTriggerChunk
{
	friend void RegisterGameClasses();
public:
	CGarbageBinYellow( const SClassCreateContext& context ) : CTriggerChunk( context ) { SET_BASEOBJECT_ID( CGarbageBinYellow ); }
	virtual void Trigger() override;
private:
	float m_fShake;
};

class CGarbageBinGreen : public CTriggerChunk
{
	friend void RegisterGameClasses();
public:
	CGarbageBinGreen( const SClassCreateContext& context ) : CTriggerChunk( context ) { SET_BASEOBJECT_ID( CGarbageBinGreen ); }
	virtual void Trigger() override;
private:
	uint32 m_nBulletCount;
	float m_fMinSpeed, m_fMaxSpeed;
	float m_fGravity;
	uint32 m_nLife;
	float m_fDamage;
	float m_fShake;
};

class CGarbageBinBlack : public CTriggerChunk
{
	friend void RegisterGameClasses();
public:
	CGarbageBinBlack( const SClassCreateContext& context ) : CTriggerChunk( context ) { SET_BASEOBJECT_ID( CGarbageBinBlack ); }
	virtual void Trigger() override;
private:
	uint32 m_nCount;
	bool m_bSetAngle;
	float m_fMinSpeed, m_fMaxSpeed;
	float m_fShake;
};

class CHouse0 : public CChunkObject
{
	friend void RegisterGameClasses();
public:
	CHouse0( const SClassCreateContext& context ) : CChunkObject( context ) { SET_BASEOBJECT_ID( CHouse0 ); }
	virtual void OnSetChunk( SChunk* pChunk, class CMyLevel* pLevel ) override;
	virtual void OnCreateComplete( class CMyLevel* pLevel ) override;
	virtual bool Damage( SDamageContext& context ) override;
private:
	CRectangle m_texRectPipe;
	CRectangle m_texRectPipe1;
	uint8 m_nTag0;
	uint8 m_nTag1;
	uint8 m_nPipeTag;
	uint8 m_nPipe1Tag;
	uint32 m_nHpPerSize;
	vector<CReference<CChunkObject> > m_vecSubChunk;
};

class CRoad0 : public CChunkObject
{
	friend void RegisterGameClasses();
public:
	CRoad0( const SClassCreateContext& context ) : CChunkObject( context ) { SET_BASEOBJECT_ID( CRoad0 ); }
	virtual void OnSetChunk( SChunk* pChunk, class CMyLevel* pLevel ) override;
private:
	uint32 m_nHpPerSize;
};

class CAirConditioner : public CChunkObject
{
	friend void RegisterGameClasses();
public:
	CAirConditioner( const SClassCreateContext& context ) : CChunkObject( context ) { SET_BASEOBJECT_ID( CAirConditioner ); }
	virtual void OnSetChunk( SChunk* pChunk, class CMyLevel* pLevel ) override;
	virtual const CMatrix2D& GetTransform( uint16 nIndex ) override;
	virtual void OnTransformUpdated() override;
	virtual void OnRemovedFromStage() override;
private:
	void AIFunc();
	void AIFunc1();
	void AIFunc2();
	void AIFunc3();
	void AIOnTick();
	CVector2 GetCenter();
	class AI : public CAIObject
	{
	protected:
		virtual void AIFunc() override { static_cast<CAirConditioner*>( GetParentEntity() )->AIFunc(); }
	};
	AI* m_pAI;

	TResourceRef<CDrawableGroup> m_pDrawable1;
	TResourceRef<CDrawableGroup> m_pDrawable2;

	uint8 m_n1;
	bool m_bDirty;
	bool m_bLeft;
	vector<CMatrix2D> m_vecTrans;
	vector<CMatrix2D> m_vecGlobalTrans;
	vector<CReference<CEntity> > m_vec;
	vector<uint32> m_vecFree;
};

class CScrap : public CChunkObject
{
	friend void RegisterGameClasses();
public:
	CScrap( const SClassCreateContext& context ) : CChunkObject( context ) { SET_BASEOBJECT_ID( CScrap ); }
	virtual void OnSetChunk( SChunk* pChunk, class CMyLevel* pLevel ) override;
private:
	TResourceRef<CDrawableGroup> m_pDrawable1;
	uint32 m_nHpPerSize;
};