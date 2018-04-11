#pragma once

struct SDateTime	//本地时间
{
	int32 nYear;
	uint8 nMonth;
	uint8 nDay;
	uint8 nWeekDay;
	uint8 nHour;
	uint8 nMin;
	uint8 nSec;
	uint16 nMilliSec;
};

uint64 GetLocalTime();
uint64 GetCycleCount();

void LocalTimeToDateTime( uint64 nLocalTime, SDateTime& dateTime );