#include "Platform_Common.h"
#ifdef _STEAM
#include "steam/steam_api.h"
#include "Game/SdkInterface.h"
#include <string>
#include <map>
using namespace std;

class CGID
{
public:

	CGID()
	{
		m_gameID.m_nType = k_EGameIDTypeApp;
		m_gameID.m_nAppID = k_uAppIdInvalid;
		m_gameID.m_nModID = 0;
	}

	explicit CGID( uint64 ulGameID )
	{
		m_ulGameID = ulGameID;
	}
#ifdef INT64_DIFFERENT_FROM_INT64_T
	CGameID( uint64_t ulGameID )
	{
		m_ulGameID = (uint64)ulGameID;
	}
#endif

	explicit CGID( int32 nAppID )
	{
		m_ulGameID = 0;
		m_gameID.m_nAppID = nAppID;
	}

	explicit CGID( uint32 nAppID )
	{
		m_ulGameID = 0;
		m_gameID.m_nAppID = nAppID;
	}

	CGID( uint32 nAppID, uint32 nModID )
	{
		m_ulGameID = 0;
		m_gameID.m_nAppID = nAppID;
		m_gameID.m_nModID = nModID;
		m_gameID.m_nType = k_EGameIDTypeGameMod;
	}

	CGID( const CGID &that )
	{
		m_ulGameID = that.m_ulGameID;
	}

	CGID& operator=( const CGID & that )
	{
		m_ulGameID = that.m_ulGameID;
		return *this;
	}

	// Hidden functions used only by Steam
	explicit CGID( const char *pchGameID );
	const char *Render() const;					// render this Game ID to string
	static const char *Render( uint64 ulGameID );		// static method to render a uint64 representation of a Game ID to a string

														// must include checksum_crc.h first to get this functionality
#if defined( CHECKSUM_CRC_H )
	CGameID( uint32 nAppID, const char *pchModPath )
	{
		m_ulGameID = 0;
		m_gameID.m_nAppID = nAppID;
		m_gameID.m_nType = k_EGameIDTypeGameMod;

		char rgchModDir[MAX_PATH];
		V_FileBase( pchModPath, rgchModDir, sizeof( rgchModDir ) );
		CRC32_t crc32;
		CRC32_Init( &crc32 );
		CRC32_ProcessBuffer( &crc32, rgchModDir, V_strlen( rgchModDir ) );
		CRC32_Final( &crc32 );

		// set the high-bit on the mod-id 
		// reduces crc32 to 31bits, but lets us use the modID as a guaranteed unique
		// replacement for appID's
		m_gameID.m_nModID = crc32 | ( 0x80000000 );
	}

	CGameID( const char *pchExePath, const char *pchAppName )
	{
		m_ulGameID = 0;
		m_gameID.m_nAppID = k_uAppIdInvalid;
		m_gameID.m_nType = k_EGameIDTypeShortcut;

		CRC32_t crc32;
		CRC32_Init( &crc32 );
		if( pchExePath )
			CRC32_ProcessBuffer( &crc32, pchExePath, V_strlen( pchExePath ) );
		if( pchAppName )
			CRC32_ProcessBuffer( &crc32, pchAppName, V_strlen( pchAppName ) );
		CRC32_Final( &crc32 );

		// set the high-bit on the mod-id 
		// reduces crc32 to 31bits, but lets us use the modID as a guaranteed unique
		// replacement for appID's
		m_gameID.m_nModID = crc32 | ( 0x80000000 );
	}

#if defined( VSTFILEID_H )

	CGameID( VstFileID vstFileID )
	{
		m_ulGameID = 0;
		m_gameID.m_nAppID = k_uAppIdInvalid;
		m_gameID.m_nType = k_EGameIDTypeP2P;

		CRC32_t crc32;
		CRC32_Init( &crc32 );
		const char *pchFileId = vstFileID.Render();
		CRC32_ProcessBuffer( &crc32, pchFileId, V_strlen( pchFileId ) );
		CRC32_Final( &crc32 );

		// set the high-bit on the mod-id 
		// reduces crc32 to 31bits, but lets us use the modID as a guaranteed unique
		// replacement for appID's
		m_gameID.m_nModID = crc32 | ( 0x80000000 );
	}

#endif /* VSTFILEID_H */

#endif /* CHECKSUM_CRC_H */


	uint64 ToUint64() const
	{
		return m_ulGameID;
	}

	uint64 *GetUint64Ptr()
	{
		return &m_ulGameID;
	}

	void Set( uint64 ulGameID )
	{
		m_ulGameID = ulGameID;
	}

	bool IsMod() const
	{
		return ( m_gameID.m_nType == k_EGameIDTypeGameMod );
	}

	bool IsShortcut() const
	{
		return ( m_gameID.m_nType == k_EGameIDTypeShortcut );
	}

	bool IsP2PFile() const
	{
		return ( m_gameID.m_nType == k_EGameIDTypeP2P );
	}

	bool IsSteamApp() const
	{
		return ( m_gameID.m_nType == k_EGameIDTypeApp );
	}

	uint32 ModID() const
	{
		return m_gameID.m_nModID;
	}

	uint32 AppID() const
	{
		return m_gameID.m_nAppID;
	}

	bool operator == ( const CGID &rhs ) const
	{
		return m_ulGameID == rhs.m_ulGameID;
	}

	bool operator != ( const CGID &rhs ) const
	{
		return !( *this == rhs );
	}

	bool operator < ( const CGID &rhs ) const
	{
		return ( m_ulGameID < rhs.m_ulGameID );
	}

	bool IsValid() const
	{
		// each type has it's own invalid fixed point:
		switch( m_gameID.m_nType )
		{
		case k_EGameIDTypeApp:
			return m_gameID.m_nAppID != k_uAppIdInvalid;

		case k_EGameIDTypeGameMod:
			return m_gameID.m_nAppID != k_uAppIdInvalid && m_gameID.m_nModID & 0x80000000;

		case k_EGameIDTypeShortcut:
			return ( m_gameID.m_nModID & 0x80000000 ) != 0;

		case k_EGameIDTypeP2P:
			return m_gameID.m_nAppID == k_uAppIdInvalid && m_gameID.m_nModID & 0x80000000;

		default:
			return false;
		}

	}

	void Reset()
	{
		m_ulGameID = 0;
	}



private:

	enum EGameIDType
	{
		k_EGameIDTypeApp = 0,
		k_EGameIDTypeGameMod = 1,
		k_EGameIDTypeShortcut = 2,
		k_EGameIDTypeP2P = 3,
	};

	struct GameID_t
	{
#ifdef VALVE_BIG_ENDIAN
		unsigned int m_nModID : 32;
		unsigned int m_nType : 8;
		unsigned int m_nAppID : 24;
#else
		unsigned int m_nAppID : 24;
		unsigned int m_nType : 8;
		unsigned int m_nModID : 32;
#endif
	};

	union
	{
		uint64 m_ulGameID;
		GameID_t m_gameID;
	};
};

class CSteamSdkInterface : public CDefaultSdkInterface
{
public:
	CSteamSdkInterface()
		: m_GameID( SteamUtils()->GetAppID() )
		, m_CallbackUserStatsReceived( this, &CSteamSdkInterface::OnUserStatsReceived )
		, m_CallbackUserStatsStored( this, &CSteamSdkInterface::OnUserStatsStored )
		, m_CallbackAchievementStored( this, &CSteamSdkInterface::OnAchievementStored )
		, m_nCallBackInterval( 0 )
	{
		m_pSteamUser = SteamUser();
		m_pSteamUserStats = SteamUserStats();
		m_bRequestedStats = false;
		m_bStatsValid = false;
		m_bStoreStats = false;
	}

	virtual void Update() override
	{
		m_nCallBackInterval++;
		if( m_nCallBackInterval >= 10 )
		{
			m_nCallBackInterval = 0;
			SteamAPI_RunCallbacks();
		}
		if( !m_bRequestedStats )
		{
			if( NULL == m_pSteamUserStats || NULL == m_pSteamUser )
			{
				m_bRequestedStats = true;
				return;
			}

			bool bSuccess = m_pSteamUserStats->RequestCurrentStats();
			m_bRequestedStats = bSuccess;
		}

		if( !m_bStatsValid )
			return;

		CheckAchievements();
		if( m_bStoreStats )
		{
			bool bSuccess = m_pSteamUserStats->StoreStats();
			m_bStoreStats = !bSuccess;
		}
	}

	virtual void UnlockAchievement( const char* sz ) override
	{
		if( m_unlockedAchievements.find( sz ) != m_unlockedAchievements.end() )
			return;
		if( !m_bStatsValid )
		{
			m_unlockedAchievements[sz] = false;
			return;
		}
		m_pSteamUserStats->GetAchievement( sz, &m_unlockedAchievements[sz] );
	}

	STEAM_CALLBACK( CSteamSdkInterface, OnUserStatsReceived, UserStatsReceived_t, m_CallbackUserStatsReceived );
	STEAM_CALLBACK( CSteamSdkInterface, OnUserStatsStored, UserStatsStored_t, m_CallbackUserStatsStored );
	STEAM_CALLBACK( CSteamSdkInterface, OnAchievementStored, UserAchievementStored_t, m_CallbackAchievementStored );

	static CSteamSdkInterface& Inst() { static CSteamSdkInterface g_inst; return g_inst; }
private:
	void CheckAchievements()
	{
		for( auto& item : m_unlockedAchievements )
		{
			if( item.second )
				continue;
			m_pSteamUserStats->SetAchievement( item.first.c_str() );
			item.second = true;
			m_bStoreStats = true;
		}
	}
	ISteamUser *m_pSteamUser;
	ISteamUserStats *m_pSteamUserStats;
	CGID m_GameID;

	map<string, bool> m_unlockedAchievements;

	bool m_bRequestedStats;
	bool m_bStatsValid;
	bool m_bStoreStats;
	int32 m_nCallBackInterval;
};

void CSteamSdkInterface::OnUserStatsReceived( UserStatsReceived_t *pCallback )
{
	if( !m_pSteamUserStats )
		return;

	// we may get callbacks for other games' stats arriving, ignore them
	if( m_GameID.ToUint64() == pCallback->m_nGameID )
	{
		if( k_EResultOK == pCallback->m_eResult )
		{
			m_bStatsValid = true;
			for( auto& item : m_unlockedAchievements )
			{
				m_pSteamUserStats->GetAchievement( item.first.c_str(), &item.second );
			}
		}
		else
		{
			//failed
		}
	}
}

void CSteamSdkInterface::OnUserStatsStored( UserStatsStored_t *pCallback )
{
	// we may get callbacks for other games' stats arriving, ignore them
	if( m_GameID.ToUint64() == pCallback->m_nGameID )
	{
		if( k_EResultOK == pCallback->m_eResult )
		{
			//OK
		}
		else if( k_EResultInvalidParam == pCallback->m_eResult )
		{
			// One or more stats we set broke a constraint. They've been reverted,
			// and we should re-iterate the values now to keep in sync.
			// Fake up a callback here so that we re-load the values.
			UserStatsReceived_t callback;
			callback.m_eResult = k_EResultOK;
			callback.m_nGameID = m_GameID.ToUint64();
			OnUserStatsReceived( &callback );
		}
		else
		{
			//failed
		}
	}
}

void CSteamSdkInterface::OnAchievementStored( UserAchievementStored_t *pCallback )
{
	// we may get callbacks for other games' stats arriving, ignore them
	if( m_GameID.ToUint64() == pCallback->m_nGameID )
	{
		if( 0 == pCallback->m_nMaxProgress )
		{
			//unlocked
		}
		else
		{
			//progress
		}
	}
}

void Init_PlatformSDK()
{
	if( SteamAPI_Init() )
		ISdkInterface::Init( &CSteamSdkInterface::Inst() );
	else
	{
		static CDefaultSdkInterface g_inst;
		ISdkInterface::Init( &g_inst );
	}
}
void Shutdown_PlatformSDK()
{
	SteamAPI_Shutdown();
}

#endif