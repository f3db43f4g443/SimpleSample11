#pragma once
#include "Useable.h"

class CEntrance : public CFastUseable
{
	friend void RegisterGameClasses();
public:
	CEntrance( const char* szStage, const char* szStartPoint, const char* szText, float fTime, float fCircleSize );
	CEntrance( const SClassCreateContext& context );
protected:
	void OnUse();
private:
	CString m_strStageName;
	CString m_strStartPointName;
};