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

	virtual void Play( uint32 nFlag ) override;
	virtual void Stop() override;
	virtual void FadeOut( float fTime ) override;
private:
	void OnTimeout();
	void OnTick();

	CReference<CSound> m_pSound;
	CReference<IDirectSoundBuffer> m_pBuffer;
	bool m_bIsPlaying;
	bool m_bIsKeepingRef;
	float m_fVolume;
	float m_fFadeoutTime;
	TClassTrigger<CSoundTrack> m_onTimeout;
	TClassTrigger<CSoundTrack> m_onTick;
};