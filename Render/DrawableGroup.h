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
	enum
	{
		eVer_Begin = 8,

		eVer_End,
		eVer_Cur = eVer_End - 1,
	};
	enum
	{
		eDrawable_Color,
		eDrawable_Occ,
		eDrawable_GUI,
	};
	
	CDrawableGroup( const char* name, int32 type );
	~CDrawableGroup();
	void Create();
	void Clear();
	void SetDrawableCount( int8 n );
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
		SDrawableInfo() : pOwner( NULL ), pDrawable( NULL ), nParamBeginIndex( 0 ), nParamCount( 0 ) {}
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

	CDrawable2D* GetColorDrawable() { return m_vecDrawables.size() > eDrawable_Color ? m_vecDrawables[eDrawable_Color].pDrawable : NULL; }
	CDrawable2D* GetOcclusionDrawable() { return m_vecDrawables.size() > eDrawable_Color ? m_vecDrawables[eDrawable_Occ].pDrawable : NULL; }
	CDrawable2D* GetGUIDrawable() { return m_vecDrawables.size() > eDrawable_Color ? m_vecDrawables[eDrawable_GUI].pDrawable : NULL; }

	CRenderObject2D* CreateInstance( bool bForceCreateStatic = false );

	void UpdateDependencies();
private:
	vector<SDrawableInfo> m_vecDrawables;
	uint8 m_nParamCount;
	uint8 m_nType;

	CRectangle m_defaultRect;
	CRectangle m_defaultTexRect;
	vector<CVector4> m_defaultParams;

	SImage2DFrameData m_frameData;
	STileMapInfo m_tileMapInfo;
};