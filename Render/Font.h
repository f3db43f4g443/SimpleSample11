#pragma once
#include "Resource.h"
#include "RenderSystem.h"
#include "Math3D.h"
#include <vector>
using namespace std;

struct SCharacterInfo
{
	SCharacterInfo() { memset( this, 0, sizeof( *this ) ); }
	/** The character this entry is for */
	uint16 character;
	/** Scale that was applied when rendering this character */
	float fontScale;

	CRectangle rect;
	CRectangle texRect;
	/** The largest vertical distance below the baseline for any character in the font */
	int16 globalDescender;
	/** The amount to advance in X before drawing the next character in a string */
	int16 xAdvance;
	/** Index to a specific texture in the font cache. */
	uint8 textureIndex;
	/** 1 if this entry has kerning, 0 otherwise. */
	bool hasKerning;
	/** 1 if this entry is valid, 0 otherwise. */
	bool valid;

	uint32 nCacheCount;
};

struct SFontFileData;
class CFont;
class CFontFile : public CResource
{
public:
	enum EType
	{
		eResType = eEngineResType_Font,
	};

	CFontFile( const char* name, int32 type ) : CResource( name, type ) {}
	~CFontFile();
	void Create();

	SFontFileData* GetData() { return m_pData; }
	CFont* GetFont( uint16 nSize );

	static void Init();
private:
	string m_strDesc;
	map<uint16, CFont*> m_mapFonts;
	SFontFileData* m_pData;
};

class CFont
{
public:
	CFont( CFontFile* pFontFile, uint16 nSize );

	uint16 GetSize() { return m_nSize; }
	int16 GetBaseLine() { return m_nBaseLine; }

	SCharacterInfo& Cache( uint16 nCharacter );
	void UnCache( uint16 nCharacter );
	SCharacterInfo& GetCharacter( uint16 nCharacter );
	ITexture* GetTexture( uint16 nIndex );

	void UpdateTexture( IRenderSystem* pRenderSystem );
private:
	struct SUpdateData
	{
		TRectangle<uint32> rect;
		vector<uint8> data;
		uint32 nTexture;
		LINK_LIST( SUpdateData, UpdateData );
	};

	SCharacterInfo& AddCharacter( uint16 nCharacter );

	void AllocTexture();

	CFontFile* m_pFontFile;
	uint16 m_nSize;
	int16 m_nBaseLine;

	vector<CReference<ITexture> > m_vecTextures;
	vector<SCharacterInfo> m_vecCharacterInfo;
	map<uint16, SCharacterInfo> m_mapCharacterInfo;

	uint16 m_nAtlasSlotPerRow;

	uint16 m_nMaxSlot;
	vector<uint16> m_vecFreeSlot;

	LINK_LIST_HEAD( m_pUpdateData, SUpdateData, UpdateData )
};