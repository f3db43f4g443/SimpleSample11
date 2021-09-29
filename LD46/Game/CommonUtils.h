#pragma once
#include "Render/Sound.h"
#include <set>
using namespace std;

class CLuaTrigger : public CTrigger
{
public:
	CLuaTrigger() { bAutoDelete = true; }
	virtual void Run( void* pContext ) override;

	enum
	{
		eParam_None,
		eParam_Int,
		eParam_Obj,
	};
	static CLuaTrigger* CreateFromText( const char* sz, int8 nParamType = eParam_None );
	static CLuaTrigger* CreateAuto( int8 nParamType = eParam_None );
private:
	CReference<CLuaState> m_pLuaState;
	int8 m_nParamType;
};


class CSoundChannel : public ISoundTrackChannel
{
public:
	CSoundChannel() : m_fVolume( 1 ) {}
	virtual void AddSoundTrack( ISoundTrack* pSoundTrack ) override
	{
		m_setTracks.insert( pSoundTrack );
	}
	virtual void RemoveSoundTrack( ISoundTrack* pSoundTrack ) override
	{
		m_setTracks.erase( pSoundTrack );
	}
	virtual float GetVolume() override
	{
		return m_fVolume;
	}
	void SetVolume( float fVolume )
	{
		m_fVolume = fVolume;
		for( auto pTrack : m_setTracks )
			pTrack->RefreshVolume();
	}

	static CSoundChannel& MusicChannel()
	{
		static CSoundChannel g_inst;
		return g_inst;
	}
	static CSoundChannel& SfxChannel()
	{
		static CSoundChannel g_inst;
		return g_inst;
	}
private:
	float m_fVolume;
	set<ISoundTrack*> m_setTracks;
};

void SetMusicGlobalVolume( float f );
void SetSfxGlobalVolume( float f );
void PlaySoundEffect( const char* szName );
ISoundTrack* PlaySoundLoop( const char* szName );
void CreateBGM( CReference<ISoundTrack>& result, const char* szName );