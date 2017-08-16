#pragma once
#include "UICommon/UITreeView.h"
#include "UICommon/UICheckBox.h"
#include "UICommon/UITextBox.h"
#include "FileView.h"
#include "Common/ClassMetaData.h"
#include <algorithm>

class CTreeFolder : public CUICheckBox
{
public:
	static CUITreeView::CTreeViewContent* Create( CUITreeView* pTreeView, CUITreeView::CTreeViewContent* pParent, const char* szName );
protected:
	virtual void OnInited() override;
	void OnSwitch();

	CUITreeView* m_pTreeView;
	CUITreeView::CTreeViewContent* m_pContent;
	TClassTrigger<CTreeFolder> m_onSwitch;
};

class CBoolEdit : public CUICheckBox
{
public:
	static CBoolEdit* Create( const char* szName );
};

class CCommonEdit : public CUITextBox
{
public:
	static CCommonEdit* Create( const char* szName );

	CUILabel* GetLabel() { return GetChildByName<CUILabel>( "label" ); }
};

class CVectorEdit : public CUILabel
{
public:
	static CVectorEdit* Create( const char* szName, uint32 nCount );

	float GetFloat();
	CVector2 GetFloat2();
	CVector3 GetFloat3();
	CVector4 GetFloat4();
	void SetFloat2( const CVector2& value );
	void SetFloat3( const CVector3& value );
	void SetFloat4( const CVector4& value );

	uint8 GetFloats( float* pValues );
	uint8 SetFloats( const float* pValues );
protected:
	virtual void OnInited() override;
	void OnEdit();
private:
	uint32 m_nCount;
	CReference<CUITextBox> m_textBox[4];
	TClassTrigger<CVectorEdit> m_onEdit[4];
};

class CFileNameEdit : public CUILabel
{
public:
	static CFileNameEdit* Create( const char* szName, const char* szExt );
protected:
	virtual void OnInited() override;
	void OnEdit();
	void OnEditOK( const char* szText );
private:
	string m_strExt;
	TClassTrigger<CFileNameEdit> m_onEdit;
	TClassTrigger1<CFileNameEdit, const char*> m_onEditOK;
};

class CDropDownBox : public CUILabel
{
	friend class CDropDownScrollView;
public:
	struct SItem
	{
		string name;
		void* pData;
	};
	
	static CDropDownBox* Create( SItem* pItems, uint32 nItems );
	static CDropDownBox* Create( const char* szName, SItem* pItems, uint32 nItems );
	uint32 GetItemCount() { return m_items.size(); }
	SItem* GetItem( uint32 nIndex );
	SItem* GetSelectedItem();
	void SetSelectedItem( uint32 nIndex, bool bTrigger = true );
	void SetSelectedItem( const char* szName, bool bTrigger = true );

	static CDropDownBox* CreateShaderSelectBox( const char* szName, uint8 nType );

	template <class T>
	static CDropDownBox* CreateClassSelectBox( const char* szName )
	{
		static vector<SItem> items;
		static bool bInited = false;
		if( !bInited )
		{
			bInited = true;
			SItem selectItem;
			selectItem.name = "(none)";
			selectItem.pData = NULL;
			items.push_back( selectItem );

			SClassMetaData* pClassData = CClassMetaDataMgr::Inst().GetClassData<T>();
			if( pClassData )
			{
				function<void( SClassMetaData* pData )> func = [] ( SClassMetaData* pData ) {
					if( pData->AllocFunc )
					{
						SItem selectItem;
						selectItem.name = pData->strDisplayName;
						selectItem.pData = pData;
						items.push_back( selectItem );
					}
				};
				pClassData->FindAllDerivedClasses( func );

				struct SLess
				{
					bool operator () ( const SItem & l, const SItem & r )
					{
						return l.name < r.name;
					}
				};
				std::sort( items.begin(), items.end(), SLess() );
			}
		}

		return Create( szName, &items[0], items.size() );
	}
protected:
	virtual void OnInited() override;
	void OnClick();
private:
	TClassTrigger<CDropDownBox> m_onClick;
	CUIButton* m_pBtn;

	uint32 m_nSelectedItem;
	vector<SItem> m_items;
	map<string, uint32> m_itemIndex;
};

class CFileSelectDialog : public CFileView
{
public:
	void Show( const char* szExt, CTrigger* pOnSelected );

	static CFileSelectDialog* Inst();
protected:
	virtual void OnInited() override;
	void OnOk();
	void OnClear();
	void OnCancel();
private:
	CTrigger* m_pOnSelected;

	TClassTrigger<CFileSelectDialog> m_onOk;
	TClassTrigger<CFileSelectDialog> m_onClear;
	TClassTrigger<CFileSelectDialog> m_onCancel;
};