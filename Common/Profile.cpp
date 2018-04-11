#include "Common.h"
#include "Profile.h"
#include "DateTime.h"

CProfile::CProfile( const char* szLabel ) : m_strLabel( szLabel )
{
	Clear();
	CProfileMgr::Inst()->AddProfile( this );
}

void CProfile::Begin()
{
	if( !CProfileMgr::Inst()->IsEnabled() )
		return;
	m_nBeginTime = GetCycleCount();
}
void CProfile::End()
{
	if( !CProfileMgr::Inst()->IsEnabled() )
		return;
	uint64 nEndTime = GetCycleCount();
	uint64 dTime = nEndTime - m_nBeginTime;
	m_nTotalTimeCurFrame += dTime;
	m_nTotalTime += dTime;
	if( m_nTotalTimeCurFrame > m_nMaxFrameTime )
		m_nMaxFrameTime = m_nTotalTimeCurFrame;
	m_nTotalCountCurFrame++;
	m_nTotalCount++;
}

void CProfile::Clear()
{
	m_nBeginTime = 0;
	m_nTotalTimeCurFrame = 0;
	m_nTotalCountCurFrame = 0;
	m_nTotalTime = 0;
	m_nTotalCount = 0;
	m_nMaxFrameTime = 0;
}

CProfileMgr::CProfileMgr() : m_bEnabled( false )
{
}

void CProfileMgr::BeginProfile()
{
	m_bEnabled = true;
	m_nFrames = 0;
}
void CProfileMgr::EndProfile()
{
	m_bEnabled = false;

	FILE* f = fopen( "./log.txt", "a" );
	fprintf( f, "label\ttime\tavgTime\tcount\tavgCount\tmaxTime\n" );

	for( int i = 0; i < m_vecProfiles.size(); i++ )
	{
		fprintf( f, "%s\t%d\t%f\t%d\t%f\t%d\n",
			m_vecProfiles[i]->m_strLabel.c_str(),
			m_vecProfiles[i]->m_nTotalTime,
			m_vecProfiles[i]->m_nTotalTime / (float)m_nFrames,
			m_vecProfiles[i]->m_nTotalCount,
			m_vecProfiles[i]->m_nTotalCount / (float)m_nFrames,
			m_vecProfiles[i]->m_nMaxFrameTime );
		m_vecProfiles[i]->Clear();
	}
	fclose( f );
}
void CProfileMgr::OnFrameMove()
{
	if( m_bEnabled )
	{
		for( int i = 0; i < m_vecProfiles.size(); i++ )
		{
			m_vecProfiles[i]->m_nTotalTimeCurFrame = 0;
			m_vecProfiles[i]->m_nTotalCountCurFrame = 0;
		}
		m_nFrames++;
	}
}

CProfileMgr* CProfileMgr::Inst()
{
	static CProfileMgr g_inst;
	return &g_inst;
}