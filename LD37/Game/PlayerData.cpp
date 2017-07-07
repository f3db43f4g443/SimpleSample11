#include "stdafx.h"
#include "PlayerData.h"
#include "FileUtil.h"
#include "BufFile.h"

void CPlayerData::Load( const char * szPlayer )
{
	strPlayerName = szPlayer;
	string strFileName = "Save/Profiles/";
	strFileName += szPlayer;
	
	vector<int8> result;
	if( GetFileContent( result, strFileName.c_str(), false ) == INVALID_32BITID )
		return;

	CBufReader buf( &result[0], result.size() );
	buf.Read( bFinishedTutorial );
	buf.Read( nPassedLevels );
}

void CPlayerData::Save()
{
	string strFileName = "Save/Profiles/";
	strFileName += strPlayerName;

	CBufFile buf;
	buf.Write( bFinishedTutorial );
	buf.Write( nPassedLevels );
	SaveFile( strFileName.c_str(), buf.GetBuffer(), buf.GetBufLen() );
}
