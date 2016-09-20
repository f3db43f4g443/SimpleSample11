#include "stdafx.h"
#include "Animation.h"


CSkeleton::~CSkeleton()
{
	for( CMatrix2D* item : m_vecPose )
		free( item );
}

uint16 CSkeleton::GetBoneIndex( const char* szName )
{
	auto itr = m_mapName2BoneIndex.find( szName );
	if( itr == m_mapName2BoneIndex.end() )
		return INVALID_16BITID;
	return itr->second;
}

uint32 CSkeleton::GetPoseIndex( const char* szName )
{
	auto itr = m_mapName2PoseIndex.find( szName );
	if( itr == m_mapName2PoseIndex.end() )
		return INVALID_32BITID;
	return itr->second;
}

CSkeletonAnimData::~CSkeletonAnimData()
{
	if( pKeys )
	{
		for( int i = 0; i < nKeys; i++ )
		{
			free( pKeys[i].fTimes );
		}
		free( pKeys );
	}

	if( pBoneAnims )
	{
		for( int i = 0; i < nBoneAnims; i++ )
		{
			free( pBoneAnims[i].position.vertices );
			free( pBoneAnims[i].position.segments );
			free( pBoneAnims[i].rotation.vertices );
			free( pBoneAnims[i].rotation.segments );
			free( pBoneAnims[i].scale.vertices );
			free( pBoneAnims[i].scale.segments );
		}
		free( pBoneAnims );
	}
	
	if( pEvents )
		free( pEvents );
}

IAnimation* CSkeletonAnimData::CreateAnim( EAnimationPlayMode eMode )
{
	return new CSkeletonAnim( this, eMode );
}

CSkeletonAnim::CSkeletonAnim( CSkeletonAnimData* pData, EAnimationPlayMode eMode )
	: m_pData( pData )
	, m_eMode( eMode )
	, m_bPaused( false )
	, m_fCurTime( 0 )
	, m_fTimeScale( 1 )
	, m_fFade( 1 )
	, m_fFadeSpeed( 0 )
	, m_nCurEvent( 0 )
{
	m_vecTriggers.resize( m_pData->nEvents );
}

bool CSkeletonAnim::Update( float fDeltaTime, const CMatrix2D& matGlobal )
{
	if( m_bPaused )
		return true;

	fDeltaTime *= m_fTimeScale;
	m_fCurTime += fDeltaTime;
	m_fFade += m_fFadeSpeed * fDeltaTime;
	m_fFade = Max( m_fFade, 0.0f );
	m_fFade = Min( m_fFade, 1.0f );
	auto pEvents = m_pData->pEvents;
	if( fDeltaTime > 0 )
	{
		while( m_nCurEvent < m_pData->nEvents && pEvents[m_nCurEvent].fTime <= m_fCurTime )
		{
			m_vecTriggers[m_nCurEvent].Trigger( 0, (void*)m_nCurEvent );
			m_nCurEvent++;
		}
	}
	else if( fDeltaTime < 0 )
	{
		while( m_nCurEvent && pEvents[m_nCurEvent - 1].fTime >= m_fCurTime )
		{
			m_nCurEvent--;
			m_vecTriggers[m_nCurEvent].Trigger( 0, (void*)m_nCurEvent );
		}
	}

	if( m_fCurTime > m_pData->fAnimTime )
	{
		if( m_eMode == eAnimPlayMode_Loop )
		{
			m_fCurTime -= m_pData->fAnimTime;
			m_nCurEvent = 0;
			while( m_nCurEvent < m_pData->nEvents && pEvents[m_nCurEvent].fTime <= m_fCurTime )
			{
				m_vecTriggers[m_nCurEvent].Trigger( 0, (void*)m_nCurEvent );
				m_nCurEvent++;
			}
		}
		else if( m_eMode == eAnimPlayMode_OnceNoRemove )
		{
			m_fCurTime = m_pData->fAnimTime;
		}
		else
		{
			if( m_pQueuedAnim )
			{
				GetController()->PlayAnim( m_pQueuedAnim );
				if( !m_pQueuedAnim->Update( ( m_fCurTime - m_pData->fAnimTime ) / m_fTimeScale, matGlobal ) )
					GetController()->StopAnim( m_pQueuedAnim );
				m_pQueuedAnim = NULL;
			}
			return false;
		}
	}
	else if( m_fCurTime < 0 )
	{
		if( m_eMode == eAnimPlayMode_Loop )
		{
			m_fCurTime += m_pData->fAnimTime;
			m_nCurEvent = m_pData->nEvents;
			while( m_nCurEvent && pEvents[m_nCurEvent - 1].fTime >= m_fCurTime )
			{
				m_nCurEvent--;
				m_vecTriggers[m_nCurEvent].Trigger( 0, (void*)m_nCurEvent );
			}
		}
		else if( m_eMode == eAnimPlayMode_OnceNoRemove )
		{
			m_fCurTime = 0;
		}
		else
		{
			if( m_pQueuedAnim )
			{
				GetController()->PlayAnim( m_pQueuedAnim );
				if( !m_pQueuedAnim->Update( -m_fCurTime / m_fTimeScale, matGlobal ) )
					GetController()->StopAnim( m_pQueuedAnim );
				m_pQueuedAnim = NULL;
			}
			return false;
		}
	}
	return true;
}

void CSkeletonAnim::FadeIn( float fTime )
{
	if( fTime <= 0 )
		m_fFade = 1;
	else
	{
		m_fFade = 0;
		m_fFadeSpeed = 1.0f / fTime;
	}
}

void CSkeletonAnim::FadeOut( float fTime )
{
	if( fTime <= 0 )
		m_fFade = 0;
	else
	{
		m_fFade = 1;
		m_fFadeSpeed = -1.0f / fTime;
	}
}

void CSkeletonAnim::GetTransform( uint16* nTransforms, CTransform2D* transforms, float* fBlendWeights )
{
	uint32 nKeys = m_pData->nKeys;
	float* T = (float*)alloca( sizeof( float ) * nKeys );
	uint32* nIndices = (uint32*)alloca( sizeof( uint32 ) * nKeys );

	for( int i = 0; i < nKeys; i++ )
	{
		nIndices[i] = m_pData->pKeys[i].GetKeyIndex( m_fCurTime, T[i] );
	}

	uint32 nBones = m_pData->nBoneAnims;
	for( int i = 0; i < nBones; i++ )
	{
		auto& boneAnim = m_pData->pBoneAnims[i];

		CVector2 pos = boneAnim.nKeyPosition < nKeys?
			boneAnim.position.GetValue( nIndices[boneAnim.nKeyPosition], T[boneAnim.nKeyPosition] ): CVector2( 0, 0 );
		float rot = boneAnim.nKeyRotation < nKeys?
			boneAnim.rotation.GetValue( nIndices[boneAnim.nKeyRotation], T[boneAnim.nKeyRotation] ): 0;
		CVector2 scale = boneAnim.nKeyScale < nKeys?
			boneAnim.scale.GetValue( nIndices[boneAnim.nKeyScale], T[boneAnim.nKeyScale] ): CVector2( 1, 1 );

		nTransforms[i] = boneAnim.nBone;
		fBlendWeights[i] = m_fFade;
		CTransform2D& transform = transforms[i];
		transform.x = pos.x;
		transform.y = pos.y;
		transform.r = rot;
		transform.sx = scale.x;
		transform.sy = scale.y;
	}
}

uint32 CSkeletonAnim::GetEvent( const char* szName )
{
	auto itr = m_pData->mapEvents.find( szName );
	if( itr == m_pData->mapEvents.end() )
		return INVALID_32BITID;
	return itr->second;
}

void CSkeletonAnim::RegisterEvent( uint32 nEvent, CTrigger* pTrigger )
{
	if( nEvent >= m_vecTriggers.size() )
		return;
	m_vecTriggers[nEvent].Register( 0, pTrigger );
}

CAnimationSet::~CAnimationSet()
{
	for( auto& item : m_mapAnimData )
	{
		delete item.second;
	}
}

IAnimation* CAnimationSet::PlayAnim( const char* szName, EAnimationPlayMode eMode )
{
	auto itr = m_mapAnimData.find( szName );
	if( itr == m_mapAnimData.end() )
		return NULL;
	return itr->second->CreateAnim( eMode );
}

CAnimationController::CAnimationController( CAnimationSet* pAnimSet, uint32 nPose )
	: m_pAnimSet( pAnimSet )
	, m_nPose( nPose )
	, m_fUpdateTime( 0 )
	, m_nAnims( 0 )
	, m_pAnims( NULL )
{
	m_vecTransforms.resize( m_pAnimSet->GetSkeleton().GetBoneCount() );
}

CAnimationController::~CAnimationController()
{
	StopAll();
}

IAnimation* CAnimationController::CreateAnim( const char* szName, EAnimationPlayMode eMode )
{
	IAnimation* pAnim = m_pAnimSet->PlayAnim( szName, eMode );
	return pAnim;
}

void CAnimationController::PlayAnim( IAnimation* pAnim )
{
	pAnim->m_pController = this;
	Insert_Anim( pAnim );
	m_nAnims++;
}

IAnimation* CAnimationController::PlayAnim( const char* szName, EAnimationPlayMode eMode )
{
	IAnimation* pAnim = CreateAnim( szName, eMode );
	if( pAnim )
		PlayAnim( pAnim );
	return pAnim;
}

void CAnimationController::StopAnim( IAnimation* pAnim )
{
	pAnim->OnStopped();
	pAnim->m_pController = NULL;
	pAnim->RemoveFrom_Anim();
	m_nAnims--;
}

void CAnimationController::StopAll()
{
	while( m_pAnims )
		StopAnim( m_pAnims );
}

void CAnimationController::Update( const CMatrix2D& matGlobal )
{
	float fDeltaTime = m_fUpdateTime;
	m_fUpdateTime = 0;
	uint16 nTransforms = m_vecTransforms.size();
	bool* bInited = nTransforms ? (bool*)alloca( nTransforms ) : NULL;
	if( bInited )
		memset( bInited, 0, nTransforms );
	CTransform2D* transforms = nTransforms ? (CTransform2D*)alloca( sizeof(CTransform2D)* nTransforms ) : NULL;
	CTransform2D* transformsTemp = nTransforms ? (CTransform2D*)alloca( sizeof(CTransform2D)* nTransforms ) : NULL;
	uint16* nTransformsTemp = nTransforms ? (uint16*)alloca( sizeof(uint16)* nTransforms ) : NULL;
	float* fBlendWeightsTemp = nTransforms ? (float*)alloca( sizeof( float ) * nTransforms ) : NULL;
	IAnimation** pAnims = m_nAnims ? (IAnimation**)alloca( sizeof(IAnimation*)* m_nAnims ) : NULL;
	uint32 nControlGroupCount = m_pAnimSet->GetControlGroupCount();
	bool* bControlGroup = (bool*)alloca( nControlGroupCount );
	memset( bControlGroup, 0, nControlGroupCount );

	for( IAnimation* pAnim = m_pAnims; pAnim; )
	{
		IAnimation* pAnim1 = pAnim->NextAnim();
		if( !pAnim->Update( fDeltaTime, matGlobal ) )
			StopAnim( pAnim );
		pAnim = pAnim1;
	}
	
	int32 iAnim = 0;
	for( IAnimation* pAnim = m_pAnims; pAnim; )
	{
		IAnimation* pAnim1 = pAnim->NextAnim();

		uint32 nControlGroup = pAnim->GetControlGroup();
		if( nControlGroup != INVALID_32BITID )
		{
			if( bControlGroup[nControlGroup] )
				StopAnim( pAnim );
			else
			{
				if( pAnim->GetFade() >= 1 )
					bControlGroup[nControlGroup] = true;
				pAnims[iAnim++] = pAnim;
			}
		}
		pAnim = pAnim1;
	}

	for( iAnim--; iAnim >= 0; iAnim-- )
	{
		IAnimation* pAnim = pAnims[iAnim];

		uint16 nCount = pAnim->GetTransformCount();
		if( !nCount )
			continue;

		pAnim->GetTransform( nTransformsTemp, transformsTemp, fBlendWeightsTemp );
		for( uint16 i = 0; i < nCount; i++ )
		{
			uint16 nTransform = nTransformsTemp[i];
			CTransform2D& transform = transformsTemp[i];
			float fBlendWeight = fBlendWeightsTemp[i];
			if( fBlendWeight == 0 )
				continue;

			if( !bInited[nTransform] )
			{
				bInited[nTransform] = true;
				fBlendWeight = 1;
			}
			if( fBlendWeight == 1 )
				transforms[nTransform] = transform;
			else
				transforms[nTransform].Lerp( transforms[nTransform], transform, fBlendWeight );
		}
	}

	CMatrix2D* pBoneMatrices = m_pAnimSet->GetSkeleton().GetSkeletonPose( m_nPose );
	for( uint16 i = 0; i < nTransforms; i++ )
	{
		SBone& bone = m_pAnimSet->GetSkeleton().m_vecBones[i];
		const CMatrix2D& par = bone.nPar == i ? matGlobal : m_vecTransforms[bone.nPar];
		const CMatrix2D& pose = pBoneMatrices[i];
		CMatrix2D& trans = m_vecTransforms[i];
		if( !bInited[i] )
			trans = par;
		else
		{
			CMatrix2D local = transforms[i].ToMatrix();
			if( bone.bLockDirectionLocal )
			{
				float sx = sqrt( local.m00 * local.m00 + local.m10 * local.m10 );
				float sy = sqrt( local.m01 * local.m01 + local.m11 * local.m11 );
				local.m00 = sx;
				local.m01 = 0;
				local.m10 = 0;
				local.m11 = sy;
			}
			trans = par * local;
		}

		if( bone.bLockDirectionGlobal && bone.bNoScale )
		{
			if( bone.bNoScale )
			{
				trans.m00 = 1;
				trans.m01 = 0;
				trans.m10 = 0;
				trans.m11 = 1;
			}
			else
			{
				float sx = sqrt( trans.m00 * trans.m00 + trans.m10 * trans.m10 );
				float sy = sqrt( trans.m01 * trans.m01 + trans.m11 * trans.m11 );
				trans.m00 = sx;
				trans.m01 = 0;
				trans.m10 = 0;
				trans.m11 = sy;
			}
		}
		else if( bone.bNoScale )
		{
			float sx = sqrt( trans.m00 * trans.m00 + trans.m10 * trans.m10 );
			float sy = sqrt( trans.m01 * trans.m01 + trans.m11 * trans.m11 );
			trans.m00 /= sx;
			trans.m10 /= sx;
			trans.m01 /= sy;
			trans.m11 /= sy;
		}

		trans = trans * pose;
	}
}