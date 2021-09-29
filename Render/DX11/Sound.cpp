#include "stdafx.h"
#include "DX11Sound.h"
#include "DX11RenderSystem.h"

CSound::CSound( IDirectSound* pDSound, const void* pData, uint32 nSize, const SWaveFormat& format )
	: ISound( format, nSize )
{	 
	DSBUFFERDESC dsbdesc;
	memset( &dsbdesc, 0, sizeof( DSBUFFERDESC ) );
	dsbdesc.dwSize = sizeof( DSBUFFERDESC );
	dsbdesc.dwFlags = DSBCAPS_STATIC | DSBCAPS_CTRLVOLUME;
	dsbdesc.dwBufferBytes = nSize;
	dsbdesc.lpwfxFormat = (LPWAVEFORMATEX)&format;
	HRESULT hr = pDSound->CreateSoundBuffer( &dsbdesc, m_pBuffer.AssignPtr(), NULL );

	void* pBufData;
	DWORD nDataSize;
	m_pBuffer->Lock( 0, 0, &pBufData, &nDataSize, NULL, NULL, DSBLOCK_ENTIREBUFFER );
	memcpy( pBufData, pData, nSize );
	m_pBuffer->Unlock( pBufData, nDataSize, NULL, 0 );
}

CSoundTrack::CSoundTrack( IDirectSound* pDSound, CSound* pSound )
	: m_onTimeout( this, &CSoundTrack::OnTimeout )
	, m_onTick( this, &CSoundTrack::OnTick )
	, m_pChannel( NULL )
	, m_fVolume( 1 )
{
	m_bIsPlaying = false;
	m_bIsKeepingRef = false;
	m_pSound = pSound;
	pDSound->DuplicateSoundBuffer( pSound->m_pBuffer, m_pBuffer.AssignPtr() );
}

void CSoundTrack::Play( uint32 nFlag, bool bReset )
{
	CReference<CSoundTrack> pTemp = this;
	Stop();

	m_nPlayFlag = nFlag;
	m_pBuffer->SetCurrentPosition( 0 );
	if( !bReset )
	{
		bool bLoop = nFlag & ESoundPlay_Loop;
		m_pBuffer->Play( 0, 0, bLoop ? DSBPLAY_LOOPING : 0 );
		m_bIsPlaying = true;
		m_nPlayTimeStamp = static_cast<CRenderSystem*>( IRenderSystem::Inst() )->GetTimeStamp();
		if( !bLoop )
			static_cast<CRenderSystem*>( IRenderSystem::Inst() )->Register( m_pSound->GetDesc().nTotalTime, &m_onTimeout );
		if( nFlag & ESoundPlay_KeepRef )
		{
			m_bIsKeepingRef = true;
			AddRef();
		}
		if( m_pChannel )
			m_pChannel->AddSoundTrack( this );
	}
	else
		m_nPlayedTime = 0;
	RefreshVolume();
}

void CSoundTrack::Stop()
{
	if( !m_bIsPlaying )
		return;
	m_bIsPlaying = false;
	if( m_pChannel )
		m_pChannel->RemoveSoundTrack( this );
	m_pBuffer->Stop();
	m_nPlayedTime = static_cast<CRenderSystem*>( IRenderSystem::Inst() )->GetTimeStamp() - m_nPlayTimeStamp;
	if( m_onTimeout.IsRegistered() )
		m_onTimeout.Unregister();
	if( m_bIsKeepingRef )
	{
		m_bIsKeepingRef = false;
		Release();
	}
}

void CSoundTrack::Resume()
{
	if( m_bIsPlaying )
		return;
	CReference<CSoundTrack> pTemp = this;
	bool bLoop = m_nPlayFlag & ESoundPlay_Loop;
	m_pBuffer->Play( 0, 0, bLoop ? DSBPLAY_LOOPING : 0 );
	m_bIsPlaying = true;
	m_nPlayTimeStamp = static_cast<CRenderSystem*>( IRenderSystem::Inst() )->GetTimeStamp() - m_nPlayedTime;
	if( !bLoop )
		static_cast<CRenderSystem*>( IRenderSystem::Inst() )->Register( Max<int32>( 1, m_pSound->GetDesc().nTotalTime - m_nPlayedTime ), &m_onTimeout );
	if( m_nPlayFlag & ESoundPlay_KeepRef )
	{
		m_bIsKeepingRef = true;
		AddRef();
	}
	if( m_pChannel )
		m_pChannel->AddSoundTrack( this );
}

void CSoundTrack::FadeOut( float fTime )
{
	if( !m_bIsPlaying )
		return;
	
	if( m_onTimeout.IsRegistered() )
		m_onTimeout.Unregister();
	m_fFadeoutTime = fTime;
	static_cast<CRenderSystem*>( IRenderSystem::Inst() )->Register( 33, &m_onTick );
}

void CSoundTrack::SetVolume( float fVolume )
{
	m_fVolume = fVolume;
	if( m_pChannel )
		fVolume *= m_pChannel->GetVolume();
	int32 volume = log10( fVolume ) * 10 * 100;
	HRESULT hr = m_pBuffer->SetVolume( Max( DSBVOLUME_MIN, Min( DSBVOLUME_MAX, volume ) ) );
}

void CSoundTrack::SetVolumeDB( float fVolume )
{
	m_fVolume = exp( fVolume / 10 );
	if( m_pChannel )
		fVolume += log10( m_pChannel->GetVolume() ) * 10;
	int32 volume = fVolume * 100;
	HRESULT hr = m_pBuffer->SetVolume( Max( DSBVOLUME_MIN, Min( DSBVOLUME_MAX, volume ) ) );
}

void CSoundTrack::RefreshVolume()
{
	SetVolume( m_fVolume );
}

void CSoundTrack::SetChannel( ISoundTrackChannel* pChannel )
{
	m_pChannel = pChannel;
	if( m_bIsPlaying )
	{
		pChannel->AddSoundTrack( this );
		RefreshVolume();
	}
}

void CSoundTrack::OnTick()
{
	float fTime = 33.0f / 1000;
	float fPreTime = m_fFadeoutTime;
	m_fFadeoutTime = Max( m_fFadeoutTime - fTime, 0.0f );
	m_fVolume = m_fVolume / fPreTime * m_fFadeoutTime;
	if( m_fVolume <= 0 )
	{
		Stop();
		return;
	}

	RefreshVolume();
	static_cast<CRenderSystem*>( IRenderSystem::Inst() )->Register( 33, &m_onTick );
}

void CSoundTrack::OnTimeout()
{
	m_bIsPlaying = false;
	if( m_pChannel )
		m_pChannel->RemoveSoundTrack( this );
	if( m_bIsKeepingRef )
	{
		m_bIsKeepingRef = false;
		Release();
	}
}

ISound* CRenderSystem::CreateSound( const void* pBuffer, uint32 nSize, const SWaveFormat& format )
{
	return new CSound( m_pDSound, pBuffer, nSize, format );
}

ISoundTrack* CRenderSystem::CreateSoundTrack( ISound* pSound )
{
	return new CSoundTrack( m_pDSound, static_cast<CSound*>( pSound ) );
}