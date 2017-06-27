#pragma once
#include <string>
#include <vector>
#include <map>
#include <functional>
#include "BufFile.h"
#include "Math3D.h"
#include "Reference.h"
#include "BitArray.h"
#include "ResourceManager.h"
using namespace std;

struct SClassCreateContext
{
};

struct SEnumMetaData
{
	string strName;
	string strDisplayName;
	map<string, uint32> mapNames2Values;
	map<uint32, string> mapValues2Names;
	void AddItem( const char* szName, uint32 nValue );

	void PackData( uint8* pObj, CBufFile& buf, bool bWithMetaData );
	void UnpackData( uint8* pObj, IBufReader& buf, bool bWithMetaData );
};

struct SClassMetaData
{
	SClassMetaData() {}

	uint32 nID;
	string strClassName;
	string strDisplayName;
	uint32 nObjSize;

	struct SMemberData
	{
		string strName;
		enum
		{
			eType_bool,
			eType_uint8,
			eType_uint16,
			eType_uint32,
			eType_uint64,
			eType_int8,
			eType_int16,
			eType_int32,
			eType_int64,
			eType_float,
			eType_float2,
			eType_float3,
			eType_float4,

			eTypeClass,
			eTypeClassPtr,
			eTypeEnum,
			eTypeTaggedPtr,
		};
		uint32 nType;
		uint32 nFlag;
		string strTypeName;
		SClassMetaData* pTypeData;
		SEnumMetaData* pEnumData;
		uint32 nOffset;
		
		uint32 GetDataSize();
		void PackData( uint8* pObj, CBufFile& buf, bool bWithMetaData );
		void UnpackData( uint8* pObj, IBufReader& buf, bool bWithMetaData );
	};
	vector<SMemberData> vecMemberData;
	map<string, uint32> mapMemberDataIndex;
	SMemberData& AddMemberData( const char* szName, uint32 nType, uint32 nOffset );
	SMemberData& AddMemberData( const char* szName, const char* szTypeName, uint32 nOffset );
	SMemberData& AddMemberDataPtr( const char* szName, const char* szTypeName, uint32 nOffset );
	SMemberData& AddMemberDataTaggedPtr( const char* szName, const char* szTypeName, const char* szTag, uint32 nOffset );

	template <typename T>
	struct AddMemberData_Impl{ static void call( SClassMetaData* pThis, const char* szName, uint32 nOffset ) { pThis->AddMemberData( szName, typeid( T ).name(), nOffset ); } };

	template <typename T>
	struct AddMemberData_Impl<T*>{ static void call( SClassMetaData* pThis, const char* szName, uint32 nOffset ) { pThis->AddMemberDataPtr( szName, typeid( T ).name(), nOffset ); } };
	template <typename T>
	struct AddMemberData_Impl<CReference<T> >{ static void call( SClassMetaData* pThis, const char* szName, uint32 nOffset ) { pThis->AddMemberDataPtr( szName, typeid( T ).name(), nOffset ); } };
	
	template <>
	struct AddMemberData_Impl<bool>{ static void call( SClassMetaData* pThis, const char* szName, uint32 nOffset ) { pThis->AddMemberData( szName, SMemberData::eType_bool, nOffset ); } };
	template <>
	struct AddMemberData_Impl<uint8>{ static void call( SClassMetaData* pThis, const char* szName, uint32 nOffset ) { pThis->AddMemberData( szName, SMemberData::eType_uint8, nOffset ); } };
	template <>
	struct AddMemberData_Impl<uint16>{ static void call( SClassMetaData* pThis, const char* szName, uint32 nOffset ) { pThis->AddMemberData( szName, SMemberData::eType_uint16, nOffset ); } };
	template <>
	struct AddMemberData_Impl<uint32>{ static void call( SClassMetaData* pThis, const char* szName, uint32 nOffset ) { pThis->AddMemberData( szName, SMemberData::eType_uint32, nOffset ); } };
	template <>
	struct AddMemberData_Impl<uint64>{ static void call( SClassMetaData* pThis, const char* szName, uint32 nOffset ) { pThis->AddMemberData( szName, SMemberData::eType_uint64, nOffset ); } };
	template <>
	struct AddMemberData_Impl<int8>{ static void call( SClassMetaData* pThis, const char* szName, uint32 nOffset ) { pThis->AddMemberData( szName, SMemberData::eType_int8, nOffset ); } };
	template <>
	struct AddMemberData_Impl<int16>{ static void call( SClassMetaData* pThis, const char* szName, uint32 nOffset ) { pThis->AddMemberData( szName, SMemberData::eType_int16, nOffset ); } };
	template <>
	struct AddMemberData_Impl<int32>{ static void call( SClassMetaData* pThis, const char* szName, uint32 nOffset ) { pThis->AddMemberData( szName, SMemberData::eType_int32, nOffset ); } };
	template <>
	struct AddMemberData_Impl<int64>{ static void call( SClassMetaData* pThis, const char* szName, uint32 nOffset ) { pThis->AddMemberData( szName, SMemberData::eType_int64, nOffset ); } };
	template <>
	struct AddMemberData_Impl<float>{ static void call( SClassMetaData* pThis, const char* szName, uint32 nOffset ) { pThis->AddMemberData( szName, SMemberData::eType_float, nOffset ); } };
	template <>
	struct AddMemberData_Impl<CVector2>{ static void call( SClassMetaData* pThis, const char* szName, uint32 nOffset ) { pThis->AddMemberData( szName, SMemberData::eType_float2, nOffset ); } };
	template <>
	struct AddMemberData_Impl<CVector3>{ static void call( SClassMetaData* pThis, const char* szName, uint32 nOffset ) { pThis->AddMemberData( szName, SMemberData::eType_float3, nOffset ); } };
	template <>
	struct AddMemberData_Impl<CVector4>{ static void call( SClassMetaData* pThis, const char* szName, uint32 nOffset ) { pThis->AddMemberData( szName, SMemberData::eType_float4, nOffset ); } };
	template <>
	struct AddMemberData_Impl<CRectangle>{ static void call( SClassMetaData* pThis, const char* szName, uint32 nOffset ) { pThis->AddMemberData( szName, SMemberData::eType_float4, nOffset ); } };

	template <typename T, int a>
	struct AddMemberData_Impl<T[a]>{ static void call( SClassMetaData* pThis, const char* szName, uint32 nOffset )
	{
		for( int i = 0; i < a; i++ )
		{
			stringstream ss;
			ss << szName << "[" << i << "]";
			pThis->AddMemberData<T>( ss.str().c_str(), nOffset + sizeof( T ) * i );
		}
	} };
	template <typename T>
	struct AddMemberData_Impl<TResourceRef<T> > { static void call( SClassMetaData* pThis, const char* szName, uint32 nOffset )
	{
		pThis->AddMemberData( szName, typeid( CString ).name(), nOffset ).nFlag = 1 | ( T::eResType << 16 );
	} };

	template<typename T>
	void AddMemberData( const char* szName, uint32 nOffset ) { AddMemberData_Impl<T>::call( this, szName, nOffset ); }

	template <typename T>
	struct AddMemberDataTaggedPtr_Impl { static void call( SClassMetaData* pThis, const char* szName, const char* szTag, uint32 nOffset ) {} };
	template <typename T>
	struct AddMemberDataTaggedPtr_Impl<CReference<T> > { static void call( SClassMetaData* pThis, const char* szName, const char* szTag, uint32 nOffset ) { pThis->AddMemberDataTaggedPtr( szName, typeid( T ).name(), szTag, nOffset ); } };
	template <typename T>
	struct AddMemberDataTaggedPtr_Impl<CReference<T>& > { static void call( SClassMetaData* pThis, const char* szName, const char* szTag, uint32 nOffset ) { pThis->AddMemberDataTaggedPtr( szName, typeid( T ).name(), szTag, nOffset ); } };

	template<typename T>
	void AddMemberDataTaggedPtr( const char* szName, const char* szTag, uint32 nOffset ) { AddMemberDataTaggedPtr_Impl<T>::call( this, szName, szTag, nOffset ); }

	struct SBaseClassData
	{
		string strBaseClassName;
		SClassMetaData* pBaseClass;
		uint32 nOffset;
		
		void PackData( uint8* pObj, CBufFile& buf, bool bWithMetaData );
		void UnpackData( uint8* pObj, IBufReader& buf, bool bWithMetaData );
	};
	vector<SBaseClassData> vecBaseClassData;
	map<string, uint32> mapBaseClassDataIndex;
	void AddBaseClassData( const char* szTypeName, uint32 nOffset );
	template<typename T>
	void AddBaseClassdata( uint32 nOffset ) { AddBaseClassData( typeid(T).name(), nOffset ); }

	int32 GetBaseClassOfs( SClassMetaData* pBaseClass );
	bool Is( SClassMetaData* pBaseClass ) { return pBaseClass == this || GetBaseClassOfs( pBaseClass ) >= 0; }
	map<SClassMetaData*, int32> mapBaseClassOfs;
	vector<SClassMetaData*> vecDerivedClasses;
	
	function<uint8*()> AllocFunc;
	function<void(void*)> CreateFunc;
	function<void(void*)> DestroyFunc;
	function<void( uint8*, CBufFile&, bool )> PackFunc;
	function<void( uint8*, IBufReader&, bool )> UnpackFunc;

	void PackData( uint8* pObj, CBufFile& buf, bool bWithMetaData );
	void UnpackData( uint8* pObj, IBufReader& buf, bool bWithMetaData );
	uint8* NewObjFromData( IBufReader& buf, bool bWithMetaData )
	{
		if( !this )
		{
			UnpackData( NULL, buf, bWithMetaData );
			return NULL;
		}
		uint8* pObj = AllocFunc();
		UnpackData( pObj, buf, bWithMetaData );
		CreateFunc( pObj );
		return pObj;
	}

	void FindAllDerivedClasses( function<void( SClassMetaData* pData )>& func );
	void FindAllTaggedPtr( function<void( SMemberData* pData, uint32 nOfs )>& func, SClassMetaData* pBaseClass = NULL, uint32 nOfs = 0 );
	void FindAllResPtr( function<void( SMemberData* pData, uint32 nOfs )>& func, SClassMetaData* pBaseClass = NULL, uint32 nOfs = 0 );
};

class CBaseObject
{
public:
	uint32 GetTypeID() const { return m_nTypeID; }
protected:
	uint32 m_nTypeID;
};

class CClassMetaDataMgr
{
public:
	CClassMetaDataMgr() : m_bInited( false ) {}
	SClassMetaData* RegisterClass( const char* strClassName );
	template<class T>
	SClassMetaData* RegisterClass()
	{
		auto pData = RegisterClass( typeid(T).name() );
		_getClassID<T>() = pData->nID;
		return pData;
	}
	SEnumMetaData* RegisterEnum( const char* strEnumName );
	template<typename T>
	SEnumMetaData* RegisterEnum()
	{
		return RegisterEnum( typeid(T).name() );
	}
	
	template <typename T>
	uint32 GetClassID()
	{
		return _getClassID<T>();
	}

	SClassMetaData* GetClassData( uint32 nID );
	SClassMetaData* GetClassData( const char* strClassName );
	template<class T>
	SClassMetaData* GetClassData()
	{
		return GetClassData( typeid(T).name() );
	}
	template<class T>
	SClassMetaData* GetClassData( T* t )
	{
		return GetClassData( typeid(*t).name() );
	}
	uint32 GetClassCount() { return m_vecClasses.size(); }

	template< typename T, typename T1 >
	T* SafeCast( T1* t )
	{
		uint32 nDerived = t->GetTypeID();
		uint32 nBase = CClassMetaDataMgr::Inst().GetClassID<T>();
		if( !m_isDerivedClassArray.GetBit( GetIsDerivedClassIndex( nBase, nDerived ) ) )
			return NULL;
		return (T*)t;
	}

	template< typename T, typename T1 >
	const T* SafeCast( const T1* t )
	{
		uint32 nDerived = t->GetTypeID();
		uint32 nBase = CClassMetaDataMgr::Inst().GetClassID<T>();
		if( !m_isDerivedClassArray.GetBit( GetIsDerivedClassIndex( nBase, nDerived ) ) )
			return NULL;
		return (const T*)t;
	}

	DECLARE_GLOBAL_INST_REFERENCE( CClassMetaDataMgr )
private:
	template <typename T>
	uint32& _getClassID()
	{
		static uint32 g_nID;
		return g_nID;
	}
	uint32 GetIsDerivedClassIndex( uint32 nBaseID, uint32 nDerivedID ) { return nBaseID * GetClassCount() + nDerivedID; }

	void Init();
	map<string, SClassMetaData> m_mapClasses;
	map<string, SEnumMetaData> m_mapEnums;
	vector<SClassMetaData*> m_vecClasses;
	bool m_bInited;

	CBitArray m_isDerivedClassArray;
};

#define SET_BASEOBJECT_ID( Type ) \
	m_nTypeID = CClassMetaDataMgr::Inst().GetClassID<Type>();

template< typename T, typename T1 >
T* SafeCast( T1* t )
{
	return t? CClassMetaDataMgr::Inst().SafeCast<T>( t ) : NULL;
}

template< typename T, typename T1 >
const T* SafeCast( const T1* t )
{
	return t? CClassMetaDataMgr::Inst().SafeCast<T>( t ) : NULL;
}

#define REGISTER_CLASS_BEGIN( Class ) \
{ \
	typedef Class __cur_class; \
	SClassMetaData* pData = CClassMetaDataMgr::Inst().RegisterClass<Class>(); \
	pData->strDisplayName = #Class; \
	pData->AllocFunc = [] () { uint8* pData = (uint8*)malloc( sizeof( Class ) ); memset( pData, 0, sizeof( Class ) ); return pData; }; \
	pData->CreateFunc = [] ( void* pObj ) { new (pObj)Class( SClassCreateContext() ); }; \
	pData->DestroyFunc = [] ( void* pObj ) { delete (Class*)pObj; }; \
	pData->nObjSize = sizeof( Class );


#define REGISTER_CLASS_BEGIN_ABSTRACT( Class ) \
{ \
	typedef Class __cur_class; \
	SClassMetaData* pData = CClassMetaDataMgr::Inst().RegisterClass<Class>(); \
	pData->strDisplayName = #Class; \
	pData->nObjSize = sizeof( Class );

#define REGISTER_MEMBER( Name ) \
	pData->AddMemberData<decltype( ( (__cur_class*)NULL )->Name )>( #Name, MEMBER_OFFSET( __cur_class, Name ) );

#define REGISTER_MEMBER_TAGGED_PTR( Name, Tag ) \
	pData->AddMemberDataTaggedPtr<decltype( ( (__cur_class*)NULL )->Name )>( #Name, #Tag, MEMBER_OFFSET( __cur_class, Name ) );

#define REGISTER_BASE_CLASS( Name ) \
	pData->AddBaseClassdata<Name>( BASECLASS_OFFSET( __cur_class, Name ) );

#define REGISTER_PACK_FUNC( Name ) \
	pData->PackFunc = [] ( uint8* pObj, CBufFile& buf, bool bWithMetaData ) { ( (__cur_class*)pObj )->Name( buf, bWithMetaData ); };

#define REGISTER_UNPACK_FUNC( Name ) \
	pData->UnpackFunc = [] ( uint8* pObj, IBufReader& buf, bool bWithMetaData ) { ( (__cur_class*)pObj )->Name( buf, bWithMetaData ); };

#define REGISTER_CLASS_END() \
}

#define REGISTER_ENUM_BEGIN( Enum ) \
{ \
	typedef Enum __cur_enum; \
	SEnumMetaData* pData = CClassMetaDataMgr::Inst().RegisterEnum<Enum>(); \
	pData->strDisplayName = #Enum;

#define REGISTER_ENUM_ITEM( Name ) \
	pData->AddItem( #Name, __cur_enum::Name );

#define REGISTER_ENUM_END() \
}

class CObjectPrototype
{
public:
	CObjectPrototype( SClassMetaData* pBaseClass ) : m_pBaseClass( pBaseClass ), m_pClassMetaData( NULL ), m_nCastOffset( 0 ), m_pObj( NULL ), m_bObjDataDirty( false ) {}
	~CObjectPrototype() { SetClassName( NULL ); }

	SClassMetaData* GetClassData() { return m_pClassMetaData; }
	uint8* GetObjData() { return m_pObj; }
	uint32 GetCastOffset() { return m_nCastOffset; }
	bool SetClassName( const char* szName );
	void CopyData( CObjectPrototype& copyTo );
	void SetDirty() { m_bObjDataDirty = true; }

	uint8* CreateObject();
	void Load( IBufReader& buf, bool bWithMetaData );
	void Save( CBufFile& buf, bool bWithMetaData );
private:
	SClassMetaData* m_pBaseClass;
	SClassMetaData* m_pClassMetaData;
	uint32 m_nCastOffset;
	uint8* m_pObj;
	CBufFile m_objData;
	bool m_bObjDataDirty;
};

template <class T>
class TObjectPrototype : public CObjectPrototype
{
public:
	TObjectPrototype() : CObjectPrototype( GetBaseClass() ) {}
	T* CreateObject() { return (T*)CObjectPrototype::CreateObject(); }

	static SClassMetaData* GetBaseClass()
	{
		static SClassMetaData* g_pBaseClass = CClassMetaDataMgr::Inst().GetClassData<T>();
		return g_pBaseClass;
	}
};