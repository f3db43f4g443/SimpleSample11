#include "Common.h"
#include "DateTime.h"
#include <time.h>

#ifdef _WIN32
#include <Windows.h>
#else
#include <sys/time.h>
#endif

uint64 GetLocalTime()
{
	timeval tv;
#ifdef _WIN32
	time_t clock;
	struct tm tm;
	SYSTEMTIME wtm;
	GetLocalTime(&wtm);
	tm.tm_year     = wtm.wYear - 1900;
	tm.tm_mon     = wtm.wMonth - 1;
	tm.tm_mday     = wtm.wDay;
	tm.tm_hour     = wtm.wHour;
	tm.tm_min     = wtm.wMinute;
	tm.tm_sec     = wtm.wSecond;
	tm.tm_isdst    = -1;
	clock = mktime(&tm);
	tv.tv_sec = clock;
	tv.tv_usec = wtm.wMilliseconds * 1000;
#else
	gettimeofday(&tv,NULL);    
#endif
	return tv.tv_sec * 1000LL + tv.tv_usec / 1000;    
}

void LocalTimeToDateTime( uint64 nLocalTime, SDateTime& dateTime )
{
	time_t nSec = nLocalTime / 1000;
	uint32 nMilliSec = nLocalTime - nSec * 1000;
	time_t time;
	auto pTM = localtime( &nSec );
	dateTime.nYear = pTM->tm_year + 1900;
	dateTime.nMonth = pTM->tm_mon + 1;
	dateTime.nDay = pTM->tm_mday;
	dateTime.nWeekDay = pTM->tm_wday;
	dateTime.nHour = pTM->tm_hour;
	dateTime.nMin = pTM->tm_min;
	dateTime.nSec = pTM->tm_sec;
	dateTime.nMilliSec = nMilliSec;
}

#ifdef _WIN32
#include <Windows.h>
uint64 GetCycleCount()
{
	static UINT64 g_nFreq = 0;
	if( !g_nFreq )
		QueryPerformanceFrequency( (LARGE_INTEGER*)&g_nFreq );

	LARGE_INTEGER nCur;
	QueryPerformanceCounter( &nCur );
	uint64 nSecond = nCur.QuadPart / g_nFreq;
	uint64 nTick = nCur.QuadPart % g_nFreq;
	return nSecond * 1000000 + ( nTick * 1000000 ) / g_nFreq;
}
#else
#include <sys/time.h>
uint64 GetCycleCount()
{
	struct timeval tv;
	gettimeofday( tv, NULL );
	UINT64 seconds = (UINT64)tv.tv_sec * 1000000;
	UINT64 useconds = (UINT64)tv.tv_usec;
	return seconds + useconds;
}
#endif