#pragma once

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