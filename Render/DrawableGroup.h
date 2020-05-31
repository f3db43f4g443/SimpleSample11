#pragma once
#include "Resource.h"
#include "Drawable2D.h"
#include "Image2D.h"
#include "TileMap2D.h"

class CDrawableGroup : public CResource
{
	friend class CMaterialEditor;
public:
	enum EType
	{
		eResType = eEngineResType_DrawableGroup,
	};
	CDrawableGroup( const char* name, int32 type ) : CResource( name, type ), m_nParamCount( 0 )
		, m_colorDrawable( this ), m_occlusionDrawable( this ), m_guiDrawable( this ) {}
	void Create();
	void Clear()
	{
		m_nParamCount = 0;
		m_colorDrawable.Clear();
		m_occlusionDrawable.Clear();
		m_guiDrawable.Clear();
		m_frameData.frames.clear();
		m_tileMapInfo.nWidth = m_tileMapInfo.nHeight = m_tileMapInfo.nTileCount = 0;
		m_tileMapInfo.params.clear();
		ClearDependency();
	}
	uint8 GetType() { return m_nType; }
	uint8 GetParamCount() { return m_nParamCount; }

	void BindShaderResource( EShaderType eShaderType, const char* szName, IShaderResourceProxy* pShaderResource );

	enum
	{
		eType_Default,
		eType_Rope,
		eType_MultiFrame,
		eType_TileMap,
	};

	struct SDrawableInfo
	{
		SDrawableInfo( CDrawableGroup* pOwner ) : pOwner( pOwner ), pDrawable( NULL ), nParamBeginIndex( 0 ), nParamCount( 0 ) {}
		~SDrawableInfo() { Clear(); }
		void Clear()
		{
			if( pDrawable )
			{
				delete pDrawable;
				pDrawable = NULL;
			}
			nParamBeginIndex = 0;
			nParamCount = 0;
		}
		CDrawableGroup* pOwner;
		CDrawable2D* pDrawable;
		uint16 nParamBeginIndex;
		uint16 nParamCount;
		void Load( IBufReader& buf );
		void Save( CBufFile& buf );
	};

	void Load( IBufReader& buf );
	void Save( CBufFile& buf );

	CDrawable2D* GetColorDrawable() { return m_colorDrawable.pDrawable; }
	CDrawable2D* GetOcclusionDrawable() { return m_occlusionDrawable.pDrawable; }
	CDrawable2D* GetGUIDrawable() { return m_guiDrawable.pDrawable; }

	CRenderObject2D* CreateInstance( bool bForceCreateStatic = false );

	void UpdateDependencies();
private:
	SDrawableInfo m_colorDrawable;
	SDrawableInfo m_occlusionDrawable;
	SDrawableInfo m_guiDrawable;
	uint8 m_nParamCount;
	uint8 m_nType;

	CRectangle m_defaultRect;
	CRectangle m_defaultTexRect;
	vector<CVector4> m_defaultParams;

	SImage2DFrameData m_frameData;
	STileMapInfo m_tileMapInfo;
};