#pragma once
#include "Render/Sound.h"

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

void PlaySoundEffect( const char* szName );
ISoundTrack* PlaySoundLoop( const char* szName );
void CreateBGM( CReference<ISoundTrack>& result, const char* szName );