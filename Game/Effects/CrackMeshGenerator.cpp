#include "stdafx.h"
#include "CrackMeshGenerator.h"
#include "Render/GlobalRenderResources.h"

const CVertexBufferDesc** CCrackMeshGenerator::GetVBDesc()
{
	static SVertexBufferElement g_CrackMeshVBElements[] =
	{
		{ "Position", 0, EFormat::EFormatR32G32Float, 0 },
		{ "Position", 1, EFormat::EFormatR32G32Float, 0 },
		{ "TexCoord", 0, EFormat::EFormatR32G32B32Float, 0 },
		{ "TexCoord", 1, EFormat::EFormatR32G32B32Float, 0 },
		{ "TexCoord", 2, EFormat::EFormatR32G32B32Float, 0 },
		{ "TexCoord", 3, EFormat::EFormatR32G32B32Float, 0 },
		{ "TexCoord", 4, EFormat::EFormatR32G32Float, 0 },
		{ "TexCoord", 5, EFormat::EFormatR32G32Float, 0 },
		{ "TexCoord", 6, EFormat::EFormatR32G32Float, 0 },
		{ "TexCoord", 7, EFormat::EFormatR32G32Float, 0 },
	};
	static const CVertexBufferDesc* pDesc = new CVertexBufferDesc( ELEM_COUNT( g_CrackMeshVBElements ), g_CrackMeshVBElements, 0, false, false, true );
	return &pDesc;
}

class CCrackMeshGenVS : public CGlobalShader
{
	DECLARE_GLOBAL_SHADER( CCrackMeshGenVS );
protected:
	virtual void OnCreated() override
	{
		GetShader()->GetShaderInfo().Bind( m_paramVertInfo, "vertInfo", "VertGenInfoBuffer" );
	}
public:
	void SetParams( IRenderSystem* pRenderSystem, CCrackMeshGenerator::SVertexLoop* pLoop )
	{
		uint32 nLen = pLoop->nVertCount * sizeof( CCrackMeshGenerator::SVertex );
		m_paramVertInfo.Set( pRenderSystem, pLoop->vert, nLen );
		m_paramVertInfo.Set( pRenderSystem, pLoop->vert, sizeof( CCrackMeshGenerator::SVertex ), nLen );
	}
private:
	CShaderParam m_paramVertInfo;
};

class CCrackMeshGenGS : public CGlobalShader
{
	DECLARE_GLOBAL_SHADER( CCrackMeshGenGS );
protected:
	virtual void OnCreated() override
	{
		GetShader()->GetShaderInfo().Bind( m_paramVertInfo, "vertInfo", "VertGenInfoBuffer" );
		GetShader()->GetShaderInfo().Bind( m_paramSpeed, "fSpeed" );
	}
public:
	void SetParams( IRenderSystem* pRenderSystem, CCrackMeshGenerator::SVertexLoop* pLoop, float fSpeed )
	{
		if( pLoop )
		{
			uint32 nLen = pLoop->nVertCount * sizeof( CCrackMeshGenerator::SVertex );
			m_paramVertInfo.Set( pRenderSystem, pLoop->vert, nLen );
		}
		else
		{
			CCrackMeshGenerator::SVertex vert = { { 0, 0 }, 0, 0 };
			m_paramVertInfo.Set( pRenderSystem, &vert, sizeof( vert ) );
		}
		m_paramSpeed.Set( pRenderSystem, &fSpeed );
	}
private:
	CShaderParam m_paramVertInfo;
	CShaderParam m_paramSpeed;
};

IMPLEMENT_GLOBAL_SHADER( CCrackMeshGenVS, "Shader/CrackEffect.shader", "VSGen", "vs_5_0" );
IMPLEMENT_GLOBAL_SHADER_WITH_SO( CCrackMeshGenGS, "Shader/CrackEffect.shader", "GSGen", "gs_5_0", CCrackMeshGenerator::GetVBDesc(), 1, INVALID_32BITID );

CCrackMeshGenerator::CCrackMeshGenerator()
	: m_fStepLength( 32 )
	, m_fSplitMin( 64 )
	, m_fSplitCoef( 16 )
	, m_fSpeed( 512 )
	, m_nMaxLoops( 12 )
	, m_fTime( 0 )
	, m_nVertCount( 0 )
{
}

void CCrackMeshGenerator::Generate( IRenderSystem* pRenderSystem )
{
	SVertexLoop* pLoop = NULL;
	SVertexLoop* pPreLoop = NULL;
	
	if( !m_pVertexBuffer )
	{
		auto pVBDesc = *GetVBDesc();
		m_pVertexBuffer = pRenderSystem->CreateVertexBuffer( pVBDesc->vecElements.size(), &pVBDesc->vecElements[0], pVBDesc->nStride * 16384, NULL, false, false, true );
	}

	if( !m_vertices.size() )
	{
		pLoop = SVertexLoop::Alloc( 6 );
		pLoop->nVertIndexBegin = 1;
		for( int i = 0; i < 6; i++ )
		{
			pLoop->vert[i].nParent = 0;
			float fAngle = ( SRand::Inst().Rand( -0.1f, 0.1f ) + i ) * PI / 3;
			float fRadius = SRand::Inst().Rand( 0.9f, 1.1f ) * m_fStepLength;
			pLoop->vert[i].pos = CVector2( cos( fAngle ), sin( fAngle ) ) * fRadius;
			float l = pLoop->vert[i].pos.Length();
			float fTime = l / m_fSpeed;
			pLoop->vert[i].fArriveTime = fTime;
			pLoop->fMinTime = Min( pLoop->fMinTime, fTime );
			pLoop->fMaxTime = Max( pLoop->fMaxTime, fTime );
		}
		m_vertices.push_back( pLoop );
		m_nVertCount = 18;
	}
	else
	{
		struct SGenerateContext
		{
			bool bSplit;
			CVector2 pos1, pos2;
		};
		vector<SGenerateContext> vecContext;

		pPreLoop = m_vertices.back();
		vecContext.resize( pPreLoop->nVertCount );
		uint32 nVerts = pPreLoop->nVertCount;
		int iVert, iVert1;
		for( iVert = 0; iVert < pPreLoop->nVertCount; iVert++ )
		{
			auto& vert = pPreLoop->vert[iVert];
			auto& vertLeft = pPreLoop->vert[iVert - 1 >= 0 ? iVert - 1 : pPreLoop->nVertCount - 1];
			auto& vertRight = pPreLoop->vert[iVert + 1 < pPreLoop->nVertCount ? iVert + 1 : 0];
			auto& context = vecContext[iVert];

			auto dVec1 = vertLeft.pos - vert.pos;
			auto dVec2 = vertRight.pos - vert.pos;
			float l1 = dVec1.Length();
			float l2 = dVec2.Length();
			float r = Max( sqrt( l1 * l2 ) - m_fSplitMin, 0.0f );
			r = r / ( r + m_fSplitCoef );
			context.bSplit = SRand::Inst().Rand( 0.0f, 1.0f ) < r;
			if( context.bSplit )
				nVerts++;

			CVector2 norm1 = CVector2( -dVec1.y / l1, dVec1.x / l1 );
			CVector2 norm2 = CVector2( dVec2.y / l2, -dVec2.x / l2 );
			CVector2 ofs1 = dVec2 * ( 1.0f / dVec2.Dot( norm1 ) ) * m_fStepLength;
			CVector2 ofs2 = dVec1 * ( 1.0f / dVec1.Dot( norm2 ) ) * m_fStepLength;
			CVector2 ofs = ofs1 + ofs2;
			norm1 = norm1 * m_fStepLength;
			norm2 = norm2 * m_fStepLength;

			float l = ofs1.Length();
			float fMaxl1 = Min( m_fStepLength, l1 * 0.4f );
			if( l > fMaxl1 )
				ofs1 = ofs + ( ofs1 - ofs ) * ( fMaxl1 / l );
			float fMaxl2 = Min( m_fStepLength, l2 * 0.4f );
			if( l > fMaxl2 )
				ofs2 = ofs + ( ofs2 - ofs ) * ( fMaxl2 / l );

			if( context.bSplit )
			{
				auto& pos1 = context.pos1;
				auto& pos2 = context.pos2;
				float r1 = SRand::Inst().Rand( 0.0f, 0.7f );
				pos1 = ofs1 + ( ofs - ofs1 ) * r1;
				float r2 = SRand::Inst().Rand( 0.0f, 0.7f );
				pos2 = ofs2 + ( ofs - ofs2 ) * r2;
			}
			else
			{
				auto& pos = context.pos1;
				float r = SRand::Inst().Rand( 0.0f, l1 + l2 );
				if( r < l1 )
					pos = ofs + ( ofs1 - ofs ) * SRand::Inst().Rand( 0.0f, 1.0f );
				else
					pos = ofs + ( ofs2 - ofs ) * SRand::Inst().Rand( 0.0f, 1.0f );
			}
		}

		pLoop = SVertexLoop::Alloc( nVerts );
		pLoop->nVertIndexBegin = pPreLoop->nVertIndexBegin + pPreLoop->nVertCount;
		m_vertices.push_back( pLoop );

		for( iVert = 0, iVert1 = 0; iVert < pPreLoop->nVertCount; iVert++ )
		{
			auto& preVert = pPreLoop->vert[iVert];
			auto& context = vecContext[iVert];
			pLoop->vert[iVert1].nParent = iVert;
			pLoop->vert[iVert1].fArriveTime = preVert.fArriveTime;
			pLoop->vert[iVert1++].pos = preVert.pos + context.pos1;
			m_nVertCount += 6;
			if( context.bSplit )
			{
				pLoop->vert[iVert1].nParent = iVert;
				pLoop->vert[iVert1].fArriveTime = preVert.fArriveTime;
				pLoop->vert[iVert1++].pos = preVert.pos + context.pos2;
				m_nVertCount += 3;
			}
		}

		uint32 n1 = SRand::Inst().Rand( 0u, pLoop->nVertCount );
		auto pos1 = pLoop->vert[n1].pos;
		float lMax = -1;
		CVector2 dVecMax( 0, 0 );
		for( iVert = 0; iVert < pLoop->nVertCount; iVert++ )
		{
			CVector2 dVec = pLoop->vert[iVert].pos - pos1;
			float l = dVec.Dot( dVec );
			if( l > lMax )
			{
				lMax = l;
				dVecMax = dVec;
			}
		}
		lMax = sqrt( lMax );
		float k = SRand::Inst().Rand( 0.0f, m_fStepLength ) / lMax;
		dVecMax = dVecMax * ( 1.0f / lMax );

		for( iVert = 0; iVert < pLoop->nVertCount; iVert++ )
		{
			auto& vert = pLoop->vert[iVert];
			auto& pos = vert.pos;
			pos = pos + dVecMax * ( k * ( pos - pos1 ).Dot( dVecMax ) );

			auto& preVert = pPreLoop->vert[vert.nParent];
			float l = ( vert.pos - preVert.pos ).Length();
			float fTime = l / m_fSpeed;
			vert.fArriveTime += fTime;
			pLoop->fMinTime = Min( pLoop->fMinTime, vert.fArriveTime );
			pLoop->fMaxTime = Min( pLoop->fMaxTime, vert.fArriveTime );
		}
	}

	auto pStreamOutput = m_pVertexBuffer->GetStreamOutput();
	uint32 nOffset = m_vertices.size() == 1 ? 0 : -1;

	pRenderSystem->SetPrimitiveType( EPrimitiveType::LineList );
	pRenderSystem->SetSOTargets( &pStreamOutput, &nOffset, 1 );
	pRenderSystem->SetVertexBuffer( 0, CGlobalRenderResources::Inst()->GetVBStrip() );
	
	auto pVertexShader = CCrackMeshGenVS::Inst();
	auto pGeometryShader = CCrackMeshGenGS::Inst();
	static IShaderBoundState* g_pShaderBoundState = NULL;
	const CVertexBufferDesc* pDesc = &CGlobalRenderResources::Inst()->GetVBQuad()->GetDesc();
	pRenderSystem->SetShaderBoundState( g_pShaderBoundState, pVertexShader->GetShader(), NULL, &pDesc, 1, pGeometryShader->GetShader() );

	pVertexShader->SetParams( pRenderSystem, pLoop );
	pGeometryShader->SetParams( pRenderSystem, pPreLoop, m_fSpeed );

	pRenderSystem->Draw( pLoop->nVertCount * 2, 0 );
	pRenderSystem->SetPrimitiveType( EPrimitiveType::TriangleList );
	pRenderSystem->SetSOTargets( NULL, NULL, 0 );
}

void CCrackMeshGenerator::Update( IRenderSystem* pRenderSystem, float fElapsedTime )
{
	m_fTime += fElapsedTime;

	while( m_vertices.size() < m_nMaxLoops )
	{
		float fTime = m_vertices.size() ? m_vertices.back()->fMinTime : 0;
		if( fTime > m_fTime )
			break;
		Generate( pRenderSystem );
	}
}