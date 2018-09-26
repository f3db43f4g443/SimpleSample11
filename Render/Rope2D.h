#pragma once
#include "Drawable2D.h"
#include "Material.h"
#include "ParticleSystem.h"
#include <vector>
using namespace std;

struct SRopeData
{
	SRopeData() : pParticleInstData( NULL ), nSegmentsPerData( 1 ), pExtraData( NULL ), nExtraDataStride( 0 ), nExtraDataOfs( 0 ), nExtraDataSize( 0 ) {}
	struct SData
	{
		SData() : center( 0, 0 ), fWidth( 1 ), tex0( 0, 0 ), tex1( 0, 0 ), pRefObj( NULL ), nRefTransformIndex( -1 ), bBegin( false ), bEnd( false ) {}
		~SData() {}
		CVector2 center;
		CVector2 worldCenter;
		CMatrix2D worldMat;
		float fWidth;
		CVector2 tex0;
		CVector2 tex1;
		CRenderObject2D* pRefObj;
		int16 nRefTransformIndex;
		bool bBegin;
		bool bEnd;
	};

	void SetDataCount( uint32 nCount );
	void SetData( uint32 nData, const CVector2& center, float fWidth, const CVector2& tex0, const CVector2& tex1 );
	bool CalcAABB( CRectangle& rect );
	void Update( const CMatrix2D& globalTransform );
	
	SParticleInstanceData* pParticleInstData;
	vector<SData> data;
	uint32 nSegmentsPerData;

	CVector4* pExtraData;
	uint16 nExtraDataStride, nExtraDataOfs, nExtraDataSize;
};

class TiXmlElement;
class CRopeDrawable2D : public CParticleSystemDrawable
{
	friend class CMaterialEditor;
	friend class SParticleDrawableEditItem;
public:
	CRopeDrawable2D( CParticleSystemData* pParticleData = NULL ) : CParticleSystemDrawable( pParticleData ) {}
	virtual void Flush( CRenderContext2D& context ) override;
	virtual void Load( IBufReader& buf ) override;
	virtual void Save( CBufFile& buf ) override;
	void LoadNoParticleSystem( IBufReader& buf );
	void SaveNoParticleSystem( CBufFile& buf );
	virtual void BindParams() override;
	void BindParamsNoParticleSystem();
	virtual void LoadXml( TiXmlElement* pRoot ) override;

	vector<CReference<CResource> >& GetDependentResources() { return m_material.GetDependentResources(); }
	void BindShaderResource( EShaderType eShaderType, const char* szName, IShaderResourceProxy* pShaderResource );
private:
	uint32 m_nRopeMaxInst;
	CShaderParam m_paramSegmentsPerData;
};

class CRopeObject2D : public CParticleSystemObject
{
	friend class CRopeDrawable2D;
public:
	CRopeObject2D( CDrawable2D* pDrawable, CDrawable2D* pOcclusionDrawable, CParticleSystemInstance* pData = NULL, bool bGUI = false );
	~CRopeObject2D() { m_data.data.clear(); }

	SRopeData& GetData() { return m_data; }
	void SetBoundExt( const CRectangle& rect ) { m_boundExt = rect; }
	void SetSegmentsPerData( uint32 nSegmentsPerData ) { m_data.nSegmentsPerData = nSegmentsPerData; }
	void SetDataCount( uint32 nCount ) { m_data.SetDataCount( nCount ); m_params.resize( m_nParamCount * nCount ); }
	void SetData( uint32 nData, const CVector2& center, float fWidth, const CVector2& tex0, const CVector2& tex1, uint16 nTransformIndex = INVALID_16BITID );
	void SetParams( uint16 nParamCount, const CVector4* pData,
		uint16 nColorParamBeginIndex, uint16 nColorParamCount,
		uint16 nOcclusionParamBeginIndex, uint16 nOcclusionParamCount,
		uint16 nGUIParamBeginIndex, uint16 nGUIParamCount, bool bDefaultParam = false );
	CVector4* GetParam( uint32 nData );

	virtual const CMatrix2D& GetTransform( uint16 nIndex ) override;

	virtual bool CalcAABB() override;
	virtual void Render( CRenderContext2D& context ) override;
protected:
	virtual void OnTransformUpdated() override { m_data.Update( globalTransform ); }
	SRopeData m_data;
	CRectangle m_boundExt;

	uint32 m_nParamCount;
	uint16 m_nColorParamBeginIndex, m_nColorParamCount;
	uint16 m_nOcclusionParamBeginIndex, m_nOcclusionParamCount;
	uint16 m_nGUIParamBeginIndex, m_nGUIParamCount;
	vector<CVector4> m_params;
};