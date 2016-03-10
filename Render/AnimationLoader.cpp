#include "stdafx.h"
#include "Animation.h"
#include "xml.h"

void CSkeleton::LoadXml( TiXmlElement* pRoot )
{
	const char* szPoseName = XmlGetAttr( pRoot, "armatureName", "" );
	bool bAddPose = m_vecPose.size() > 0;

	if( !bAddPose )
	{
		for( TiXmlElement* pElement = pRoot->FirstChildElement(); pElement; pElement = pElement->NextSiblingElement() )
		{
			bool bBone = !strcmp( pElement->Value(), "bone" );
			bool bSlot = !strcmp( pElement->Value(), "slot" );
			if( !bBone && !bSlot )
				continue;
		
			uint32 nIndex = 0;
			const char* szName = XmlGetAttr( pElement, "name", "" );
			auto itr = m_mapName2BoneIndex.find( szName );
			if( itr != m_mapName2BoneIndex.end() )
				continue;

			const char* szPar = XmlGetAttr( pElement, "parent", "" );
			uint16 nPar = 0;
			if( m_vecBones.size() )
			{
				itr = m_mapName2BoneIndex.find( szPar );
				if( itr == m_mapName2BoneIndex.end() )
					continue;
				nPar = itr->second;
			}
			
			nIndex = m_vecBones.size();
			SBone bone;
			memset( &bone, 0, sizeof( bone ) );
			bone.nPar = nPar;
			m_vecBones.push_back( bone );
			m_mapName2BoneIndex[szName] = nIndex;
		}
	}

	CMatrix2D* pTransforms = (CMatrix2D*)malloc( sizeof( CMatrix2D ) * m_vecBones.size() );
	for( TiXmlElement* pElement = pRoot->FirstChildElement(); pElement; pElement = pElement->NextSiblingElement() )
	{
		bool bBone = !strcmp( pElement->Value(), "bone" );
		bool bSlot = !strcmp( pElement->Value(), "slot" );
		if( !bBone && !bSlot )
			continue;
		
		uint32 nIndex = 0;
		const char* szName = XmlGetAttr( pElement, "name", "" );
		auto itr = m_mapName2BoneIndex.find( szName );
		if( itr == m_mapName2BoneIndex.end() )
			continue;
		nIndex = itr->second;

		TiXmlElement* pElementTransform = pElement->FirstChildElement( "transform" );
		CTransform2D transform2D( 0, 0, 0, 1, 1 );
		if( pElementTransform )
		{
			transform2D.x = XmlGetAttr( pElementTransform, "x", 0.0f );
			transform2D.y = -XmlGetAttr( pElementTransform, "y", 0.0f );
			transform2D.r = -XmlGetAttr( pElementTransform, "skX", 0.0f ) / 180 * PI;
			transform2D.sx = XmlGetAttr( pElementTransform, "scX", 1.0f );
			transform2D.sy = XmlGetAttr( pElementTransform, "scY", 1.0f );
		}

		CMatrix2D& mat = pTransforms[nIndex];
		mat = transform2D.ToMatrix();
	}

	auto pSkin = pRoot->FirstChildElement( "skin" );
	if( pSkin )
	{
		for( TiXmlElement* pElement = pSkin->FirstChildElement(); pElement; pElement = pElement->NextSiblingElement() )
		{
			uint32 nIndex = 0;
			const char* szName = XmlGetAttr( pElement, "name", "" );
			auto itr = m_mapName2BoneIndex.find( szName );
			if( itr == m_mapName2BoneIndex.end() )
				continue;
			nIndex = itr->second;

			TiXmlElement* pDisplay = pElement->FirstChildElement( "display" );
			if( !pDisplay )
				continue;

			TiXmlElement* pElementTransform = pDisplay->FirstChildElement( "transform" );
			CTransform2D transform2D( 0, 0, 0, 1, 1 );
			if( pElementTransform )
			{
				transform2D.x = XmlGetAttr( pElementTransform, "x", 0.0f );
				transform2D.y = -XmlGetAttr( pElementTransform, "y", 0.0f );
				transform2D.r = -XmlGetAttr( pElementTransform, "skX", 0.0f ) / 180 * PI;
				transform2D.sx = XmlGetAttr( pElementTransform, "scX", 1.0f );
				transform2D.sy = XmlGetAttr( pElementTransform, "scY", 1.0f );
			}

			CMatrix2D& mat = pTransforms[nIndex];
			mat = transform2D.ToMatrix();
		}
	}

	m_mapName2PoseIndex[szPoseName] = m_vecPose.size();
	m_vecPose.push_back( pTransforms );
}

void CSkeletonAnimData::LoadXml( TiXmlElement* pRoot, CSkeleton& skeleton )
{
	uint32 nFrameRate = 60;

	fAnimTime = XmlGetAttr( pRoot, "duration", 0.0f ) / nFrameRate;
	nControlGroup = XmlGetAttr( pRoot, "group", 0 );

	uint32 nBoneCount = 0;
	uint32 nEventCount = 0;
	for( TiXmlElement* pElement = pRoot->FirstChildElement(); pElement; pElement = pElement->NextSiblingElement() )
	{
		if( !strcmp( pElement->Value(), "bone" ) )
		{
			TiXmlElement* pFrame = pElement->FirstChildElement( "frame" );
			if( !pFrame )
				continue;

			const char* name = XmlGetAttr( pElement, "name", "" );
			if( skeleton.m_mapName2BoneIndex.find( name ) == skeleton.m_mapName2BoneIndex.end() )
				continue;

			nBoneCount++;
		}

		if( !strcmp( pElement->Value(), "frame" ) )
		{
			const char* szEvent = XmlGetAttr( pElement, "event", "" );
			if( !szEvent[0] )
				continue;
			nEventCount++;
		}
	}

	nBoneAnims = nKeys = nBoneCount;
	if( nBoneCount )
	{
		pBoneAnims = (SBoneAnim*)malloc( sizeof(SBoneAnim)* nBoneCount );
		pKeys = (SCurveKey*)malloc( sizeof(SCurveKey)* nBoneCount );
		nBoneCount = 0;
		for( TiXmlElement* pElement = pRoot->FirstChildElement(); pElement; pElement = pElement->NextSiblingElement() )
		{
			if( strcmp( pElement->Value(), "bone" ) )
				continue;

			uint32 nFrames = 0;
			for( TiXmlElement* pFrame = pElement->FirstChildElement( "frame" ); pFrame; pFrame = pFrame->NextSiblingElement( "frame" ) )
				nFrames++;
			if( !nFrames )
				continue;

			const char* name = XmlGetAttr( pElement, "name", "" );
			auto itr = skeleton.m_mapName2BoneIndex.find( name );
			if( itr == skeleton.m_mapName2BoneIndex.end() )
				continue;
			uint16 nBone = itr->second;

			SCurveKey& key = pKeys[nBoneCount];
			key.nTimes = nFrames;
			key.fTimes = (float*)malloc( sizeof( float ) * nFrames );
			SBoneAnim& anim = pBoneAnims[nBoneCount];
			anim.nKeyPosition = anim.nKeyRotation = anim.nKeyScale = nBoneCount;
			anim.nBone = nBone;
			anim.position.nVertices = anim.rotation.nVertices = anim.scale.nVertices = nFrames;
			anim.position.vertices = (TCurve<CVector2>::SVertex*)malloc( sizeof( TCurve<CVector2>::SVertex ) * nFrames );
			anim.rotation.vertices = (TCurve<float>::SVertex*)malloc( sizeof( TCurve<float>::SVertex ) * nFrames );
			anim.scale.vertices = (TCurve<CVector2>::SVertex*)malloc( sizeof( TCurve<CVector2>::SVertex ) * nFrames );
			anim.position.segments = (TCurve<CVector2>::SSegment*)malloc( sizeof( TCurve<CVector2>::SSegment ) * nFrames );
			anim.rotation.segments = (TCurve<float>::SSegment*)malloc( sizeof( TCurve<float>::SSegment ) * nFrames );
			anim.scale.segments = (TCurve<CVector2>::SSegment*)malloc( sizeof( TCurve<CVector2>::SSegment ) * nFrames );

			float fTime = 0;
			nFrames = 0;
			int32 nRotate = 0;
			for( TiXmlElement* pFrame = pElement->FirstChildElement( "frame" ); pFrame; pFrame = pFrame->NextSiblingElement( "frame" ) )
			{
				key.fTimes[nFrames] = fTime;
				float fDuration = XmlGetAttr( pFrame, "duration", 0.0f ) / 60;
				fTime += fDuration;

				TiXmlElement* pElementTransform = pFrame->FirstChildElement( "transform" );
				CTransform2D transform2D( 0, 0, 0, 1, 1 );
				if( pElementTransform )
				{
					transform2D.x = XmlGetAttr( pElementTransform, "x", 0.0f );
					transform2D.y = -XmlGetAttr( pElementTransform, "y", 0.0f );
					transform2D.r = -XmlGetAttr( pElementTransform, "skX", 0.0f ) / 180 * PI;
					transform2D.sx = XmlGetAttr( pElementTransform, "scX", 1.0f );
					transform2D.sy = XmlGetAttr( pElementTransform, "scY", 1.0f );
				}
				transform2D.r -= nRotate * PI * 2;
				anim.position.vertices[nFrames].value = CVector2( transform2D.x, transform2D.y );
				anim.rotation.vertices[nFrames].value = transform2D.r;
				anim.scale.vertices[nFrames].value = CVector2( transform2D.sx, transform2D.sy );
				anim.position.vertices[nFrames].bSmooth = false;
				anim.rotation.vertices[nFrames].bSmooth = false;
				anim.scale.vertices[nFrames].bSmooth = false;

				nRotate += XmlGetAttr( pFrame, "tweenRotate", 0 );
				nFrames++;
			}

			nFrames = 0;
			for( TiXmlElement* pFrame = pElement->FirstChildElement( "frame" ); pFrame && nFrames < key.nTimes - 1; pFrame = pFrame->NextSiblingElement( "frame" ) )
			{
				float fCurve[4] = { 0, 0, 1, 1 };
				uint32 nCurve = 0;
				for( TiXmlElement* pCurve = pFrame->FirstChildElement( "curve" ); pCurve; pCurve = pCurve->NextSiblingElement( "curve" ) )
				{
					const char* szText = pCurve->GetText();
					stringstream ss;
					ss << szText;
					ss >> fCurve[nCurve++];
				}
				float k0 = fCurve[0] == 0? 1: fCurve[1] / fCurve[0];
				float k1 = fCurve[2] == 1 ? 1 : ( 1 - fCurve[3] ) / ( 1 - fCurve[2] );
				float fDuration = key.fTimes[nFrames + 1] - key.fTimes[nFrames];
				CVector2 dPosition = anim.position.vertices[nFrames + 1].value - anim.position.vertices[nFrames].value;
				anim.position.vertices[nFrames].tangentOut = dPosition * ( k0 / fDuration );
				anim.position.vertices[nFrames + 1].tangentIn = dPosition * ( k1 / fDuration );
				float dRotation = anim.rotation.vertices[nFrames + 1].value - anim.rotation.vertices[nFrames].value;
				anim.rotation.vertices[nFrames].tangentOut = dRotation * ( k0 / fDuration );
				anim.rotation.vertices[nFrames + 1].tangentIn = dRotation * ( k1 / fDuration );
				CVector2 dScale = anim.scale.vertices[nFrames + 1].value - anim.scale.vertices[nFrames].value;
				anim.scale.vertices[nFrames].tangentOut = dScale * ( k0 / fDuration );
				anim.scale.vertices[nFrames + 1].tangentIn = dScale * ( k1 / fDuration );

				if( fCurve[0] != 0 && fCurve[1] != 0 )
				{
					anim.position.vertices[nFrames].bSmooth = true;
					anim.rotation.vertices[nFrames].bSmooth = true;
					anim.scale.vertices[nFrames].bSmooth = true;
				}
				if( fCurve[2] != 1 && fCurve[3] != 1 )
				{
					anim.position.vertices[nFrames + 1].bSmooth = true;
					anim.rotation.vertices[nFrames + 1].bSmooth = true;
					anim.scale.vertices[nFrames + 1].bSmooth = true;
				}

				nFrames++;
			}
			anim.position.CalcSegments( key );
			anim.rotation.CalcSegments( key );
			anim.scale.CalcSegments( key );

			nBoneCount++;
		}
	}

	nEvents = nEventCount;
	if( nEventCount )
	{
		pEvents = (SEvent*)malloc( sizeof(SEvent)* nEventCount );
		nEventCount = 0;
		float fDuration = 0;

		for( TiXmlElement* pElement = pRoot->FirstChildElement(); pElement; pElement = pElement->NextSiblingElement() )
		{
			if( strcmp( pElement->Value(), "frame" ) )
				continue;
			const char* szEvent = XmlGetAttr( pElement, "event", "" );
			if( szEvent[0] )
			{
				pEvents[nEventCount].nType = 0;
				pEvents[nEventCount].fTime = fDuration;
				mapEvents[szEvent] = nEventCount;
				nEventCount++;
			}
			fDuration += XmlGetAttr( pElement, "duration", 0.0f ) / 60;
		}
	}
}

void CAnimationSet::LoadXml( TiXmlElement* pRoot )
{
	for( TiXmlElement* pElement = pRoot->FirstChildElement( "armature" ); pElement; pElement = pElement->NextSiblingElement( "armature" ) )
	{
		m_skeleton.LoadXml( pElement );

		for( TiXmlElement* pAnim = pElement->FirstChildElement( "animation" ); pAnim; pAnim = pAnim->NextSiblingElement( "animation" ) )
		{
			const char* szName = XmlGetAttr( pAnim, "name", "" );
			CSkeletonAnimData* pAnimData = new CSkeletonAnimData;
			pAnimData->LoadXml( pAnim, m_skeleton );
			m_mapAnimData[szName] = pAnimData;

			if( pAnimData->nControlGroup >= m_nControlGroupCount )
				m_nControlGroupCount = pAnimData->nControlGroup + 1;
		}
	}
}