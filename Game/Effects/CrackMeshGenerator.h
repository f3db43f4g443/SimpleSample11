#pragma once
#include "Common/Common.h"
#include "Common/Math3D.h"
#include "Common/MathUtil.h"
#include "Render/RenderSystem.h"
#include <vector>
using namespace std;

class CCrackMeshGenerator
{
public:
	CCrackMeshGenerator();
	~CCrackMeshGenerator() { Reset(); }
	void Generate( IRenderSystem* pRenderSystem );
	void Update( IRenderSystem* pRenderSystem, float fElapsedTime );
	float GetTime() { return m_fTime; }
	uint32 GetVertCount() { return m_nVertCount; }

	IVertexBuffer* GetVertexBuffer() { return m_pVertexBuffer; }

	void Reset()
	{
		for( auto item : m_vertices )
			delete item;
		m_vertices.clear();
		m_fTime = 0;
		m_nVertCount = 0;
	}

	struct SVertex
	{
		CVector2 pos;
		uint32 nParent;
		float fArriveTime;
	};

	struct SVertexLoop
	{
		uint32 nVertIndexBegin;
		uint32 nVertCount;
		float fMinTime, fMaxTime;
		SVertex vert[1];

		static SVertexLoop* Alloc( uint32 nVert )
		{
			auto pLoop = new ( malloc( sizeof( SVertexLoop ) + sizeof( SVertex ) * ( nVert - 1 ) ) ) SVertexLoop;
			pLoop->nVertCount = nVert;
			pLoop->fMinTime = FLT_MAX;
			pLoop->fMaxTime = -FLT_MAX;
			return pLoop;
		}
	};

	static const CVertexBufferDesc** GetVBDesc();
private:
	vector<SVertexLoop*> m_vertices;
	CReference<IVertexBuffer> m_pVertexBuffer;
	float m_fTime;

	uint32 m_nMaxLoops;
	uint32 m_nVertCount;
	float m_fStepLength;
	float m_fSplitMin;
	float m_fSplitCoef;
	float m_fSpeed;
};