#pragma once

#include <vector>
#include <string>
using namespace std;

class CProfile
{
	friend class CProfileMgr;
public:
	CProfile( const char* szLabel );

	void Begin();
	void End();

	void Clear();
private:
	string m_strLabel;
	unsigned int m_nBeginTime;
	unsigned int m_nTotalTimeCurFrame;
	unsigned int m_nTotalCountCurFrame;
	unsigned int m_nTotalTime;
	unsigned int m_nTotalCount;
	unsigned int m_nMaxFrameTime;
};

class CProfileMgr
{
public:
	CProfileMgr();
	void BeginProfile();
	void EndProfile();
	void OnFrameMove();

	void AddProfile( CProfile* pProfile ) { m_vecProfiles.push_back( pProfile ); }

	bool IsEnabled() { return m_bEnabled; }

	static CProfileMgr* Inst();
private:
	bool m_bEnabled;
	unsigned int m_nFrames;
	vector<CProfile*> m_vecProfiles;
};

#define PROFILE_BEGIN( Label ) static CProfile __Profile_##Label( #Label ); __Profile_##Label.Begin();
#define PROFILE_END( Label ) __Profile_##Label.End();