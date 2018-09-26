#pragma once
#include "Drawable2D.h"
#include "Material.h"
#include "BufFile.h"

class TiXmlElement;
class CDefaultDrawable2D : public CDrawable2D
{
	friend class CMaterialEditor;
public:
	CDefaultDrawable2D();
	~CDefaultDrawable2D();

	void Load( IBufReader& buf );
	void Save( CBufFile& buf );

	void LoadXml( TiXmlElement* pRoot );
	void BindShaderResource( EShaderType eShaderType, const char* szName, IShaderResourceProxy* pShaderResource );
	void Flush( CRenderContext2D& context );

	vector<CReference<CResource> >& GetDependentResources() { return m_material.GetDependentResources(); }
protected:
	virtual void OnApplyMaterial( CRenderContext2D& context ) {}
	virtual bool OnFlushElement( CRenderContext2D& context, CElement2D* pElement, bool bBreak ) { return false; }
	IBlendState* m_pBlendState;
	CMaterial m_material;
	CMaterial m_material1;
};
