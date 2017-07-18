#pragma once
#include <string>
using namespace std;

class CPlayerData
{
public:
	CPlayerData() : bFinishedTutorial( false ), bIsDesign( false ), nPassedLevels( 0 ), nShakeType( 2 ) {}
	string strPlayerName;
	bool bIsDesign;

	uint8 bFinishedTutorial;
	int32 nPassedLevels;

	int32 nShakeType;

	void Load( const char* szPlayer );
	void Save();

	DECLARE_GLOBAL_INST_REFERENCE( CPlayerData )
};