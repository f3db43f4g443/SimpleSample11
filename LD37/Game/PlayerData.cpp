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
	bFinishedTutorial = buf.Read<uint8>();
}

void CPlayerData::Save()
{
	string strFileName = "Save/Profiles/";
	strFileName += strPlayerName;

	CBufFile buf;
	buf.Write<uint8>( bFinishedTutorial );
	SaveFile( strFileName.c_str(), buf.GetBuffer(), buf.GetBufLen() );
}
