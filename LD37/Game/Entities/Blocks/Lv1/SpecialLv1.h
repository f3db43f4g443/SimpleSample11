#pragma once
#include "Entities/Blocks/SpecialBlocks.h"

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

class CAirConditioner : public CChunkObject
{
	friend void RegisterGameClasses();
public:
	CAirConditioner( const SClassCreateContext& context ) : CChunkObject( context ) { SET_BASEOBJECT_ID( CAirConditioner ); }
	virtual void OnSetChunk( SChunk* pChunk, class CMyLevel* pLevel ) override;
private:
	TResourceRef<CDrawableGroup> m_pDrawable1;
	TResourceRef<CDrawableGroup> m_pDrawable2;
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