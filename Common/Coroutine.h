#pragma once
#include <vector>
using namespace std;

class ICoroutine
{
public:
	ICoroutine( uint32 nStackSize );
	virtual ~ICoroutine();

	typedef uint32 (*CoroutineFunc)( void* );
	enum EState
	{
		eState_Stopped,
		eState_Running,
		eState_Pending
	};
	virtual void Create( CoroutineFunc pFunc, void* pContext ) = 0;
	uint32 Resume();
	EState GetState() { return m_eState; }
	static ICoroutine* CreateCoroutine( uint32 nStackSize );
	virtual void Yield( uint32 nReturnValue ) = 0;
protected:
	virtual uint32 _Resume() = 0;

	uint32 m_nStackSize;
	EState m_eState;
};

template< int StackSize >
class TCoroutinePool
{
public:
	ICoroutine* Alloc()
	{
		if( m_objs.size() )
		{
			ICoroutine* pCoroutine = m_objs.back();
			m_objs.pop_back();
			return pCoroutine;
		}
		return ICoroutine::CreateCoroutine( StackSize );
	}
	void Free( ICoroutine* pCoroutine )
	{
		m_objs.push_back( pCoroutine );
	}

	DECLARE_GLOBAL_INST_REFERENCE( TCoroutinePool )
private:
	vector<ICoroutine*> m_objs;
};