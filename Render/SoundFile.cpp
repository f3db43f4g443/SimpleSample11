#include "stdafx.h"
#include "Sound.h"
#include "RenderSystem.h"
#include "FileUtil.h"
#include "ResourceManager.h"
#include "xml.h"
#include "Rand.h"

#include "libmad-0.15.1b/mad.h"

struct SSoundDecodeBuffer
{
	const int8 *inData;
	uint32 inDataLen;

	SWaveFormat format;
	CBufFile buf;
};

enum mad_flow mp3_decode_input( void *data,
	struct mad_stream *stream )
{
	SSoundDecodeBuffer *buffer = (SSoundDecodeBuffer*)data;
	const uint8* pData = (const uint8*)buffer->inData;

	if( !buffer->inDataLen )
		return MAD_FLOW_STOP;

	struct SID3V2
	{
		char Header[3];
		char Ver;
		char Revision;
		char Flag;
		char Size[4];
	};
	SID3V2* pID3V2 = (SID3V2*)pData;
	if( !strncmp( pID3V2->Header, "ID3", 3 ) )
	{
		int32 nHeaderSize = ( pID3V2->Size[0] & 0x7F ) * 0x200000
			+ ( pID3V2->Size[1] & 0x7F ) * 0x400
			+ ( pID3V2->Size[2] & 0x7F ) * 0x80
			+ ( pID3V2->Size[3] & 0x7F );
		pData += nHeaderSize + 10;
	}

	mad_stream_buffer( stream, pData, buffer->inDataLen );

	buffer->inDataLen = 0;

	return MAD_FLOW_CONTINUE;
}

enum mad_flow mp3_decode_header( void *data,
	struct mad_header const *header )
{
	SSoundDecodeBuffer *buffer = (SSoundDecodeBuffer*)data;
	auto& format = buffer->format;

	format.nFormatTag = 1;
	switch( header->mode )
	{
	case MAD_MODE_SINGLE_CHANNEL:
		format.nChannels = 1;
		break;
	case MAD_MODE_DUAL_CHANNEL:
	case MAD_MODE_JOINT_STEREO:
	case MAD_MODE_STEREO:
		format.nChannels = 2;
		break;
	default:
		format.nChannels = 0;
		break;
	}
	format.nSamplesPerSec = header->samplerate;
	format.wBitsPerSample = 16;
	format.nBlockAlign = 2 * format.nChannels;
	format.nAvgBytesPerSec = format.nSamplesPerSec * format.nBlockAlign;
	format.cbSize = 0;

	return MAD_FLOW_CONTINUE;
}

inline int32 mp3_decode_scale( mad_fixed_t sample )
{
	/* round */
	sample += ( 1L << ( MAD_F_FRACBITS - 16 ) );

	/* clip */
	if( sample >= MAD_F_ONE )
		sample = MAD_F_ONE - 1;
	else if( sample < -MAD_F_ONE )
		sample = -MAD_F_ONE;

	/* quantize */
	return sample >> ( MAD_F_FRACBITS + 1 - 16 );
}

enum mad_flow mp3_decode_output( void *data,
	struct mad_header const *header,
	struct mad_pcm *pcm )
{
	SSoundDecodeBuffer *buffer = (SSoundDecodeBuffer*)data;
	unsigned int nchannels, nsamples;
	mad_fixed_t const *left_ch, *right_ch;

	/* pcm->samplerate contains the sampling frequency */

	nchannels = pcm->channels;
	nsamples = pcm->length;
	left_ch = pcm->samples[0];
	right_ch = pcm->samples[1];

	while( nsamples-- ) {
		signed int sample;

		/* output sample(s) in 16-bit signed little-endian PCM */

		sample = mp3_decode_scale( *left_ch++ );
		buffer->buf.Write<int16>( sample );

		if( nchannels == 2 ) {
			sample = mp3_decode_scale( *right_ch++ );
			buffer->buf.Write<int16>( sample );
		}
	}

	return MAD_FLOW_CONTINUE;
}

enum mad_flow mp3_decode_error( void *data,
	struct mad_stream *stream,
	struct mad_frame *frame )
{
	return MAD_FLOW_BREAK;
}

void CSoundFile::Create()
{
	if( !strcmp( GetFileExtension( GetName() ), "sf" ) )
	{
		vector<char> content;
		if( GetFileContent( content, GetName(), true ) == INVALID_32BITID )
			return;

		TiXmlDocument doc;
		doc.LoadFromBuffer( &content[0] );
		for( auto pNode = doc.RootElement()->FirstChildElement(); pNode; pNode = pNode->NextSiblingElement() )
		{
			auto szName = XmlGetAttr( pNode, "name", "" );
			auto pSound = CreateSound( szName );
			if( pSound )
				m_vecSounds.push_back( pSound );
		}
		m_nSoundType = 1;
		m_bCreated = true;
		return;
	}

	m_pSound = CreateSound( GetName() );
	if( m_pSound )
		m_bCreated = true;
}

ISoundTrack* CSoundFile::CreateSoundTrack()
{
	if( m_nSoundType == 0 )
		return IRenderSystem::Inst()->CreateSoundTrack( m_pSound );
	else
	{
		if( !m_vecSounds.size() )
			return NULL;
		ISound* pSound = m_vecSounds[SRand::Inst().Rand<int32>( 0, m_vecSounds.size() )];
		return IRenderSystem::Inst()->CreateSoundTrack( pSound );
	}
}

ISound* CSoundFile::CreateSound( const char* szFileName )
{
	vector<char> content;
	if( GetFileContent( content, szFileName, false ) == INVALID_32BITID )
		return NULL;
	ISound* pSound;
	if( !strcmp( GetFileExtension( szFileName ), "wav" ) )
	{
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
			return NULL;
		WaveHeader* pWaveHead = (WaveHeader*)&content[0];
		if( content.size() < pWaveHead->size + 8 )
			return NULL;

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
		pSound = IRenderSystem::Inst()->CreateSound( pData->pData, pData->length, format );
	}
	else if( !strcmp( GetFileExtension( szFileName ), "mp3" ) )
	{
		SSoundDecodeBuffer buffer;
		struct mad_decoder decoder;
		int result;

		/* initialize our private message structure */

		buffer.inData = &content[0];
		buffer.inDataLen = content.size();

		/* configure input, output, and error functions */

		mad_decoder_init( &decoder, &buffer,
			mp3_decode_input, mp3_decode_header, 0 /* filter */, mp3_decode_output,
			mp3_decode_error, 0 /* message */ );

		/* start decoding */

		result = mad_decoder_run( &decoder, MAD_DECODER_MODE_SYNC );

		pSound = IRenderSystem::Inst()->CreateSound( buffer.buf.GetBuffer(), buffer.buf.GetBufLen(), buffer.format );

		/* release the decoder */

		mad_decoder_finish( &decoder );
	}
	return pSound;
}

ISoundTrack* CSoundFile::PlaySound( const char* szFileName, bool bLoop )
{
	ISoundTrack* pSoundTrack = CResourceManager::Inst()->CreateResource<CSoundFile>( szFileName )->CreateSoundTrack();
	pSoundTrack->Play( ESoundPlay_KeepRef | ( bLoop ? ESoundPlay_Loop : NULL ) );
	return pSoundTrack;
}