#include "stdafx.h"
#include "UITextBox.h"
#include "Common/SystemUtil.h"
#include "Common/Utf8Util.h"

void CUITextBox::OnStartDrag( const CVector2 & mousePos )
{
	m_pTextObject->BeginEdit( mousePos - m_pTextObject->globalTransform.GetPosition() );
	CUILabel::OnStartDrag( mousePos );
}

void CUITextBox::OnDragged( const CVector2 & mousePos )
{
	m_pTextObject->Select( mousePos - m_pTextObject->globalTransform.GetPosition() );
	CUILabel::OnDragged( mousePos );
}

void CUITextBox::OnSetFocused( bool bFocused )
{
	CUILabel::OnSetFocused( bFocused );
	if( !bFocused )
	{
		m_pTextObject->EndEdit();
		m_events.Trigger( eEvent_Action, NULL );
	}
}

void CUITextBox::OnChar( uint32 nChar )
{
	switch( nChar )
    {
		case VK_BACK:
			m_pTextObject->Backspace();
			break;
		case 127:
			m_pTextObject->Delete();
			break;

        case 24:
		{
			SetClipboard( UnicodeToUtf8( m_pTextObject->GetSelectedText().c_str() ).c_str() );
			m_pTextObject->Insert( L"" );
			break;
		}
		case VK_CANCEL: // Ctrl-C Copy
		{
			SetClipboard( UnicodeToUtf8( m_pTextObject->GetSelectedText().c_str() ).c_str() );
			break;
		}
        case 22:
        {
			string str = GetClipboard();
			if( str.length() )
				m_pTextObject->Insert( Utf8ToUnicode( str.c_str() ).c_str() );
			break;
        }

            // Ctrl-A Select All
        case 1:
            break;

		case VK_RETURN:
		{
			wchar_t str[2] = L"\n";
			m_pTextObject->Insert( str );
			break;
		}

            // Junk characters we don't want in the string
        case 26:  // Ctrl Z
        case 2:   // Ctrl B
        case 14:  // Ctrl N
        case 19:  // Ctrl S
        case 4:   // Ctrl D
        case 6:   // Ctrl F
        case 7:   // Ctrl G
        case 10:  // Ctrl J
        case 11:  // Ctrl K
        case 12:  // Ctrl L
        case 17:  // Ctrl Q
        case 23:  // Ctrl W
        case 5:   // Ctrl E
        case 18:  // Ctrl R
        case 20:  // Ctrl T
        case 25:  // Ctrl Y
        case 21:  // Ctrl U
        case 9:   // Ctrl I
        case 15:  // Ctrl O
        case 16:  // Ctrl P
        case 27:  // Ctrl [
        case 29:  // Ctrl ]
        case 28:  // Ctrl \ 
            break;

        default:
        {
			wchar_t str[2] = { nChar, 0 };
			m_pTextObject->Insert( str );
        }
    }

}