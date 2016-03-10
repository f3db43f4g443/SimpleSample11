#pragma once
#include "Math3D.h"
#include "Element2D.h"
#include "RenderSystem.h"

struct SDirectionalLight2D
{
	CVector2 Dir;
	float fIntensity;
	float fShadowScale;
	float fMaxShadowDist;

	LINK_LIST( SDirectionalLight2D, DirectionalLight );
};

struct SPointLight2D
{
	CVector2 Pos;
	CVector4 AttenuationIntensity;
	float fShadowScale;
	float fMaxRange;
	float fLightHeight;

	LINK_LIST( SPointLight2D, PointLight );
};

enum ERenderPass
{
	eRenderPass_Color,
	eRenderPass_Occlusion,
	eRenderPass_Light,
	eRenderPass_GUI,
};

struct SRenderGroup
{
	SRenderGroup() { memset( this, 0, sizeof( SRenderGroup ) ); }
	CElement2D* m_pTransparent;
	uint32 m_nElemCount;

	LINK_LIST_HEAD( m_pOpaqueQueue, CElement2D, Element );
};

class CDrawable2D;
class CRenderObject2D;
class CRenderContext2D
{
public:
	CRenderContext2D() : pUpdatedObjects( NULL ), pRenderSystem( NULL ), renderGroup( NULL ), m_pDirectionalLight( NULL ), m_pPointLight( NULL ), pCurElement( NULL ),
		dTime( 0 ), nTimeStamp( 0 ), nFixedUpdateCount( 0 ), pInstanceDataSize( NULL ), ppInstanceData( NULL ), nRenderGroups( 2 )
	{
		memset( nElemCount, 0, sizeof( nElemCount ) );
	}
	CRenderContext2D( const CRenderContext2D& context );
	double dTime;
	uint32 nTimeStamp;
	uint32 nFixedUpdateCount;
	ERenderPass eRenderPass;

	CVector2 screenRes;
	CVector2 lightMapRes;

	CRectangle rectScene;
	CRectangle rectViewport;

	CMatrix mat;
	CElement2D* pCurElement;

	uint32* pInstanceDataSize;
	void** ppInstanceData;

	CRenderObject2D* pUpdatedObjects;

	IRenderSystem* pRenderSystem;
	SRenderGroup* renderGroup;
	uint32 nRenderGroups;
	uint32 nElemCount[2];

	unsigned int GetElemCount( uint32 nGroup = 0 ) { return renderGroup[nGroup].m_nElemCount; }

	void SetSystemShaderParam( CShaderParam& shaderParam, uint32 nType );

	void AddElement( CElement2D* pElement, uint32 nGroup = 0 );
	void AddDirectionalLight( SDirectionalLight2D* pLight ) { Insert_DirectionalLight( pLight ); }
	void AddPointLight( SPointLight2D* pLight ) { Insert_PointLight( pLight ); }
	void Render( CRenderObject2D* pObject, bool bTest = true );
	void FlushElements( uint32 nGroup = 0 );
private:
	LINK_LIST_HEAD( m_pDirectionalLight, SDirectionalLight2D, DirectionalLight );
	LINK_LIST_HEAD( m_pPointLight, SPointLight2D, PointLight );
};
