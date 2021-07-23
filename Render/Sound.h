#pragma once
#include "Resource.h"

struct SWaveFormat
{
	uint16 nFormatTag;
	uint16 nChannels;
	uint32 nSamplesPerSec;
	uint32 nAvgBytesPerSec;
	uint16 nBlockAlign;
	uint16 wBitsPerSample;
	uint16 cbSize;
};
struct SSoundDesc
{
	SWaveFormat format;
	uint32 nLength;
	uint32 nTotalTime;
};

class IRenderSystem;
class ISound : public CReferenceObject
{
public:
	ISound( const SWaveFormat& fmt, uint32 nLength )
	{
		m_desc.format = fmt;
		m_desc.nLength = nLength;
		m_desc.nTotalTime = m_desc.nLength * 1000LL / m_desc.format.nAvgBytesPerSec;
	}
	const SSoundDesc& GetDesc() { return m_desc; }
private:
	SSoundDesc m_desc;
};

enum
{
	ESoundPlay_Loop = 1,
	ESoundPlay_KeepRef = 2,
};

class ISoundTrack : public CReferenceObject
{
public:
	virtual void Play( uint32 nFlag, bool bReset = false ) = 0;
	virtual void Stop() = 0;
	virtual void Resume() = 0;
	virtual void FadeOut( float fTime ) = 0;
	virtual void SetVolume( float fVolume ) = 0;
	virtual void SetVolumeDB( float fVolume ) = 0;
};

class CSoundFile : public CResource
{
public:
	enum EType
	{
		eResType = eEngineResType_Sound,
	};
	CSoundFile( const char* name, int32 type ) : CResource( name, type ), m_nSoundType( 0 ) {}
	void Create();
	ISoundTrack* CreateSoundTrack();

	static ISound* CreateSound( const char* szFileName );
	static ISoundTrack* PlaySound( const char* szFileName, bool bLoop = false );
private:
	int8 m_nSoundType;
	CReference<ISound> m_pSound;
	vector<CReference<ISound> > m_vecSounds;
};