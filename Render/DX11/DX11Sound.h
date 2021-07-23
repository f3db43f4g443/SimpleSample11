#pragma once

#include "DX11Common.h"
#include "Sound.h"

class CSound : public ISound
{
	friend class CSoundTrack;
public:
	CSound( IDirectSound* pDSound, const void* pData, uint32 nSize, const SWaveFormat& format );
private:
	CReference<IDirectSoundBuffer> m_pBuffer;
};

class CSoundTrack : public ISoundTrack
{
public:
	CSoundTrack( IDirectSound* pDSound, CSound* pSound );

	virtual void Play( uint32 nFlag, bool bReset = false ) override;
	virtual void Stop() override;
	virtual void Resume() override;
	virtual void FadeOut( float fTime ) override;
	virtual void SetVolume( float fVolume ) override;
	virtual void SetVolumeDB( float fVolume ) override;
private:
	void OnTimeout();
	void OnTick();

	CReference<CSound> m_pSound;
	CReference<IDirectSoundBuffer> m_pBuffer;
	bool m_bIsPlaying;
	bool m_bIsKeepingRef;
	float m_fVolume;
	float m_fFadeoutTime;
	uint32 m_nPlayFlag;
	int32 m_nPlayTimeStamp;
	int32 m_nPlayedTime;
	TClassTrigger<CSoundTrack> m_onTimeout;
	TClassTrigger<CSoundTrack> m_onTick;
};