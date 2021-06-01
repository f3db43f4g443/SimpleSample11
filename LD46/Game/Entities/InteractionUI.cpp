#include "stdafx.h"
#include "InteractionUI.h"
#include "MyGame.h"
#include "Common/Coroutine.h"
#include "Render/Image2D.h"
#include "UtilEntities.h"
#include "MyLevel.h"



class CInteractionUIScript : public CInteractionUI
{
	friend void RegisterGameClasses_InteractionUI();
public:
	CInteractionUIScript( const SClassCreateContext& context ) : CInteractionUI( context ) { SET_BASEOBJECT_ID( CInteractionUIScript ); }
	virtual void Init( CPawn* pPawn ) override
	{
		m_pLuaState = CLuaMgr::GetCurLuaState()->CreateCoroutine( m_strScript );
		m_pLuaState->PushLua( this );
		m_pLuaState->PushLua( pPawn );
		if( !m_pLuaState->Resume( 2, 0 ) )
			m_pLuaState = NULL;
	}
	virtual bool Update( CPawn* pPawn ) override
	{
		if( !m_pLuaState )
			return false;
		if( !m_pLuaState->Resume( 0, 0 ) )
		{
			m_pLuaState = NULL;
			return false;
		}
		return true;
	}
protected:
	CString m_strScript;
	
	CReference<CLuaState> m_pLuaState;
};

class CInteractionUICoroutine : public CInteractionUI
{
public:
	CInteractionUICoroutine( const SClassCreateContext& context ) : CInteractionUI( context ) {}
	virtual void Init( CPawn* pPawn ) override
	{
		m_pPawn = pPawn;
		struct _STemp
		{
			static uint32 Func( void* pThis )
			{
				try
				{
					( (CInteractionUICoroutine*)pThis )->Func();
				}
				catch( int e )
				{
				}
				return 1;
			}
		};
		m_pCoroutine = TCoroutinePool<0x10000>::Inst().Alloc();
		m_pCoroutine->Create( &_STemp::Func, this );
		m_pCoroutine->Resume();
		if( m_pCoroutine->GetState() == ICoroutine::eState_Stopped )
		{
			TCoroutinePool<0x10000>::Inst().Free( m_pCoroutine );
			m_pCoroutine = NULL;
		}
	}
	virtual bool Update( CPawn* pPawn ) override
	{
		if( m_pCoroutine )
		{
			m_pCoroutine->Resume();
			if( m_pCoroutine->GetState() == ICoroutine::eState_Stopped )
			{
				TCoroutinePool<0x10000>::Inst().Free( m_pCoroutine );
				m_pCoroutine = NULL;
			}
		}
		if( !m_pCoroutine )
			return false;
		return true;
	}
	virtual void OnRemovedFromStage() override
	{
		if( m_pCoroutine )
		{
			m_bEnd = true;
			m_pCoroutine->Resume();
			if( m_pCoroutine->GetState() == ICoroutine::eState_Stopped )
			{
				TCoroutinePool<0x10000>::Inst().Free( m_pCoroutine );
				m_pCoroutine = NULL;
			}
			m_pPawn = NULL;
		}
	}
protected:
	void Yield()
	{
		m_pCoroutine->Yield( 0 );
		if( m_bEnd )
			throw( 1 );
	}
	virtual void Func() {}

	ICoroutine* m_pCoroutine;
	CReference<CPawn> m_pPawn;
	bool m_bEnd;
};

class CCupboardInteractionUI : public CInteractionUICoroutine
{
	friend void RegisterGameClasses_InteractionUI();
public:
	CCupboardInteractionUI( const SClassCreateContext& context ) : CInteractionUICoroutine( context ) { SET_BASEOBJECT_ID( CCupboardInteractionUI ); }
protected:
	virtual void Func() override
	{
		int8 nRowType[3];
		int8 nRowState[3];
		int8 nEnemyState = -1;
		int8 nCursorX = 0, nCursorY = 0;
		nRowType[0] = 0;
		nRowType[1] = 1;
		nRowType[2] = 0;
		for( int i = 0; i < 3; i++ )
			nRowState[i] = nRowType[i] ? 1 : 2;

		auto UpdateCursor = [&, this] ()
		{
			int8 nCursor = nCursorX == 0 ? 1 : 2;
			if( nRowState[nCursorY] == 0 && nCursorX == 1 || nRowState[nCursorY] == 3 && nCursorX == 0 )
				nCursor = 0;
			for( int i = 0; i < 3; i++ )
			{
				m_pCursorImg[i]->bVisible = i == nCursor;
				if( i == nCursor )
					m_pCursorImg[i]->SetPosition( CVector2( nCursorX ? m_fHalfWidth * 0.5f : m_fHalfWidth * -0.5f, m_y0 + m_h * nCursorY ) );
			}
		};

		while( 1 )
		{
			while( 1 )
			{
				UpdateCursor();
				Yield();
				if( CGame::Inst().IsInputDown( eInput_Up ) )
					nCursorY = Min( 2, nCursorY + 1 );
				if( CGame::Inst().IsInputDown( eInput_Down ) )
					nCursorY = Max( 0, nCursorY - 1 );
				if( CGame::Inst().IsInputDown( eInput_Right ) )
					nCursorX = Min( 1, nCursorX + 1 );
				if( CGame::Inst().IsInputDown( eInput_Left ) )
					nCursorX = Max( 0, nCursorX - 1 );
				if( CGame::Inst().IsInputDown( eInput_A ) || CGame::Inst().IsKeyDown( VK_RETURN ) || CGame::Inst().IsKeyDown( ' ' ) )
					break;
			}

			int8 nNewRowStates[3];
			for( int i = 0; i < 3; i++ )
				nNewRowStates[i] = nRowState[i];
			auto& nNewState = nNewRowStates[nCursorY];
			auto n1 = nCursorX ? nNewState : ~nNewState;
			if( !!( n1 & 2 ) )
				nNewState ^= 2;
			else if( !!( n1 & 1 ) )
				nNewState ^= 1;
			else
				continue;

			for( int i = 0; i < 3; i++ )
				m_pCursorImg[i]->bVisible = false;
			bool bEnemyMoved = false;
			auto nNewEnemyState = nEnemyState;
			if( nEnemyState == -1 )
				nNewEnemyState = 1 - nCursorX;
			else if( nNewState == 0 && nNewEnemyState == 1 || nNewState == 3 && nNewEnemyState == 0 )
			{
				nNewEnemyState = 1 - nNewEnemyState;
				bEnemyMoved = true;
				for( int i = 0; i < 3; i++ )
				{
					if( nNewEnemyState == 1 && nNewRowStates[i] == 0 )
						nNewRowStates[i] = 1;
					else if( nNewEnemyState == 0 && nNewRowStates[i] == 3 )
						nNewRowStates[i] = 2;
				}
			}

			for( int i = m_nMoveFrames - 1; i >= 0; i-- )
			{
				for( int y = 0; y < 3; y++ )
				{
					auto nOldState = nRowState[y];
					auto nNewState = nNewRowStates[y];
					for( int k = 0; k < 2; k++ )
					{
						auto pImg = m_pImg[k + y * 2]; 
						auto n0 = nOldState & ( 1 << k );
						auto n1 = nNewState & ( 1 << k );
						if( n0 != n1 )
						{
							float x = m_fHalfWidth * i / m_nMoveFrames;
							x = floor( x * 0.5f + 0.5f ) * 2;
							x = n1 ? m_fHalfWidth * 0.5f - x : -m_fHalfWidth * 0.5f + x;
							pImg->SetPosition( CVector2( x, pImg->y ) );
						}
					}
				}
				if( nNewEnemyState != nEnemyState )
				{
					float x = m_pEnemyImg->x;
					float x1 = m_fHalfWidth * i / m_nMoveFrames;
					x1 = floor( x1 * 0.5f + 0.5f ) * 2;
					x1 = nNewEnemyState ? m_fHalfWidth * 0.5f - x1 : -m_fHalfWidth * 0.5f + x1;
					x = nNewEnemyState ? Max( x, x1 ) : Min( x, x1 );
					m_pEnemyImg->SetPosition( CVector2( x, m_pEnemyImg->y ) );
				}
				Yield();
			}
			bool bFinished = true;
			nEnemyState = nNewEnemyState;
			for( int i = 0; i < 3; i++ )
			{
				nRowState[i] = nNewRowStates[i];
				if( nRowState[i] != ( nRowType[i] ? 2 : 1 ) )
					bFinished = false;
			}
			if( bFinished )
				break;

			if( !bEnemyMoved )
			{
				bool b = false;
				for( int y = 0; y < 3; y++ )
				{
					auto& nNewState = nNewRowStates[y];
					if( nEnemyState == 0 && nNewState == 0 || nEnemyState == 1 && nNewState == 3 )
					{
						auto nState1 = nNewState ^ 1;
						if( nState1 == ( nRowType[y] ? 1 : 2 ) )
						{
							nNewState = nState1;
							b = true;
						}
					}
				}
				if( b )
				{
					for( int i = m_nMoveFrames - 1; i >= 0; i-- )
					{
						for( int y = 0; y < 3; y++ )
						{
							auto nOldState = nRowState[y];
							auto nNewState = nNewRowStates[y];
							for( int k = 0; k < 2; k++ )
							{
								auto pImg = m_pImg[k + y * 2];
								auto n0 = nOldState & ( 1 << k );
								auto n1 = nNewState & ( 1 << k );
								if( n0 != n1 )
								{
									float x = m_fHalfWidth * i / m_nMoveFrames;
									x = floor( x * 0.5f + 0.5f ) * 2;
									x = n1 ? m_fHalfWidth * 0.5f - x : -m_fHalfWidth * 0.5f + x;
									pImg->SetPosition( CVector2( x, pImg->y ) );
								}
							}
						}
						Yield();
					}
					for( int i = 0; i < 3; i++ )
						nRowState[i] = nNewRowStates[i];
				}
			}
		}
		//end
		float x0 = m_pEnemyImg->x;
		float nFrames1 = m_nMoveFrames / 4;
		for( int i = nFrames1 - 1; i >= 0; i-- )
		{
			float x = x0 * i / nFrames1;
			x = floor( x * 0.5f + 0.5f ) * 2;
			m_pEnemyImg->SetPosition( CVector2( x, m_pEnemyImg->y ) );
			Yield();
		}
		for( int i = 0; i < 6; i++ )
			m_pImg[i]->bVisible = false;
		for( int i = 0; i < 3; i++ )
		{
			for( int j = 0; j < 40; j++ )
				Yield();
			auto texRect = static_cast<CImage2D*>( m_pEnemyImg.GetPtr() )->GetElem().texRect;
			texRect.x += texRect.width;
			static_cast<CImage2D*>( m_pEnemyImg.GetPtr() )->SetTexRect( texRect );
		}
		for( int j = 0; j < 60; j++ )
			Yield();
		m_pPawn->PlayState( "opening" );
	}
	CReference<CRenderObject2D> m_pImg[6];
	CReference<CRenderObject2D> m_pEnemyImg;
	CReference<CRenderObject2D> m_pCursorImg[3];
	float m_fHalfWidth;
	float m_y0, m_h;
	int32 m_nMoveFrames;
};

class CPasswordUI : public CInteractionUICoroutine
{
	friend void RegisterGameClasses_InteractionUI();
public:
	CPasswordUI( const SClassCreateContext& context ) : CInteractionUICoroutine( context ) { SET_BASEOBJECT_ID( CPasswordUI ); }
	void SetPassword( const char* sz ) { m_strPassword = sz; }
	void AddMultiPassword( const char* sz )
	{
		m_vecMultiPassword.resize( m_vecMultiPassword.size() + 1 );
		auto& item = m_vecMultiPassword.back();
		item.strPassword = sz;
		item.pLuaState = CLuaMgr::GetCurLuaState()->CreateCoroutineAuto();
	}
protected:
	virtual void Func() override
	{
		if( m_strInitScript.length() )
		{
			auto pLuaState = CLuaMgr::GetCurLuaState();
			pLuaState->Load( m_strInitScript );
			pLuaState->PushLua( this );
			pLuaState->Call( 1, 0 );
		}

		m_pOK->bVisible = false;
		m_pError->bVisible = false;
		Yield();

		while( 1 )
		{
			string str = "";
			int8 nTick = 0;
			while( 1 )
			{
				if( CGame::Inst().IsKeyDown( VK_ESCAPE ) )
				{
					m_pPawn->Signal( 3 );
					goto end;
				}
				for( int8 i = 'A'; i <= 'Z' && str.length() < 4; i++ )
				{
					if( CGame::Inst().IsKeyDown( i ) )
					{
						m_pBox[str.length()]->bVisible = false;
						str = str + i;
						m_pText->Set( str.c_str() );
					}
				}
				if( str.length() >= 4 || m_strPassword.length() > 0 && str == m_strPassword )
					break;
				auto pCur = m_pBox[str.length()];
				pCur->bVisible = nTick < 4;
				nTick++;
				if( nTick >= 8 )
					nTick = 0;
				Yield();
			}

			if( str == m_strPassword )
			{
				if( m_strPassword.length() < 4 )
					break;
				m_pOK->bVisible = true;
				for( int i = 0; i < 60; i++ )
					Yield();
				m_pOK->bVisible = false;
				if( m_pPawn )
					m_pPawn->Signal( 1 );
				break;
			}
			else
			{
				for( auto& item : m_vecMultiPassword )
				{
					if( item.strPassword == str )
					{
						m_pOK->bVisible = true;
						for( int i = 0; i < 60; i++ )
							Yield();
						m_pOK->bVisible = false;
						if( m_pPawn )
							m_pPawn->Signal( 1 );
						item.pLuaState->PushLua( this );
						bool b = item.pLuaState->Resume( 1, 0 );
						while( b )
						{
							Yield();
							b = item.pLuaState->Resume( 0, 0 );
						}
						goto end;
					}
				}
				m_pError->bVisible = true;
				for( int i = 0; i < 60; i++ )
					Yield();
				m_pError->bVisible = false;
				if( m_pPawn )
					m_pPawn->Signal( 2 );
			}
			for( int i = 0; i < 4; i++ )
				m_pBox[i]->bVisible = true;
			m_pText->Set( "" );
		}
	end:
		m_vecMultiPassword.resize( 0 );
	}

	CReference<CSimpleText> m_pText;
	CReference<CRenderObject2D> m_pBox[4];
	CReference<CRenderObject2D> m_pOK;
	CReference<CRenderObject2D> m_pError;
	CString m_strInitScript;

	string m_strPassword;
	struct SMultiPasswordItem
	{
		string strPassword;
		CReference<CLuaState> pLuaState;
	};
	vector<SMultiPasswordItem> m_vecMultiPassword;
};

class CDialogueUI : public CInteractionUIScript
{
	friend void RegisterGameClasses_InteractionUI();
public:
	CDialogueUI( const SClassCreateContext& context ) : CInteractionUIScript( context ) { SET_BASEOBJECT_ID( CDialogueUI ); }

	virtual void Init( CPawn* pPawn ) override
	{
		m_p->SetRenderObject( NULL );
		CInteractionUIScript::Init( pPawn );
	}

	void Refresh()
	{
		int32 n = CLuaState::GetCurLuaState()->GetTop() - 1;
		m_str = "";
		string str1;
		for( int i = 0; i < n; i++ )
		{
			auto sz = CLuaState::GetCurLuaState()->FetchLuaString( i + 2 );
			if( i > 0 )
			{
				str1 += "\n\n";
				str1 += sz;
			}
			else
				m_str = sz;
		}
		m_pText1->Set( m_str.c_str() );
		m_pText2->Set( str1.c_str() );
		m_pText2->SetPosition( CVector2( m_pText2->x, m_pText1->y - m_pText1->GetLineCount() * m_pText1->GetInitTextBound().height ) );
		while( m_p->Get_ChildEntity() )
			m_p->Get_ChildEntity()->SetParentEntity( NULL );
		auto pPrefabNode = m_p->GetInstanceOwnerNode();
		m_p2->bVisible = false;
		for( int i = 1; i < n; i++ )
		{
			auto p = SafeCast<CEntity>( pPrefabNode->CreateInstance() );
			p->y = m_pText2->y - ( i * 2 * m_pText2->GetInitTextBound().height );
			p->SetParentEntity( m_p );
		}
	}
	void SelectOption( int32 i )
	{
		m_p2->bVisible = true;
		m_p2->SetPosition( CVector2( m_p2->x, m_pText2->y - ( i * 2 * m_pText2->GetInitTextBound().height ) ) );
	}

	const char* PickWord( float x, float y )
	{
		m_p1->bVisible = false;
		auto result = m_pText1->PickWord( CVector2( x, y ) - m_pText1->GetPosition() );
		if( result.x < 0 )
			return "";
		m_tempStr = m_str.substr( result.x, result.y - result.x );
		m_p1->bVisible = true;
		auto wordBound = m_pText1->GetWordBound( result.x, result.y );
		static_cast<CImage2D*>( m_p1.GetPtr() )->SetRect( wordBound.Offset( m_pText1->GetPosition() ) );
		return m_tempStr.c_str();
	}
private:
	CReference<CSimpleText> m_pText1;
	CReference<CSimpleText> m_pText2;
	CReference<CEntity> m_p;
	CReference<CRenderObject2D> m_p1;
	CReference<CEntity> m_p2;

	string m_str;
	string m_tempStr;
};

void RegisterGameClasses_InteractionUI()
{
	REGISTER_CLASS_BEGIN( CInteractionUI )
		REGISTER_BASE_CLASS( CEntity )
		DEFINE_LUA_REF_OBJECT()
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CInteractionUIScript )
		REGISTER_BASE_CLASS( CInteractionUI )
		REGISTER_MEMBER_BEGIN( m_strScript )
			MEMBER_ARG( text, 1 )
		REGISTER_MEMBER_END()
		DEFINE_LUA_REF_OBJECT()
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CCupboardInteractionUI )
		REGISTER_BASE_CLASS( CInteractionUI )
		REGISTER_MEMBER( m_fHalfWidth )
		REGISTER_MEMBER( m_y0 )
		REGISTER_MEMBER( m_h )
		REGISTER_MEMBER( m_nMoveFrames )
		REGISTER_MEMBER_TAGGED_PTR( m_pImg[0], 0 )
		REGISTER_MEMBER_TAGGED_PTR( m_pImg[1], 1 )
		REGISTER_MEMBER_TAGGED_PTR( m_pImg[2], 2 )
		REGISTER_MEMBER_TAGGED_PTR( m_pImg[3], 3 )
		REGISTER_MEMBER_TAGGED_PTR( m_pImg[4], 4 )
		REGISTER_MEMBER_TAGGED_PTR( m_pImg[5], 5 )
		REGISTER_MEMBER_TAGGED_PTR( m_pEnemyImg, enemy )
		REGISTER_MEMBER_TAGGED_PTR( m_pCursorImg[0], c0 )
		REGISTER_MEMBER_TAGGED_PTR( m_pCursorImg[1], c1 )
		REGISTER_MEMBER_TAGGED_PTR( m_pCursorImg[2], c2 )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CPasswordUI )
		REGISTER_BASE_CLASS( CInteractionUI )
		REGISTER_MEMBER_TAGGED_PTR( m_pText, text )
		REGISTER_MEMBER_TAGGED_PTR( m_pBox[0], 0/a )
		REGISTER_MEMBER_TAGGED_PTR( m_pBox[1], 1/a )
		REGISTER_MEMBER_TAGGED_PTR( m_pBox[2], 2/a )
		REGISTER_MEMBER_TAGGED_PTR( m_pBox[3], 3/a )
		REGISTER_MEMBER_TAGGED_PTR( m_pOK, ok )
		REGISTER_MEMBER_TAGGED_PTR( m_pError, error )
		REGISTER_MEMBER_BEGIN( m_strInitScript )
			MEMBER_ARG( text, 1 )
		REGISTER_MEMBER_END()
		DEFINE_LUA_REF_OBJECT()
		REGISTER_LUA_CFUNCTION( SetPassword )
		REGISTER_LUA_CFUNCTION( AddMultiPassword )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CDialogueUI )
		REGISTER_BASE_CLASS( CInteractionUIScript )
		REGISTER_MEMBER_TAGGED_PTR( m_pText1, text1 )
		REGISTER_MEMBER_TAGGED_PTR( m_pText2, text2 )
		REGISTER_MEMBER_TAGGED_PTR( m_p, p )
		REGISTER_MEMBER_TAGGED_PTR( m_p1, p1 )
		REGISTER_MEMBER_TAGGED_PTR( m_p2, p2 )
		DEFINE_LUA_REF_OBJECT()
		REGISTER_LUA_CFUNCTION( Refresh )
		REGISTER_LUA_CFUNCTION( SelectOption )
		REGISTER_LUA_CFUNCTION( PickWord )
	REGISTER_CLASS_END()
}