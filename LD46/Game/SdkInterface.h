#pragma once

class ISdkInterface
{
public:
	virtual ~ISdkInterface() {}

	virtual void Update() = 0;
	virtual void UnlockAchievement( const char* sz ) = 0;
	virtual void OpenExplorer( const char* sz ) = 0;

	static void Init( ISdkInterface* pInterface )
	{
		ISdkInterface*& p = _inst();
		p = pInterface;
	}
	static ISdkInterface* Inst() { return _inst(); }
private:
	static ISdkInterface*& _inst()
	{
		static ISdkInterface* g_pInst = 0;
		return g_pInst;
	}
};