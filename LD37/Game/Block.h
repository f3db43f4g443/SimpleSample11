#pragma once
#include "Entity.h"
#include "Common/StringUtil.h"
#include "Render/DrawableGroup.h"

enum EBlockType
{
	eBlockType_Wall,
	eBlockType_Ladder,
	eBlockType_LowBlock,
	eBlockType_Door,
	eBlockType_Block,

	eBlockType_Count,
};

struct SBlockBaseInfo
{
	EBlockType eBlockType;
	uint8 nTag;
	uint8 bImmuneToBlockBuff;
	float fDmgPercent;
};

struct SBlockLayer
{
	struct SBlock* pParent;
	LINK_LIST( SBlockLayer, BlockLayer )

	SBlockLayer* GetPrev( SBlockLayer* &pHead ) { return *__pPrevBlockLayer == pHead? (SBlockLayer*)( (uint8*)__pPrevBlockLayer - ( (uint8*)(&__pNextBlockLayer)- (uint8*)this ) ) : NULL; }
};

struct SBlock
{
	SBlock() : eBlockType( eBlockType_Wall ), nTag( 0 ), fDmgPercent( 1 ), pOwner( NULL ), nX( 0 ), nY( 0 ), nUpperMargin( 0 ), nLowerMargin( 0 ), rtTexRect( 0, 0, 0, 0 ), bImmuneToBlockBuff( 0 )
	{ layers[0].pParent = layers[1].pParent = this; }
	struct SChunk* pOwner;
	float fDmgPercent;
	uint8 eBlockType;
	uint8 nTag;
	uint8 nX, nY;
	uint8 nUpperMargin, nLowerMargin;
	uint8 nAttachType;

	uint8 bImmuneToBlockBuff : 1;

	CReference<CEntity> pEntity;

	CRectangle rtTexRect;

	enum
	{
		eAttachedPrefab_Center,
		eAttachedPrefab_Right,
		eAttachedPrefab_Upper,
		eAttachedPrefab_Left,
		eAttachedPrefab_Lower,

		eAttachedPrefab_Count
	};

	CReference<CPrefab> pAttachedPrefab[eAttachedPrefab_Count];
	TVector2<int32> attachedPrefabSize;
	SBlockLayer layers[2];
};

class CBlockObject : public CEntity
{
	friend class CMyLevel;
	friend class CChunkObject;
	friend class CBlockDetectUI;
public:
	CBlockObject( const SClassCreateContext& context ) : CEntity( context ), m_nBlockRTIndex( -1 ), m_bBlockRTActive( false ) { SET_BASEOBJECT_ID( CBlockObject ); }
	CBlockObject( SBlock* pBlock, CEntity* pParent, class CMyLevel* pLevel );
	CBlockObject( SBlock* pBlock, CEntity* pParent, uint32 nSize );
	SBlock* GetBlock() { return m_pBlock; }

	virtual void OnRemovedFromStage() override;
	void ClearBuffs();
	void ClearEfts();
private:
	SBlock* m_pBlock;
	int16 m_nBlockRTIndex;

	CReference<CRenderObject2D> m_pBlockRTObject;
	CReference<CRenderObject2D> m_pDetectUI;
	bool m_bBlockRTActive;
};

class CChunkStopEvent : public CReferenceObject
{
public:
	int32 nHeight;
	function<void( struct SChunk* )> func;
	function<void( struct SChunk* )> killedFunc;

	LINK_LIST_REF( CChunkStopEvent, StopEvent )
};

struct SChunkBaseInfo
{
	uint32 nWidth;
	uint32 nHeight;
	float fWeight;
	float fDestroyWeight;
	float fDestroyBalance;
	float fImbalanceTime;
	float fShakeDmg;
	float fShakeDmgPerWidth;
	uint32 nShakeDmgThreshold;
	uint32 nAbsorbShakeStrength;
	uint32 nDestroyShake;

	float fWeightPerWidth;
	float fDestroyWeightPerWidth;
	float fAbsorbShakeStrengthPerHeight;

	uint8 nLayerType;
	uint8 bIsRoom;
	uint8 nMoveType;
	uint8 nSubChunkType;
	uint8 nShowLevelType;

	vector<SBlockBaseInfo> blockInfos;
	CReference<CPrefab> pPrefab;

	bool HasLayer( uint8 i ) { return !!( nLayerType & ( 1 << i ) ); }
};

struct SChunkSpawnInfo
{
	CReference<CPrefab> pPrefab;
	CRectangle rect;
	float r;

	LINK_LIST( SChunkSpawnInfo, SpawnInfo )
};

struct SChunk
{
	SChunk() { memset( this, 0, sizeof( SChunk ) ); }
	SChunk( const SChunkBaseInfo& baseInfo, const TVector2<int32>& pos, const TVector2<int32>& size );
	~SChunk();
	uint32 nWidth;
	uint32 nHeight;
	float fWeight;
	float fDestroyWeight;
	float fDestroyBalance;
	float fImbalanceTime;
	float fShakeDmg;
	uint32 nShakeDmgThreshold;
	uint32 nAbsorbShakeStrength;
	uint32 nDestroyShake;

	TVector2<int32> pos;
	vector<SBlock> blocks;
	CReference<CPrefab> pPrefab;
	class CChunkObject* pChunkObject;

	uint32 nFallSpeed;
	uint32 nUpdateCount;
	float fAppliedWeight;
	float fBalance;
	float fCurImbalanceTime;
	uint32 nCurShakeStrength;

	SBlock* GetBlock( uint32 x, uint32 y ) { return x < nWidth && y < nHeight ? &blocks[x + y * nWidth] : NULL; }
	bool CreateChunkObject( class CMyLevel* pLevel, SChunk* pParent = NULL );
	void CreateChunkObjectPreview( CEntity* pRootEntity, SChunk* pParent = NULL );
	float GetFallSpeed();
	bool HasLayer( uint8 i ) { return !!( nLayerType & ( 1 << i ) ); }
	uint8 GetMinLayer() { return nLayerType == 2 ? 1 : 0; }
	uint8 GetMaxLayer() { return nLayerType == 1 ? 1 : 2; }

	bool IsFullyUpdated() { return nUpdateCount == ( nLayerType == 3 ? nWidth * 2 : nWidth ); }

	void ForceDestroy();

	uint8 bStopMove : 1;
	uint8 bForceStop : 1;
	uint8 bSpawned : 1;
	uint8 bIsSubChunk : 1;
	uint8 bMovedLastFrame : 1;
	uint8 bIsBeingRepaired : 1;
	uint8 nVisitFlag : 1;
	uint8 nShowLevelType : 1;

	uint8 bInvulnerable : 1;
	uint8 bIsRoom : 1;
	uint8 nMoveType : 2;
	uint8 nLayerType : 2;
	uint8 nSubChunkType : 2;

	uint8 nLevelBarrierType : 2;
	uint8 nBarrierHeight : 4;

	SChunk* pParentChunk;
	LINK_LIST( SChunk, SubChunk )
	LINK_LIST_HEAD( m_pSpawnInfos, SChunkSpawnInfo, SpawnInfo )
	LINK_LIST_HEAD( m_pSubChunks, SChunk, SubChunk )
	LINK_LIST_REF_HEAD( m_pChunkStopEvents, CChunkStopEvent, StopEvent )
};

class CChunkObject : public CEntity
{
	friend class CMyLevel;
	friend struct SChunk;
	friend void RegisterGameClasses();
public:
	enum
	{
		eDamage_Normal,
		eDamage_Crush
	};
	struct SDamageContext
	{
		float fDamage;
		uint8 nType;
		uint8 nSourceType;
		CVector2 hitDir;
	};

	CChunkObject( const SClassCreateContext& context ) : CEntity( context ), m_pChunk( NULL ), m_onHitShakeTick( this, &CChunkObject::HitShakeTick ),
		m_fHp( m_nMaxHp ), m_nDamagedEffectsCount( 0 ), m_hitShakeVector( CVector2( 0, 0 ) ), m_nHitShakeFrame( 0 ) { SET_BASEOBJECT_ID( CChunkObject ); }
	~CChunkObject() {}
	SBlock* GetBlock( uint32 x, uint32 y ) { return m_pChunk->GetBlock( x, y ); }
	SChunk* GetChunk() { return m_pChunk; }
	void SetChunk( SChunk* pChunk, class CMyLevel* pLevel );
	void Preview( SChunk* pChunk, CEntity* pParent );
	virtual void OnSetChunk( SChunk* pChunk, class CMyLevel* pLevel );
	virtual void OnCreateComplete( class CMyLevel* pLevel ) {}
	CRectangle GetRect();
	CEntity* GetDecoratorRoot() { return m_p1; }

	virtual void CreateBlockRTLayer( CBlockObject* pBlockObject );

	float GetHp() { return m_fHp; }
	int32 GetMaxHp() { return m_nMaxHp; }
	int32 GetCrushCost() { return m_nCrushCost; }

	virtual void OnLandImpact( uint32 nPreSpeed, uint32 nCurSpeed ) {}
	bool Damage( float nDmg, uint8 nType = 0 );
	virtual bool Damage( SDamageContext& context );
	bool CanDamage() { return m_fHp > 0 && ( !m_pChunk || !m_pChunk->bInvulnerable ); }
	float Repair( float fAmount );
	void AddHitShake( CVector2 shakeVector );
	void ClearHitShake();
	virtual void Kill();
	virtual void Crush() { m_triggerCrushed.Trigger( 0, this ); Kill(); }
	void RemoveChunk();

	void RegisterKilledEvent( CTrigger* pTrigger ) { m_triggerKilled.Register( 0, pTrigger ); }
	void RegisterCrushedEvent( CTrigger* pTrigger ) { m_triggerCrushed.Register( 0, pTrigger ); }

	virtual void OnRemovedFromStage() override { ClearHitShake(); RemoveChunk(); }
protected:
	void HitShakeTick();

	virtual void HandleHitShake( const CVector2& ofs );
	virtual void OnKilled();

	CReference<CEntity> m_p1;

	CReference<CEntity> m_pDamagedEffectsRoot;
	CRenderObject2D* m_pDamagedEffects[4];
	uint32 m_nDamagedEffectsCount;

	TResourceRef<CPrefab> m_strEffect;
	bool m_bEftTiled;
	TResourceRef<CSoundFile> m_strSoundEffect;

	TResourceRef<CDrawableGroup> m_strBlockRTDrawable;

	SChunk* m_pChunk;
	float m_fHp;
	int32 m_nMaxHp;
	float m_nCrushCost;

	CVector2 m_hitShakeVector;
	int32 m_nHitShakeFrame;
	TClassTrigger<CChunkObject> m_onHitShakeTick;

	CEventTrigger<1> m_triggerKilled;
	CEventTrigger<1> m_triggerCrushed;
};

class CSpecialChunk : public CChunkObject
{
	friend void RegisterGameClasses();
public:
	CSpecialChunk( const SClassCreateContext& context ) : CChunkObject( context ), m_strBullet( context ) { SET_BASEOBJECT_ID( CSpecialChunk ); }

	virtual void OnAddedToStage() override;
	virtual void OnLandImpact( uint32 nPreSpeed, uint32 nCurSpeed ) override { if( m_nTriggerImpact && nPreSpeed - nCurSpeed >= m_nTriggerImpact ) Trigger(); }

	virtual void Trigger();
protected:
	virtual void OnKilled() override { if( m_bKillTrigger ) Trigger(); CChunkObject::OnKilled(); }
	bool m_bKillTrigger;
	uint32 m_nTriggerImpact;
	CString m_strBullet;
	CReference<CPrefab> m_pBulletPrefab;
};

class CSpecialChunk1 : public CSpecialChunk
{
	friend void RegisterGameClasses();
public:
	CSpecialChunk1( const SClassCreateContext& context ) : CSpecialChunk( context ) { SET_BASEOBJECT_ID( CSpecialChunk1 ); }
	virtual void Trigger() override;
};

class CSpecialChunk2 : public CSpecialChunk
{
	friend void RegisterGameClasses();
public:
	CSpecialChunk2( const SClassCreateContext& context ) : CSpecialChunk( context ) { SET_BASEOBJECT_ID( CSpecialChunk2 ); }
	virtual void Trigger() override;
};

class CSpecialChunk3 : public CSpecialChunk
{
	friend void RegisterGameClasses();
public:
	CSpecialChunk3( const SClassCreateContext& context ) : CSpecialChunk( context ) { SET_BASEOBJECT_ID( CSpecialChunk3 ); }
	virtual void Trigger() override;
};

class CCharacterChunk : public CChunkObject
{
	friend void RegisterGameClasses();
public:
	CCharacterChunk( const SClassCreateContext& context ) : CChunkObject( context )
	{
		SET_BASEOBJECT_ID( CChunkObject );
	}

	virtual void OnAddedToStage() override { if( m_pCharacter ) m_charOrigPos = m_pCharacter->GetPosition(); }
	virtual void Crush() override;
	virtual void HandleHitShake( const CVector2& ofs ) override { if( m_pCharacter ) m_pCharacter->SetPosition( m_charOrigPos + ofs ); CChunkObject::HandleHitShake( ofs );}
protected:
	virtual void OnKilled() override;
	CReference<CEntity> m_pCharacter;
	CVector2 m_charOrigPos;
};

class CExplosiveChunk : public CChunkObject
{
	friend void RegisterGameClasses();
public:
	CExplosiveChunk( const SClassCreateContext& context ) : CChunkObject( context ), m_strKillEffect( context ), m_bKilled( false ), m_deathTick( this, &CExplosiveChunk::Tick )
	{ SET_BASEOBJECT_ID( CExplosiveChunk ); }

	virtual bool Damage( SDamageContext& context ) override;
	virtual void Kill() override;
	virtual void Crush() override { m_triggerCrushed.Trigger( 0, this ); Explode(); }
	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override
	{
		if( m_deathTick.IsRegistered() )
			m_deathTick.Unregister();
		CChunkObject::OnRemovedFromStage();
	}
protected:
	virtual void Tick();
	virtual void Explode() { SetParentEntity( NULL ); }

	uint32 m_nMaxKillHp;
	uint32 m_nDeathDamage;
	uint32 m_nDeathDamageInterval;
	uint32 m_nKillEffectInterval;
	CString m_strKillEffect;
	CReference<CPrefab> m_pKillEffect;

	bool m_bKilled;
	uint32 m_nDeathDamageCDLeft;
	uint32 m_nKillEffectCDLeft;
	TClassTrigger<CExplosiveChunk> m_deathTick;
};

class CRandomChunk : public CChunkObject
{
	friend void RegisterGameClasses();
public:
	CRandomChunk( const SClassCreateContext& context ) : CChunkObject( context ) { SET_BASEOBJECT_ID( CRandomChunk ); }

	virtual void OnSetChunk( SChunk* pChunk, class CMyLevel* pLevel ) override;
protected:
	virtual void OnKilled() override;
private:
	CVector2 m_texScale;
	CVector2 m_texOfs;
	CVector2 m_dmgTexScale[4];
	CVector2 m_dmgTexOfs[4];
	uint32 m_nHpPerSize;
};

class CRandomEnemyRoom : public CChunkObject
{
	friend void RegisterGameClasses();
public:
	CRandomEnemyRoom( const SClassCreateContext& context ) : CChunkObject( context ), m_strRes( context ), m_strDoor( context ) { SET_BASEOBJECT_ID( CRandomEnemyRoom ); }
	virtual void OnSetChunk( SChunk* pChunk, class CMyLevel* pLevel ) override;
protected:
	virtual void OnKilled() override;
private:
	CString m_strRes;
	CString m_strDoor;
};