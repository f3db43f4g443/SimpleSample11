#pragma once
#include <string>
using namespace std;

class CPlayerData
{
public:
	CPlayerData() : bFinishedTutorial( false ), bIsDesign( false ) {}
	string strPlayerName;
	bool bFinishedTutorial;
	bool bIsDesign;

	void Load( const char* szPlayer );
	void Save();

	DECLARE_GLOBAL_INST_REFERENCE( CPlayerData )
};