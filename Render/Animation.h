#pragma once

#include "Reference.h"
#include "Math3D.h"
#include "LinkList.h"
#include "Curve.h"
#include "Trigger.h"
#include <vector>
#include <map>
using namespace std;

enum EAnimationPlayMode
{
	eAnimPlayMode_Loop,
	eAnimPlayMode_Once,
	eAnimPlayMode_OnceNoRemove,
};

class CAnimationController;
class IAnimation : public CReferenceObject
{
	friend class CAnimationController;
public:
	IAnimation() : m_pController( NULL ) {}
	virtual void Pause() = 0;
	virtual void Resume() = 0;
	virtual void Goto( float fTime ) = 0;
	virtual void FadeIn( float fTime ) = 0;
	virtual void FadeOut( float fTime ) = 0;
	virtual void QueueAnim( IAnimation* pAnim ) = 0;
	virtual void OnStopped() = 0;

	virtual bool Update( float fDeltaTime, const CMatrix2D& matGlobal ) = 0;
	virtual float GetCurTime() = 0;
	virtual float GetTotalTime() = 0;
	virtual float GetTimeScale() = 0;
	virtual void SetTimeScale( float fTimeScale ) = 0;
	virtual float GetFade() = 0;
	virtual uint32 GetControlGroup() = 0;
	virtual EAnimationPlayMode GetPlayMode() = 0;

	virtual uint16 GetTransformCount() = 0;
	virtual void GetTransform( uint16* nTransforms, CTransform2D* transforms, float* fBlendWeights ) = 0;

	virtual uint32 GetEvent( const char* szName ) = 0;
	virtual void RegisterEvent( uint32 nEvent, CTrigger* pTrigger ) = 0;

	CAnimationController* GetController() { return m_pController; }
private:
	CAnimationController* m_pController;
	LINK_LIST_REF( IAnimation, Anim );
};

class IAnimationData
{
public:
	virtual ~IAnimationData() {}
	virtual IAnimation* CreateAnim( EAnimationPlayMode eMode ) = 0;
};


struct SBone
{
	uint16 nPar;
	uint16 bNoScale : 1;
	uint16 bLockDirectionLocal : 1;
	uint16 bLockDirectionGlobal : 1;
};

class TiXmlElement;

class CSkeleton
{
public:
	~CSkeleton();
	uint16 GetBoneCount() { return m_vecBones.size(); }
	CMatrix2D* GetSkeletonPose( uint32 nPose ) { return nPose < m_vecPose.size()? m_vecPose[nPose]: NULL; }
	uint16 GetBoneIndex( const char* szName );
	uint32 GetPoseIndex( const char* szName );

	void LoadXml( TiXmlElement* pRoot );

	vector<SBone> m_vecBones;
	vector<CMatrix2D*> m_vecPose;

	map<string, uint16> m_mapName2BoneIndex;
	map<string, uint32> m_mapName2PoseIndex;
};

class CSkeletonAnimData : public IAnimationData
{
public:
	CSkeletonAnimData() : pBoneAnims( NULL ), pKeys( NULL ), pEvents( NULL ) {}
	~CSkeletonAnimData();
	virtual IAnimation* CreateAnim( EAnimationPlayMode eMode ) override;

	void LoadXml( TiXmlElement* pRoot, CSkeleton& skeleton );

	struct SBoneAnim
	{
		uint32 nBone;
		uint32 nKeyPosition;
		TCurve<CVector2> position;
		uint32 nKeyRotation;
		TCurve<float> rotation;
		uint32 nKeyScale;
		TCurve<CVector2> scale;
	};
	float fAnimTime;
	uint32 nControlGroup;
	SBoneAnim* pBoneAnims;
	uint32 nBoneAnims;
	SCurveKey* pKeys;
	uint32 nKeys;

	struct SEvent
	{
		float fTime;
		uint8 nType;
	};
	uint32 nEvents;
	SEvent* pEvents;
	map<string, uint32> mapEvents;
};

class CSkeletonAnim : public IAnimation
{
public:
	CSkeletonAnim( CSkeletonAnimData* pData, EAnimationPlayMode eMode );
	virtual void Pause() override { m_bPaused = false; }
	virtual void Resume() override { m_bPaused = true; }
	virtual void Goto( float fTime ) override { m_fCurTime = fTime; }
	virtual void FadeIn( float fTime ) override;
	virtual void FadeOut( float fTime ) override;
	virtual void QueueAnim( IAnimation* pAnim ) override { m_pQueuedAnim = pAnim; }
	virtual void OnStopped() override {}

	virtual bool Update( float fDeltaTime, const CMatrix2D& matGlobal ) override;
	virtual float GetCurTime() override { return m_fCurTime; }
	virtual float GetTotalTime() override { return m_pData->fAnimTime; }
	virtual float GetTimeScale() override { return m_fTimeScale; }
	virtual void SetTimeScale( float fTimeScale ) override { m_fTimeScale = fTimeScale; }
	virtual float GetFade() override { return m_fFade; }
	virtual uint32 GetControlGroup() override { return m_pData->nControlGroup; }
	virtual EAnimationPlayMode GetPlayMode() override { return m_eMode; }

	virtual uint16 GetTransformCount() override { return m_pData->nBoneAnims; }
	virtual void GetTransform( uint16* nTransforms, CTransform2D* transforms, float* fBlendWeights ) override;

	virtual uint32 GetEvent( const char* szName ) override;
	virtual void RegisterEvent( uint32 nEvent, CTrigger* pTrigger ) override;
private:
	CSkeletonAnimData* m_pData;
	EAnimationPlayMode m_eMode;
	CReference<IAnimation> m_pQueuedAnim;
	bool m_bPaused;
	float m_fCurTime;
	float m_fTimeScale;
	float m_fFade;
	float m_fFadeSpeed;

	vector<CEventTrigger<1> > m_vecTriggers;
	uint32 m_nCurEvent;
};

class CAnimationSet : public CReferenceObject
{
public:
	CAnimationSet() : m_nControlGroupCount( 0 ) {}
	~CAnimationSet();
	CSkeleton& GetSkeleton() { return m_skeleton; }
	IAnimation* PlayAnim( const char* szName, EAnimationPlayMode eMode );
	uint32 GetControlGroupCount() { return m_nControlGroupCount; }

	void LoadXml( TiXmlElement* pRoot );
private:
	CSkeleton m_skeleton;
	map<string, IAnimationData*> m_mapAnimData;
	uint32 m_nControlGroupCount;
};

class CAnimationController
{
public:
	CAnimationController( CAnimationSet* pAnimSet, uint32 nPose );
	~CAnimationController();

	CAnimationSet* GetAnimSet() { return m_pAnimSet; }
	IAnimation* CreateAnim( const char* szName, EAnimationPlayMode eMode );
	void PlayAnim( IAnimation* pAnim );
	IAnimation* PlayAnim( const char* szName, EAnimationPlayMode eMode );
	void StopAnim( IAnimation* pAnim );
	void StopAll();
	uint32 GetPose() { return m_nPose; }
	void SetPose( uint32 nPose ) { m_nPose = nPose; }

	float GetUpdateTime() { return m_fUpdateTime; }
	void UpdateTime( float fDeltaTime ) { m_fUpdateTime += fDeltaTime; }
	void Update( const CMatrix2D& matGlobal );
	const CMatrix2D& GetTransform( uint32 nTransform ) { return nTransform < m_vecTransforms.size()? m_vecTransforms[nTransform]: CMatrix2D::GetIdentity(); }
private:
	CReference<CAnimationSet> m_pAnimSet;
	uint32 m_nPose;
	vector<CMatrix2D> m_vecTransforms;
	uint32 m_nAnims;
	float m_fUpdateTime;
	LINK_LIST_REF_HEAD( m_pAnims, IAnimation, Anim )
};