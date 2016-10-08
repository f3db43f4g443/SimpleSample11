#include "stdafx.h"
#include "Sound.h"
#include "RenderSystem.h"
#include "FileUtil.h"
#include "ResourceManager.h"

void CSoundFile::Create()
{
	vector<char> content;
	if( GetFileContent( content, GetName(), false ) == INVALID_32BITID )
		return;

	struct WaveHeader
	{
		uint8 riff[4];
		uint32 size;
		uint8 wave_flag[4];
		uint8 fmt[4];
		uint32 fmt_len;
		uint16 tag;
		uint16 channels;
		uint32 samp_freq;
		uint32 byte_rate;
		uint16 block_align;
		uint16 bit_samp;
	};
	struct WaveData
	{
		uint8 data_flag[4];
		uint32 length;
		uint32 pData[1];
	};
	if( content.size() < sizeof( WaveHeader ) )
		return;
	WaveHeader* pWaveHead = (WaveHeader*)&content[0];
	if( content.size() < pWaveHead->size + 8 )
		return;

	SWaveFormat format = 
	{
		pWaveHead->tag,			//uint16 nFormatTag;
		pWaveHead->channels,		//uint16 nChannels;
		pWaveHead->samp_freq,	//uint32 nSamplesPerSec;
		pWaveHead->byte_rate,	//uint32 nAvgBytesPerSec;
		pWaveHead->block_align,	//uint16 nBlockAlign;
		pWaveHead->bit_samp,		//uint16 wBitsPerSample;
		0,							//uint16 cbSize;
	};
	WaveData* pData = (WaveData*)( ( (uint8*)&pWaveHead->tag ) + pWaveHead->fmt_len );
	m_pSound = IRenderSystem::Inst()->CreateSound( pData->pData, pData->length, format );

	m_bCreated = true;
}

ISoundTrack* CSoundFile::CreateSoundTrack()
{
	return IRenderSystem::Inst()->CreateSoundTrack( m_pSound );
}

ISoundTrack* CSoundFile::PlaySound( const char* szFileName, bool bLoop )
{
	ISoundTrack* pSoundTrack = CResourceManager::Inst()->CreateResource<CSoundFile>( szFileName )->CreateSoundTrack();
	pSoundTrack->Play( ESoundPlay_KeepRef | ( bLoop ? ESoundPlay_Loop : NULL ) );
	return pSoundTrack;
}