#pragma once
#include "Drawable2D.h"
#include "RenderObject2D.h"
#include "Animation.h"
#include "Material.h"
#include "LinkList.h"
#include <functional>

struct SParticleSystemDataElement
{
	enum
	{
		eRandomType_PerComponent = 0,
		eRandomType_Lerp = 1,
		eRandomType_Slerp = 2,
		eRandomType_Circle = 3,
		eRandomType_Basic = 3,

		eRandomType_TransformToGlobalPos = 4,
		eRandomType_TransformToGlobalDir = 8
	};

	string szName;
	uint8 nComponents;
	uint8 nRandomType;
	uint16 nOffset;
	float dataMin[4], dataMax[4];
	
	void SetFloat1( float minValue, float maxValue, uint8 nRandomType );
	void SetFloat2( const CVector2& minValue, const CVector2& maxValue, uint8 nRandomType );
	void SetFloat3( const CVector3& minValue, const CVector3& maxValue, uint8 nRandomType );
	void SetFloat4( const CVector4& minValue, const CVector4& maxValue, uint8 nRandomType );

	void SetCircle( float fMinRadius, float fMaxRadius, float fMinAngle, float fMaxAngle, uint8 nRandomType );

	void GenerateValue( void* pData, const CMatrix2D& transform );
};

struct SParticleInstanceData
{
	~SParticleInstanceData() { free( pData ); }

	uint32 nBegin, nEnd;
	void* pData;
	float fTime;
	float fEmitTime;
	bool isEmitting;
};

enum EParticleSystemType
{
	eParticleSystemType_Particle,
	eParticleSystemType_Beam,
};

class IParticleEmitter;
class CParticleSystemData : public CReferenceObject
{
public:
	CParticleSystemData( EParticleSystemType eType, uint16 nMaxParticles, float lifeTime, float emitRate, uint8 emitType, bool bBatchAcrossInstances, uint8 nElements, SParticleSystemDataElement* pElements );
	~CParticleSystemData() { delete[] m_pElements; }

	void InitInstanceData( SParticleInstanceData& data );
	bool AnimateInstanceData( SParticleInstanceData& data, const CMatrix2D& transform, float fDeltaTime, IParticleEmitter* pEmitter = NULL );

	void GenerateSingleParticle( SParticleInstanceData& data, uint8* pData, const CMatrix2D& transform )
	{
		*(float*)pData = m_bBatchAcrossInstances? 0: data.fTime;
		for( int iElem = 0; iElem < m_nElements; iElem++ )
		{
			m_pElements[iElem].GenerateValue( pData + m_pElements[iElem].nOffset, transform );
		}
	}

	EParticleSystemType GetType() { return m_eType; }
	float GetLifeTime() { return m_lifeTime; }
	uint16 GetMaxParticles() { return m_nMaxParticles; }
	uint32 GetInstanceSize() { return m_instanceSize; }
	bool IsBatchAcrossInstances() { return m_bBatchAcrossInstances; }
	uint8 GetElementCount() { return m_nElements; }
	SParticleSystemDataElement* GetElements() { return m_pElements; }

	static CParticleSystemData* LoadXml( TiXmlElement* pRoot );
private:
	EParticleSystemType m_eType;
	uint16 m_nMaxParticles;
	uint8 m_emitType;
	uint8 m_nElements;
	float m_lifeTime;
	float m_emitRate;
	bool m_bBatchAcrossInstances;
	SParticleSystemDataElement* m_pElements;

	uint32 m_instanceSize;
};

class IParticleEmitter : public CReferenceObject
{
public:
	virtual void Init( SParticleInstanceData& data ) {}
	virtual void Update( SParticleInstanceData& data, float fTime, const CMatrix2D& transform ) {}
	virtual bool Emit( SParticleInstanceData& data, CParticleSystemData* pParticleSystemData, uint8* pData, const CMatrix2D& transform ) = 0;
};

class CParticleSystemInstance;
class CParticleSystemDrawable;
class CParticleSystemObject : public CRenderObject2D
{
public:
	CParticleSystemObject( CParticleSystemInstance* pData, CDrawable2D* pDrawable, CDrawable2D* pOcclusionDrawable, const CRectangle& rect, bool bGUI = false );
	~CParticleSystemObject();

	CParticleSystemInstance* GetInstanceData() { return m_pInstanceData; }
	virtual void Render( CRenderContext2D& context ) override;

	void OnStopped();
protected:
	CElement2D m_element2D;
	CDrawable2D* m_pColorDrawable;
	CDrawable2D* m_pOcclusionDrawable;
	CDrawable2D* m_pGUIDrawable;
	CReference<CParticleSystemInstance> m_pInstanceData;

	LINK_LIST( CParticleSystemObject, ParticleSystemObject );
};

class CParticleSystemInstance : public IAnimation
{
public:
	CParticleSystemInstance( CParticleSystemData* pData );

	virtual void Pause() override { m_bPaused = true; }
	virtual void Resume() override { m_bPaused = false; }
	virtual void Goto( float fTime ) override {}
	virtual void FadeIn( float fTime ) override {}
	virtual void FadeOut( float fTime ) override {}
	virtual void QueueAnim( IAnimation* pAnim ) override {}
	virtual void OnStopped() override;

	virtual bool Update( float fDeltaTime, const CMatrix2D& matGlobal ) override
	{
		if( !m_bPaused )
		{
			if( m_pEmitter )
				m_pEmitter->Update( m_data, fDeltaTime, matGlobal );
			m_pParticleSystemData->AnimateInstanceData( m_data, matGlobal, fDeltaTime, m_pEmitter );
		}
		return true;
	}
	virtual float GetCurTime() override { return 0; }
	virtual float GetTotalTime() override { return 0; }
	virtual float GetTimeScale() override { return 0; }
	virtual void SetTimeScale( float fTimeScale ) override {}
	virtual float GetFade() override { return 1; }
	virtual uint32 GetControlGroup() override { return INVALID_32BITID; }
	virtual EAnimationPlayMode GetPlayMode() override { return eAnimPlayMode_Loop; }

	virtual uint16 GetTransformCount() override { return 0; }
	virtual void GetTransform( uint16* nTransforms, CTransform2D* transforms, float* fBlendWeights ) override {}

	virtual uint32 GetEvent( const char* szName ) override { return INVALID_32BITID; }
	virtual void RegisterEvent( uint32 nEvent, CTrigger* pTrigger ) override {}

	SParticleInstanceData& GetData() { return m_data; }
	void SetEmitter( IParticleEmitter* pEmitter )
	{
		if( m_pEmitter == pEmitter )
			return;
		m_pEmitter = pEmitter;
		if( pEmitter )
			pEmitter->Init( m_data );
	}
protected:
	bool m_bPaused;
	SParticleInstanceData m_data;
	CReference<CParticleSystemData> m_pParticleSystemData;
	CReference<IParticleEmitter> m_pEmitter;

	LINK_LIST_HEAD( m_pParticleSystemObjects, CParticleSystemObject, ParticleSystemObject )
};

struct SParticleSystemShaderParam
{
	struct SParam
	{
		uint16 nSrcOfs;
		uint16 nDstOfs;
		uint16 nSize;
	};
	vector<SParam> m_shaderParams;
	uint32 m_nZOrderOfs;
	CShaderParam m_paramTime;
	CShaderParam m_paramLife;
	uint32 m_nInstStride;

	void LoadXml( TiXmlElement* pRoot, IShader* pVS, CParticleSystemData* pData );
};

class CParticleSystemDrawable : public CDrawable2D
{
public:
	CParticleSystemDrawable( CParticleSystemData* pData ) : m_pData( pData ), m_pBlendState( NULL ) {}

	virtual void Flush( CRenderContext2D& context ) override;

	CParticleSystemData* GetSystemData() { return m_pData; }

	virtual void LoadXml( TiXmlElement* pRoot );
protected:
	virtual void OnFlushElement( CRenderContext2D& context, CElement2D* pElement ) {}

	CReference<CParticleSystemData> m_pData;
	CMaterial m_material;
	SParticleSystemShaderParam m_param;
	IBlendState* m_pBlendState;
};

class CRopeObject2D;
class CParticleSystem : public CReferenceObject
{
public:
	typedef CParticleSystemDrawable* DrawableAllocFunc( CParticleSystemData* pData );
	typedef CParticleSystemObject* ParticleRenderObjectAllocFunc( CParticleSystemInstance* pData, CDrawable2D* pDrawable, CDrawable2D* pOcclusionDrawable, const CRectangle& rect, bool bGUI );
	typedef CRopeObject2D* RopeRenderObjectAllocFunc( CParticleSystemInstance* pData, CDrawable2D* pDrawable, CDrawable2D* pOcclusionDrawable, bool bGUI );

	CParticleSystem( DrawableAllocFunc *pFunc = NULL ) { m_pDrawableFunc = pFunc; }

	CParticleSystemObject* CreateParticleSystemObject( CAnimationController* pAnimController, CParticleSystemInstance** pInst = NULL, function<ParticleRenderObjectAllocFunc> *pAlloc = NULL );
	CRopeObject2D* CreateBeamObject( CAnimationController* pAnimController, CParticleSystemInstance** pInst = NULL, function<RopeRenderObjectAllocFunc> *pAlloc = NULL );
	CParticleSystemInstance* CreateParticleSystemInst( CAnimationController* pAnimController, CElement2D* pElem = NULL );

	void LoadXml( TiXmlElement* pRoot );
private:
	CParticleSystemDrawable* LoadDrawable( TiXmlElement* pElem );
	CReference<CParticleSystemData> m_pParticleSystemData;
	vector<CParticleSystemDrawable*> m_vecColorPassDrawables;
	vector<CParticleSystemDrawable*> m_vecOcclusionPassDrawables;
	vector<CParticleSystemDrawable*> m_vecGUIPassDrawables;
	CRectangle m_rect;
	DrawableAllocFunc* m_pDrawableFunc;
};