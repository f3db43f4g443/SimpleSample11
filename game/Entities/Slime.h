#pragma once
#include "Character.h"
#include "Render/Canvas.h"
#include "Render/Image2D.h"

class CSlimeGround;

class CSlimeTrap : public CEntity
{
	friend class CSlime;
public:
	CSlimeTrap( float fSizeSpeed, float fMoveSpeed, float fMaxMoveDist, const CRectangle& rect, const CRectangle& texRect );

	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;
	void Kill();
private:
	void OnTickAfterHitTest();
	float m_fSizeSpeed;
	float m_fMoveSpeed;
	float m_fMaxMoveDist;

	bool m_bAlive;
	float m_fAlpha;
	TClassTrigger<CSlimeTrap> m_tickAfterHitTest;
};

class CSlime : public CCharacter
{
	friend class CSlimeCore;
	friend class CSlimeForceField;
public:
	CSlime( CSlimeGround* pSlimeGround, const CVector2& velocity, float fSize );
	
	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;
	void OnUnbound( bool bExplode = false );
	const CVector2& GetVelocity() { return m_velocity; }
	void SetVelocity( const CVector2& velocity ) { m_velocity = velocity; }
	void ChangeColor( const CVector4& color ) { m_targetParam = color; m_bBlink = false; m_fParamChangeTimeLeft = 1.0f; }
	void Blink( const CVector4& color ) { m_blinkColor = color; m_bBlink = true; m_fParamChangeTimeLeft = 0.0f; }
protected:
	virtual void OnTickBeforeHitTest() override;
	virtual void OnTransformUpdated() override;
	virtual void ChangeVelocity( bool bExplode = false );

	void OnTickAfterHitTest();
	void OnChangeVelocity() { ChangeVelocity( false ); }

	float m_fSpawnTime;
	float m_fSize;
	CVector2 m_velocity;
	CSlimeGround* m_pSlimeGround;
	TClassTrigger<CSlime> m_tickAfterHitTest;
	TClassTrigger<CSlime> m_tickChangeVelocity;
	CReference<CSlimeTrap> m_pTrapped;
	CVector2 m_trappedMoveTarget;
	CVector4 m_param;
	CVector4 m_targetParam;
	float m_fParamChangeTimeLeft;
	bool m_bBlink;
	CVector4 m_blinkColor;

	CReference<CImage2D> m_pDynamicTextureRenderObject;

	CSlimeCore* m_pSlimeCore;
	uint32 m_nSlotIndex;
	uint32 m_nBoundState;
	float m_fSpeed;
	LINK_LIST( CSlime, UnboundSlime );
};

class CSlimeForceField : public CEntity
{
public:
	CSlimeForceField( CSlimeGround* pSlimeGround, float fLife, float fMaxDist, float fStrength );
	
	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;
private:
	void OnTickAfterHitTest();

	float m_fLife;
	float m_fMaxDist;
	float m_fStrength;
	CReference<CSlimeGround> m_pSlimeGround;
	TClassTrigger<CSlimeForceField> m_tickAfterHitTest;
};

class CSlimeGround : public CEntity
{
	friend class CSlime;
public:
	CSlimeGround() : m_pUnboundSlimes( NULL ), m_nUnboundSlimeCount( 0 ), m_dynamicTexture( 1024, 1024, EFormat::EFormatR8G8B8A8UNorm ) {}
	virtual void OnAddedToStage() override;
	virtual void OnRemovedFromStage() override;

	void AddUnboundSlime( CSlime* pSlime ) { Insert_UnboundSlime( pSlime ); m_nUnboundSlimeCount++; }
	void RemoveUnboundSlime( CSlime* pSlime ) { pSlime->RemoveFrom_UnboundSlime(); m_nUnboundSlimeCount--; }
	uint32 GetUnboundSlimeCount() { return m_nUnboundSlimeCount; }

	virtual void Render( CRenderContext2D& context ) override { if( context.eRenderPass == eRenderPass_Color ) m_dynamicTexture.Render( context ); }
private:
	CDynamicTexture m_dynamicTexture;
	
	class CDefaultDrawable2D* m_pUpdateDrawable;
	class CDefaultDrawable2D* m_pColorDrawable;
	class CDefaultDrawable2D* m_pOcclusionDrawable;
	
	uint32 m_nUnboundSlimeCount;
	LINK_LIST_HEAD( m_pUnboundSlimes, CSlime, UnboundSlime );
};