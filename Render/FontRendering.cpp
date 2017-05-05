#include "stdafx.h"
#include "FontRendering.h"

CFontObject::CFontObject( CFontFile* pFontFile, uint16 nSize, CDrawable2D* pDrawable, CDrawable2D* pOcclusionDrawable, const CRectangle& rect, uint8 nAlignment, bool bGUI, bool bInvertY )
	: m_pColorDrawable( bGUI ? NULL : pDrawable ), m_pOcclusionDrawable( bGUI ? NULL : pOcclusionDrawable ), m_pGUIDrawable( bGUI ? pDrawable : NULL )
	, m_pFontFile( pFontFile )
	, m_nAlignment( nAlignment )
	, m_bInvertY( bInvertY )
	, m_bEditMode( false )
	, m_bMultiLine( false )
	, m_editOfs( 0, 0 )
	, m_cursorPos( 0, 0 )
	, m_selectPos( 0, 0 )
	, m_bGlobalClipValid( false )
	, m_globalClip( 0, 0, 0, 0 )
	, m_color( 1, 1, 1, 1 )
	, m_selectionColor( 0, 0, 0.5, 1 )
{
	m_element2D.rect = rect;
	m_element2D.pInstData = this;
	m_localBound = rect;
	m_pFont = pFontFile->GetFont( nSize );
}

void CFontObject::SetSize( CFontFile* pFontFile, uint16 nSize )
{
	if( pFontFile == m_pFontFile && nSize == m_pFont->GetSize() )
		return;
	wstring str = m_strText;
	SetText( L"" );

	m_pFontFile = pFontFile;
	m_pFont = pFontFile->GetFont( nSize );
	SetText( str.c_str() );
}

void CFontObject::SetRect( const CRectangle& rect )
{
	m_localBound = rect;
	SetText( m_strText.c_str() );
}

void CFontObject::SetAlignment( uint8 nAlignment )
{
	m_nAlignment = nAlignment;
	SetText( m_strText.c_str() );
}

CFontObject* CFontObject::Clone()
{
	bool bGUI = m_pGUIDrawable ? true : false;
	CFontObject* pFontObject = new CFontObject( m_pFontFile, m_pFont->GetSize(),
		!bGUI ? m_pColorDrawable : m_pGUIDrawable, !bGUI ? m_pOcclusionDrawable : NULL, m_localBound, m_nAlignment, bGUI, m_bInvertY );
	pFontObject->SetMultiLine( m_bMultiLine );
	pFontObject->SetText( m_strText.c_str() );
	pFontObject->SetColor( m_color );
	pFontObject->SetSelectionColor( m_selectionColor );
	return pFontObject;
}

void CFontObject::SetText( const wchar_t* szText, int32 nCursorIndex )
{
	for( int i = 0; i < m_strText.length(); i++ )
		m_pFont->UnCache( m_strText[i] );

	if( !m_bMultiLine )
	{
		wstring str;
		for( const wchar_t* p = szText; *p; p++ )
		{
			if( *p != L'\r' && *p != L'\n' )
				str.push_back( *p );
		}
		m_strText = str;
	}
	else
		m_strText = szText;
	m_characters.resize( m_strText.length() );
	m_vecLineBeginIndex.clear();
	if( !m_strText.length() )
	{
		m_cursorPos.x = m_cursorPos.y = 0;

		m_editOfs.x = m_localBound.x;
		uint8 nAlignY = m_nAlignment >> 2;
		CRectangle bound( 0, -m_pFont->GetBaseLine() - m_pFont->GetSize(), 0, m_pFont->GetSize() );
		if( nAlignY == eAlign_Left )
			m_editOfs.y = m_localBound.y - bound.y;
		else if( nAlignY == eAlign_Right )
			m_editOfs.y = m_localBound.GetBottom() - bound.GetBottom();
		else
			m_editOfs.y = m_localBound.GetCenterY() - bound.GetCenterY();

		return;
	}

	CVector2 pen( 0, 0 );
	uint32 nColumn = 0;
	uint32 nLines = 0;
	uint32 nMaxLineWidth = 0;
	int i;
	for( int i = 0; i < m_strText.length(); i++ )
	{
		auto& character = m_characters[i];
		if( m_strText[i] == L'\n' || m_strText[i] == L'\r' )
		{
			character.rect = CRectangle( 0, 0, 0, 0 );
			character.hitRect = CRectangle( pen.x, pen.y - m_pFont->GetBaseLine() - m_pFont->GetSize(), 4, m_pFont->GetSize() );
			character.nColumn = nColumn;
			character.nRow = nLines;
			m_vecLineBeginIndex.push_back( i + 1 );
			nLines++;
			pen.y += m_pFont->GetSize();
			if( pen.x > nMaxLineWidth )
				nMaxLineWidth = pen.x;
			pen.x = 0;
			nColumn = 0;
			continue;
		}

		auto& characterInfo = m_pFont->Cache( m_strText[i] );
		character.rect = characterInfo.rect.Offset( pen );
		character.texRect = characterInfo.texRect;
		character.nTextureIndex = characterInfo.textureIndex;

		character.hitRect = CRectangle( pen.x, pen.y - m_pFont->GetBaseLine() - m_pFont->GetSize(), characterInfo.xAdvance, m_pFont->GetSize() );
		character.nColumn = nColumn;
		character.nRow = nLines;

		pen.x += characterInfo.xAdvance;
		nColumn++;
	}
	if( pen.x > nMaxLineWidth )
		nMaxLineWidth = pen.x;

	if( nCursorIndex >= 0 )
		SetCursor( nCursorIndex );

	CRectangle bound( 0, -m_pFont->GetBaseLine() - m_pFont->GetSize(), nMaxLineWidth, ( nLines + 1 ) * m_pFont->GetSize() );
	
	CRectangle textBound = m_localBound;
	if( !m_bInvertY )
		textBound.y = -textBound.height - textBound.y;
	CVector2 ofs( 0, 0 );
	if( !m_bEditMode )
	{
		uint8 nAlignX = m_nAlignment & 3;
		uint8 nAlignY = m_nAlignment >> 2;
		if( nAlignX == eAlign_Left )
			ofs.x = textBound.x - bound.x;
		else if( nAlignX == eAlign_Right )
			ofs.x = textBound.GetRight() - bound.GetRight();
		else
			ofs.x = textBound.GetCenterX() - bound.GetCenterX();
		if( nAlignY == eAlign_Left )
			ofs.y = textBound.y - bound.y;
		else if( nAlignY == eAlign_Right )
			ofs.y = textBound.GetBottom() - bound.GetBottom();
		else
			ofs.y = textBound.GetCenterY() - bound.GetCenterY();
		ofs.x = floor( ofs.x );
		ofs.y = floor( ofs.y );
	}
	else
	{
		ofs = m_editOfs;
		uint32 nCharacterBeginIndex = m_cursorPos.y ? m_vecLineBeginIndex[m_cursorPos.y - 1] : 0;
		float cursorX1 = m_cursorPos.x ? m_characters[nCharacterBeginIndex + m_cursorPos.x - 1].hitRect.GetRight() : 0;
		float cursorX0 = m_cursorPos.x > 1 ? m_characters[nCharacterBeginIndex + m_cursorPos.x - 2].hitRect.GetRight() : 0;
		float cursorY1 = m_cursorPos.y * m_pFont->GetSize() - m_pFont->GetBaseLine();
		float cursorY0 = cursorY1 - m_pFont->GetSize();

		if( cursorX1 + ofs.x > textBound.GetRight() )
		{
			ofs.x = textBound.GetRight() - cursorX1;
		}
		else if( cursorX0 + ofs.x < textBound.x )
		{
			ofs.x = textBound.x - cursorX0;
		}
		if( cursorY1 + ofs.y > textBound.GetBottom() )
		{
			ofs.y = textBound.GetBottom() - cursorY1;
		}
		else if( cursorY0 + ofs.y < textBound.y )
		{
			ofs.y = textBound.y - cursorY0;
		}
	}
	m_editOfs = ofs;

	for( i = 0; i < m_characters.size(); i++ )
	{
		CRectangle newRect = m_characters[i].rect.Offset( ofs );
		CRectangle clippedRect = newRect * textBound;
		CRectangle& texRect = m_characters[i].texRect;
		CRectangle clippedTexRect = CRectangle( texRect.x + texRect.width * ( clippedRect.x - newRect.x ) / newRect.width,
			texRect.y + texRect.height * ( clippedRect.y - newRect.y ) / newRect.height,
			texRect.width * clippedRect.width / newRect.width, texRect.height * clippedRect.height / newRect.height );
		m_characters[i].rect = clippedRect;
		m_characters[i].texRect = clippedTexRect;
		m_characters[i].hitRect = m_characters[i].hitRect.Offset( ofs ) * textBound;
		if( !m_bInvertY )
		{
			m_characters[i].rect.y = -m_characters[i].rect.height - m_characters[i].rect.y ;
			m_characters[i].hitRect.y = -m_characters[i].hitRect.height - m_characters[i].hitRect.y ;
		}
	}
}

void CFontObject::SetCursor( uint32 nIndex )
{
	if( !m_bEditMode )
		return;
	
	if( nIndex < m_characters.size() )
	{
		auto& character = m_characters[nIndex];
		m_cursorPos.x = character.nColumn;
		m_cursorPos.y = character.nRow;
	}
	else
	{
		m_cursorPos.y = m_vecLineBeginIndex.size();
		m_cursorPos.x = m_characters.size() - ( m_vecLineBeginIndex.size() ? m_vecLineBeginIndex.back() : 0 );
	}
}

TVector2<uint32> CFontObject::GetCursorPosByLocalPos( const CVector2& localPos )
{
	TVector2<uint32> cursorPos;
	cursorPos.y = floor( ( ( m_bInvertY ? localPos.y : -localPos.y ) - m_editOfs.y + m_pFont->GetBaseLine() + m_pFont->GetSize() ) / m_pFont->GetSize() );
	if( cursorPos.y > m_vecLineBeginIndex.size() )
	{
		cursorPos.y = m_vecLineBeginIndex.size();
		cursorPos.x = m_characters.size() - ( m_vecLineBeginIndex.size() ? m_vecLineBeginIndex.back() : 0 );
	}
	else
	{
		uint32 iIndex = cursorPos.y ? m_vecLineBeginIndex[cursorPos.y - 1] : 0;
		uint32 iIndex1;
		float cursorX = localPos.x;
		for( iIndex1 = iIndex; iIndex1 < m_characters.size(); iIndex1++ )
		{
			auto& character = m_characters[iIndex1];
			wchar_t ch = m_strText[iIndex1];
			if( ch == L'\n' || ch == L'\r' )
				break;
			if( character.hitRect.GetRight() > cursorX )
				break;
		}
		cursorPos.x = iIndex1 - iIndex;
	}
	return cursorPos;
}

CRectangle CFontObject::GetCursorShowRect()
{
	float yMin, yMax;
	yMax = m_cursorPos.y * m_pFont->GetSize() - m_pFont->GetBaseLine() + m_editOfs.y;
	yMin = yMax - m_pFont->GetSize();
	if( !m_bInvertY )
	{
		float fTemp = yMax;
		yMax = -yMin;
		yMin = -fTemp;
	}
	uint32 iIndex = m_cursorPos.y ? m_vecLineBeginIndex[m_cursorPos.y - 1] : 0;
	float xMin = m_cursorPos.x ? m_characters[iIndex + m_cursorPos.x - 1].hitRect.GetRight() : 0;
	return CRectangle( xMin, yMin, 1, yMax - yMin ) * m_localBound;
}

uint32 CFontObject::GetSelectRectCount()
{
	if( m_selectPos == m_cursorPos )
		return 0;

	TVector2<uint32> a, b;
	if( m_selectPos.y > m_cursorPos.y || m_selectPos.y == m_cursorPos.y && m_selectPos.x > m_cursorPos.x )
	{
		a = m_cursorPos;
		b = m_selectPos;
	}
	else
	{
		a = m_selectPos;
		b = m_cursorPos;
	}

	return b.y - a.y + ( b.x ? 1 : 0 );
}

CRectangle CFontObject::GetSelectRect( uint32 nIndex )
{
	TVector2<uint32> a, b;
	if( m_selectPos.y > m_cursorPos.y || m_selectPos.y == m_cursorPos.y && m_selectPos.x > m_cursorPos.x )
	{
		a = m_cursorPos;
		b = m_selectPos;
	}
	else
	{
		a = m_selectPos;
		b = m_cursorPos;
	}

	uint32 nRow = a.y + nIndex;

	uint32 nBeginIndex;
	if( nRow > a.y )
		nBeginIndex = GetLineBeginIndex( nRow );
	else
		nBeginIndex = GetLineBeginIndex( a.y ) + a.x;
	auto rect = m_characters[nBeginIndex].hitRect;

	uint32 nEndIndex;
	if( nRow < b.y )
		nEndIndex = nRow - 1 < GetLineCount() ? GetLineBeginIndex( nRow + 1 ) : m_characters.size();
	else
		nEndIndex = GetLineBeginIndex( b.y ) + b.x;
	rect.SetRight( m_characters[nEndIndex - 1].hitRect.GetRight() );
	rect = rect * m_localBound;

	return rect;
}

void CFontObject::BeginEdit( const CVector2& localPos )
{
	m_bEditMode = true;
	m_selectPos = m_cursorPos = GetCursorPosByLocalPos( localPos );
	SetText( m_strText.c_str(), -1 );
}

void CFontObject::Select( const CVector2 & localPos )
{
	m_bEditMode = true;
	m_cursorPos = GetCursorPosByLocalPos( localPos );
	SetText( m_strText.c_str(), -1 );
}

void CFontObject::EndEdit()
{
	m_bEditMode = false;
	SetText( m_strText.c_str() );
	m_cursorPos = m_selectPos = TVector2<uint32>( 0, 0 );
}

void CFontObject::Insert( const wchar_t* szText )
{
	if( !m_bEditMode )
		return;
	uint32 nIndex = ( m_cursorPos.y ? m_vecLineBeginIndex[m_cursorPos.y - 1] : 0 ) + m_cursorPos.x;
	uint32 nIndex1 = ( m_selectPos.y ? m_vecLineBeginIndex[m_selectPos.y - 1] : 0 ) + m_selectPos.x;
	if( nIndex > nIndex1 )
	{
		uint32 temp = nIndex;
		nIndex = nIndex1;
		nIndex1 = temp;
	}

	wstring str = m_strText;
	if( nIndex1 > nIndex )
		str.erase( nIndex, nIndex1 - nIndex );
	str.insert( nIndex, szText );

	uint32 nNewIndex = nIndex + wcslen( szText );
	SetText( str.c_str(), nNewIndex );
	m_selectPos = m_cursorPos;
}

void CFontObject::Delete()
{
	if( !m_bEditMode )
		return;
	if( m_cursorPos != m_selectPos )
	{
		Insert( L"" );
		return;
	}

	uint32 nIndex = ( m_cursorPos.y ? m_vecLineBeginIndex[m_cursorPos.y - 1] : 0 ) + m_cursorPos.x;
	if( nIndex >= m_strText.length() )
		return;
	wstring str = m_strText;
	str.erase( nIndex, 1 );
	SetText( str.c_str(), nIndex );
	m_selectPos = m_cursorPos;
}

void CFontObject::Backspace()
{
	if( !m_bEditMode )
		return;
	if( m_cursorPos != m_selectPos )
	{
		Insert( L"" );
		return;
	}

	uint32 nIndex = ( m_cursorPos.y ? m_vecLineBeginIndex[m_cursorPos.y - 1] : 0 ) + m_cursorPos.x;
	if( !nIndex )
		return;
	wstring str = m_strText;
	str.erase( nIndex - 1, 1 );
	SetText( str.c_str(), nIndex - 1 );
	m_selectPos = m_cursorPos;
}

wstring CFontObject::GetSelectedText()
{
	if( !m_bEditMode )
		return L"";
	uint32 nIndex = ( m_cursorPos.y ? m_vecLineBeginIndex[m_cursorPos.y - 1] : 0 ) + m_cursorPos.x;
	uint32 nIndex1 = ( m_selectPos.y ? m_vecLineBeginIndex[m_selectPos.y - 1] : 0 ) + m_selectPos.x;
	if( nIndex > nIndex1 )
	{
		uint32 temp = nIndex;
		nIndex = nIndex1;
		nIndex1 = temp;
	}

	wstring str = m_strText;
	return str.substr( nIndex, nIndex1 - nIndex );
}

void CFontObject::Render( CRenderContext2D& context )
{
	m_pFont->UpdateTexture( context.pRenderSystem );
	switch( context.eRenderPass )
	{
	case eRenderPass_Color:
		if( m_pColorDrawable )
		{
			m_element2D.SetDrawable( m_pColorDrawable );
			context.AddElement( &m_element2D );
		}
		else if( m_pGUIDrawable )
		{
			m_element2D.SetDrawable( m_pGUIDrawable );
			context.AddElement( &m_element2D, 1 );
		}
		break;
	case eRenderPass_Occlusion:
		if( m_pOcclusionDrawable )
		{
			m_element2D.SetDrawable( m_pOcclusionDrawable );
			context.AddElement( &m_element2D );
		}
		break;
	default:
		break;
	}
}

void CFontDrawable::LoadXml( TiXmlElement* pRoot )
{
	CDefaultDrawable2D::LoadXml( pRoot );
	IShader* pPS = m_material.GetShader( EShaderType::PixelShader );
	if( pPS )
	{
		auto& info = pPS->GetShaderInfo();
		info.Bind( m_paramTex, "Texture0" );
	}
}

void CFontDrawable::Flush( CRenderContext2D& context )
{
	IRenderSystem* pRenderSystem = context.pRenderSystem;
	pRenderSystem->SetBlendState( m_pBlendState );
	m_material.Apply( context );
	OnApplyMaterial( context );

	pRenderSystem->SetVertexBuffer( 0, CGlobalRenderResources::Inst()->GetVBQuad() );
	pRenderSystem->SetIndexBuffer( CGlobalRenderResources::Inst()->GetIBQuad() );
	
	uint32 nMaxInst = m_material.GetMaxInst();
	uint32 nExtraInstData = m_material.GetExtraInstData();
	uint32 nInstStride = ( 2 + nExtraInstData ) * sizeof( CVector4 );
	float* fInstData = (float*)alloca( nMaxInst * nInstStride );
	
	uint32 nLastElemRenderedCount = 0;
	bool bShowCursor = (int)( floor( context.pRenderSystem->GetTotalTime() / 0.25 ) ) & 1;
	while( m_pElement )
	{
		uint32 i1 = 0;
		uint32 nOfs = 0;
		ITexture* pTexture = NULL;
		while( m_pElement && i1 < nMaxInst )
		{
			CElement2D* pElement = m_pElement;
			context.pCurElement = pElement;
			CFontObject* pFontObject = (CFontObject*)pElement->pInstData;

			CFontObject::SCharacterRenderInfo tempCharacter;
			tempCharacter.texRect = pFontObject->m_pFont->GetFirstBlockRect();
			tempCharacter.nTextureIndex = -1;
			auto& characters = pFontObject->m_characters;
			uint32 nSelectRectCount = pFontObject->GetSelectRectCount();
			bool bCursor = pFontObject->IsEdit() && bShowCursor;
			uint32 nElemCount = characters.size() + nSelectRectCount + ( bCursor ? 1 : 0 );

			uint32 iBegin = nLastElemRenderedCount;
			uint32 iEnd = nElemCount;
			if( iEnd - iBegin + i1 > nMaxInst )
			{
				iEnd = nMaxInst + iBegin - i1;
				nLastElemRenderedCount = iEnd;
			}
			else
				nLastElemRenderedCount = 0;
			float fDepth = pElement->depth;

			for( ; iBegin < iEnd; iBegin++ )
			{
				auto& character = iBegin >= nSelectRectCount && iBegin < nSelectRectCount + characters.size() ?
					characters[iBegin - nSelectRectCount] : tempCharacter;
				CVector4 color = pFontObject->m_color;
				if( iBegin < nSelectRectCount )
				{
					tempCharacter.rect = pFontObject->GetSelectRect( iBegin );
					color = pFontObject->m_selectionColor;
				}
				else if( iBegin >= nSelectRectCount + characters.size() )
				{
					tempCharacter.rect = pFontObject->GetCursorShowRect();
				}

				if( character.rect.width <= 0 || character.rect.height <= 0 )
					continue;

				if( character.nTextureIndex != -1 )
				{
					ITexture* pTex = pFontObject->m_pFont->GetTexture( character.nTextureIndex );
					if( pTexture )
					{
						if( pTex != pTexture )
							break;
					}
					else
						pTexture = pTex;
				}
				else
				{
					if( !pTexture )
						pTexture = pFontObject->m_pFont->GetTexture( 0 );
				}

				float* pData = fInstData + nOfs;
				if( pFontObject->m_bGlobalClipValid )
				{
					CRectangle newRect = character.rect.Offset( pFontObject->globalTransform.GetPosition() );
					CRectangle clippedRect = newRect * pFontObject->m_globalClip;
					if( clippedRect.width <= 0 || clippedRect.height <= 0 )
					{
						continue;
					}
					CRectangle& texRect = character.texRect;
					CRectangle clippedTexRect = CRectangle( texRect.x + texRect.width * ( clippedRect.x - newRect.x ) / newRect.width,
						texRect.y + texRect.height * ( clippedRect.y - newRect.y ) / newRect.height,
						texRect.width * clippedRect.width / newRect.width, texRect.height * clippedRect.height / newRect.height );

					*pData++ = clippedRect.GetCenterX();
					*pData++ = clippedRect.GetCenterY();
					*pData++ = clippedRect.GetSizeX() / 2;
					*pData++ = 0;
					
					uint32 texX = clippedTexRect.x * 2048;
					float dTexX = 1.0 - clippedTexRect.width;
					uint32 texY = clippedTexRect.y * 2048;
					float dTexY = 1.0 - clippedTexRect.height;
					*pData++ = texX + dTexX;
					*pData++ = texY + dTexY;

					float ratio = clippedRect.GetSizeY() / clippedRect.GetSizeX();
					*pData++ = ratio;
					*pData++ = pElement->depth;
				}
				else
				{
					CMatrix2D mat( character.rect.GetSizeX() / 2, 0, character.rect.GetCenterX(),
						0, character.rect.GetSizeY() / 2, character.rect.GetCenterY(),
						0, 0, 1 );
					mat = pFontObject->globalTransform * mat;

					*pData++ = mat.m02;
					*pData++ = mat.m12;
					*pData++ = mat.m00;
					*pData++ = mat.m10;

					uint32 texX = character.texRect.x * 2048;
					float dTexX = 1.0 - character.texRect.width;
					uint32 texY = character.texRect.y * 2048;
					float dTexY = 1.0 - character.texRect.height;
					*pData++ = texX + dTexX;
					*pData++ = texY + dTexY;

					float fMax1 = fabsf( mat.m00 ) > fabsf( mat.m10 )? mat.m00: mat.m10;
					float fMax2 = fabsf( mat.m00 ) > fabsf( mat.m10 )? mat.m11: -mat.m01;
					float ratio = fMax2 / fMax1;
					*pData++ = ratio;
					*pData++ = pElement->depth;
				}

				if( nExtraInstData )
					memcpy( pData, &color, sizeof( CVector4 ) );

				i1++;
				nOfs += 4 * ( 2 + nExtraInstData );
			}
			if( iBegin < iEnd )
			{
				nLastElemRenderedCount = iBegin;
				break;
			}

			if( !nLastElemRenderedCount )
				pElement->OnFlushed();
		}

		if( i1 > 0 )
		{
			uint32 nInstanceDataSize = i1 * nInstStride;
			context.pInstanceDataSize = &nInstanceDataSize;
			context.ppInstanceData = (void**)&fInstData;
			if( pTexture )
				m_paramTex.Set( context.pRenderSystem, pTexture->GetShaderResource() );
			m_material.ApplyPerInstance( context );
			pRenderSystem->DrawInputInstanced( i1 );
		}
	}

	m_material.UnApply( context );
}