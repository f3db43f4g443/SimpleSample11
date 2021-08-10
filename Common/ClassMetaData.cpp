#include "Common.h"
#include "ClassMetaData.h"

void SEnumMetaData::AddItem( const char* szName, uint32 nValue )
{
	mapNames2Values[szName] = nValue;
	mapValues2Names[nValue] = szName;
}

void SEnumMetaData::PackData( uint8* pObj, CBufFile& buf, bool bWithMetaData )
{
	if( !bWithMetaData )
		buf.Write( pObj, sizeof( uint32 ) );
	else
	{
		auto itr = mapValues2Names.find( *(uint32*)pObj );
		if( itr != mapValues2Names.end() )
			buf.Write( itr->second );
		else
			buf.Write( string() );
	}
}

void SEnumMetaData::UnpackData( uint8* pObj, IBufReader& buf, bool bWithMetaData, void* pContext )
{
	if( !bWithMetaData )
		buf.Read( pObj, sizeof( uint32 ) );
	else
	{
		string strName;
		buf.Read( strName );
		auto itr = mapNames2Values.find( strName );
		if( itr != mapNames2Values.end() )
			*(uint32*)pObj = itr->second;
		else if( mapValues2Names.size() )
			*(uint32*)pObj = mapValues2Names.begin()->first;
		else
			*(uint32*)pObj = 0;
	}
}

void SEnumMetaData::ExtractData( IBufReader & bufIn, CBufFile & bufOut )
{
	string strName;
	bufIn.Read( strName );
	bufOut.Write( strName );
}

bool SEnumMetaData::DiffData( uint8* pObj0, uint8* pObj1, CBufFile & buf )
{
	auto n0 = *(uint32*)pObj0;
	auto n1 = *(uint32*)pObj1;
	if( n0 == n1 )
		return false;
	PackData( pObj1, buf, true );
	return true;
}

void SEnumMetaData::PatchData( uint8* pObj1, IBufReader &buf, void* pContext )
{
	UnpackData( pObj1, buf, true, pContext );
}

SClassMetaData::SMemberData& SClassMetaData::AddMemberData( const char* szName, uint32 nType, uint32 nOffset )
{
	uint32 nIndex = vecMemberData.size();
	vecMemberData.resize( nIndex + 1 );
	SMemberData& data = vecMemberData[nIndex];
	mapMemberDataIndex[szName] = nIndex;
	data.strName = szName;
	data.nType = nType;
	data.nFlag = 0;
	data.pTypeData = NULL;
	data.pEnumData = NULL;
	data.nOffset = nOffset;
	return data;
}

SClassMetaData::SMemberData& SClassMetaData::AddMemberData( const char* szName, const char* szTypeName, uint32 nOffset )
{
	uint32 nIndex = vecMemberData.size();
	vecMemberData.resize( nIndex + 1 );
	SMemberData& data = vecMemberData[nIndex];
	mapMemberDataIndex[szName] = nIndex;
	data.strName = szName;
	data.nType = SMemberData::eTypeClass;
	data.nFlag = 0;
	data.strTypeName = szTypeName;
	data.pTypeData = NULL;
	data.pEnumData = NULL;
	data.nOffset = nOffset;
	return data;
}

SClassMetaData::SMemberData& SClassMetaData::AddMemberDataObjRef( const char* szName, const char* szTypeName, uint32 nOffset )
{
	uint32 nIndex = vecMemberData.size();
	vecMemberData.resize( nIndex + 1 );
	SMemberData& data = vecMemberData[nIndex];
	mapMemberDataIndex[szName] = nIndex;
	data.strName = szName;
	data.nType = SMemberData::eTypeObjRef;
	data.nFlag = 0;
	data.strTypeName = szTypeName;
	data.pTypeData = NULL;
	data.pEnumData = NULL;
	data.nOffset = nOffset;
	return data;
}

SClassMetaData::SMemberData& SClassMetaData::AddMemberDataTaggedPtr( const char* szName, const char* szTypeName, const char* szTag, uint32 nOffset )
{
	uint32 nIndex = vecMemberData.size();
	vecMemberData.resize( nIndex + 1 );
	SMemberData& data = vecMemberData[nIndex];
	mapMemberDataIndex[szTag] = nIndex;
	data.strName = szTag;
	data.nType = SMemberData::eTypeTaggedPtr;
	data.nFlag = 0;
	data.strTypeName = szTypeName;
	data.pTypeData = NULL;
	data.pEnumData = NULL;
	data.nOffset = nOffset;
	return data;
}

SClassMetaData::SMemberData& SClassMetaData::AddMemberDataCustomTaggedPtr( const char* szName, const char* szTypeName, uint32 nOffset )
{
	uint32 nIndex = vecMemberData.size();
	vecMemberData.resize( nIndex + 1 );
	SMemberData& data = vecMemberData[nIndex];
	mapMemberDataIndex[szName] = nIndex;
	data.strName = szName;
	data.nType = SMemberData::eTypeCustomTaggedPtr;
	data.nFlag = 0;
	data.strTypeName = szTypeName;
	data.pTypeData = NULL;
	data.pEnumData = NULL;
	data.nOffset = nOffset;
	return data;
}

void SClassMetaData::AddBaseClassData( const char* szTypeName, uint32 nOffset )
{
	uint32 nIndex = vecBaseClassData.size();
	vecBaseClassData.resize( nIndex + 1 );
	SBaseClassData& data = vecBaseClassData[nIndex];
	mapBaseClassDataIndex[szTypeName] = nIndex;
	data.strBaseClassName = szTypeName;
	data.pBaseClass = NULL;
	data.nOffset = nOffset;
}

int32 SClassMetaData::GetBaseClassOfs( SClassMetaData* pBaseClass )
{
	auto itr = mapBaseClassOfs.find( pBaseClass );
	if( itr != mapBaseClassOfs.end() )
		return itr->second;

	int32 nOfs = -1;
	for( auto& item : vecBaseClassData )
	{
		if( item.pBaseClass == pBaseClass )
		{
			nOfs = item.nOffset;
			break;
		}

		int32 nBaseClassOfs = item.pBaseClass->GetBaseClassOfs( pBaseClass );
		if( nBaseClassOfs >= 0 )
		{
			nOfs = nBaseClassOfs + item.nOffset;
			break;
		}
	}
	mapBaseClassOfs[pBaseClass] = nOfs;
	return nOfs;
}

uint32 SClassMetaData::SMemberData::GetDataSize()
{
	if( nType == eTypeTaggedPtr )
		return 0;
	else if( nType == eTypeObjRef )
		return sizeof( TObjRef<CReferenceObject> );
	else if( nType == eTypeCustomTaggedPtr )
		return sizeof( TCustomTaggedRef<CReferenceObject> );
	else if( nType == eTypeClass )
	{
		if( pTypeData == CClassMetaDataMgr::Inst().GetClassData<CString>() && ( nFlag & 1 ) )
			return sizeof( TResourceRef<CResource> );
		return pTypeData ? pTypeData->nObjSize : 0;
	}
	else if( nType == eTypeEnum )
		return sizeof( int32 );
	else
	{
		static uint32 g_nSize[] = {
			1,	//eType_bool,
			1,	//eType_uint8,
			2,	//eType_uint16,
			4,	//eType_uint32,
			8,	//eType_uint64,
			1,	//eType_int8,
			2,	//eType_int16,
			4,	//eType_int32,
			8,	//eType_int64,
			4,	//eType_float,
			8,	//eType_float2,
			12,	//eType_float3,
			16,	//eType_float4,
		};
		return g_nSize[nType];
	}
}

int32 SClassMetaData::SMemberData::GetArg( const char* szKey )
{
	auto itr = mapArgs.find( szKey );
	if( itr == mapArgs.end() )
		return 0;
	return itr->second;
}

void SClassMetaData::SMemberData::PackData( uint8* pObj, CBufFile& buf, bool bWithMetaData )
{
	pObj += nOffset;
	int32 nCount = 1;
	auto nDataSize = GetDataSize();
	if( !!( nFlag & 2 ) )
	{
		nCount = *(uint32*)pObj;
		buf.Write( nCount );
		if( !nCount )
			return;
		pObj = *(uint8**)( pObj + sizeof(uint32) );
	}
	for( int i = 0; i < nCount; i++, pObj += nDataSize )
	{
		if( nType == eTypeTaggedPtr )
		{
		}
		else if( nType == eTypeObjRef )
		{
			auto pObj1 = (TObjRef<CReferenceObject>*)pObj;
			buf.Write( pObj1->_n );
			if( !bWithMetaData )
				buf.Write( pObj1->_pt );
		}
		else if( nType == eTypeCustomTaggedPtr )
		{
			auto pObj1 = ( TCustomTaggedRef<CReferenceObject>*)pObj;
			CClassMetaDataMgr::Inst().GetClassData<CString>()->PackData( (uint8*)(CString*)pObj1, buf, bWithMetaData );
		}
		else if( nType == eTypeClass )
		{
			if( pTypeData )
				pTypeData->PackData( pObj, buf, bWithMetaData );
		}
		else if( nType == eTypeEnum )
		{
			if( pEnumData )
				pEnumData->PackData( pObj, buf, bWithMetaData );
		}
		else
			buf.Write( pObj, nDataSize );
	}
}

void SClassMetaData::SMemberData::UnpackData( uint8* pObj, IBufReader& buf, bool bWithMetaData, void* pContext )
{
	pObj += nOffset;
	int32 nCount = 1;
	auto nDataSize = GetDataSize();
	if( !!( nFlag & 2 ) )
	{
		buf.Read( nCount );
		*(uint32*)pObj = nCount;
		if( !nCount )
			return;
		auto pData = (uint8*)malloc( nDataSize * nCount );
		memset( pData, 0, nDataSize * nCount );
		*(uint8**)( pObj + sizeof( uint32 ) ) = pData;
		pObj = pData;
	}
	for( int i = 0; i < nCount; i++, pObj += nDataSize )
	{
		if( nType == eTypeTaggedPtr )
		{
		}
		else if( nType == eTypeObjRef )
		{
			auto pObj1 = ( TObjRef<CReferenceObject>* )pObj;
			buf.Read( pObj1->_n );
			if( !bWithMetaData )
				buf.Read( pObj1->_pt );
			else if( pObj1->_n )
				pObj1->_pt = pContext;
		}
		else if( nType == eTypeCustomTaggedPtr )
		{
			auto pObj1 = ( TCustomTaggedRef<CReferenceObject>* )pObj;
			CClassMetaDataMgr::Inst().GetClassData<CString>()->UnpackData( ( uint8* )(CString*)pObj1, buf, bWithMetaData, pContext );
		}
		else if( nType == eTypeClass )
		{
			if( pTypeData )
				pTypeData->UnpackData( pObj, buf, bWithMetaData, pContext );
		}
		else if( nType == eTypeEnum )
		{
			if( pEnumData )
				pEnumData->UnpackData( pObj, buf, bWithMetaData, pContext );
		}
		else
			buf.Read( pObj, nDataSize );
	}
}

bool SClassMetaData::SMemberData::ExtractData( bool bDiffMode, SClassMetaData* pOwner, IBufReader& bufIn, CBufFile& bufOut, function<bool( SClassMetaData*, SMemberData*, int32, IBufReader&, CBufFile& )>& handler )
{
	int32 nCount = 1;
	auto nDataSize = GetDataSize();
	if( !!( nFlag & 2 ) )
	{
		if( bDiffMode )
		{
			bool bNewArray = false;
			bufIn.Read( bNewArray );
			bufOut.Write( bNewArray );
			if( bNewArray )
				return ExtractData( false, pOwner, bufIn, bufOut, handler );
		}
		else
		{
			if( handler( pOwner, this, -1, bufIn, bufOut ) )
				return true;
			bufIn.Read( nCount );
			bufOut.Write( nCount );
			if( !nCount )
				return false;
		}
	}
	bool b = false;
	bool bArrIndex = bDiffMode && !!( nFlag & 2 );
	for( int i = 0; bArrIndex ? true : i < nCount; i++ )
	{
		int32 i1 = i;
		if( bArrIndex )
		{
			bufIn.Read( i1 );
			bufOut.Write( i1 );
			if( i1 < 0 )
				break;
		}
		if( handler( pOwner, this, i1, bufIn, bufOut ) )
		{
			b = true;
			continue;
		}

		if( nType == eTypeTaggedPtr )
		{
		}
		else if( nType == eTypeObjRef )
		{
			bufOut.Write( bufIn.Read<int32>() );
		}
		else if( nType == eTypeCustomTaggedPtr )
		{
			b = CClassMetaDataMgr::Inst().GetClassData<CString>()->ExtractData( bDiffMode, bufIn, bufOut, handler ) || b;
		}
		else if( nType == eTypeClass )
		{
			if( pTypeData )
				b = pTypeData->ExtractData( bDiffMode, bufIn, bufOut, handler ) || b;
		}
		else if( nType == eTypeEnum )
		{
			if( pEnumData )
				pEnumData->ExtractData( bufIn, bufOut );
		}
		else
		{
			bufOut.Write( bufIn.GetCurBuffer(), nDataSize );
			bufIn.Read( NULL, nDataSize );
		}
	}
	return b;
}

bool SClassMetaData::SMemberData::DiffData( uint8* pObj0, uint8* pObj1, CBufFile& buf )
{
	auto p0 = pObj0;
	auto p1 = pObj1;
	pObj0 += nOffset;
	pObj1 += nOffset;

	int32 nCount = 1;
	auto nDataSize = GetDataSize();
	int8 bNewArray = false;
	if( !!( nFlag & 2 ) )
	{
		int32 nCount0 = *(uint32*)pObj0;
		nCount = *(uint32*)pObj1;
		if( nCount0 != nCount )
		{
			bNewArray = true;
		}
		else
		{
			if( !nCount )
				return false;
			pObj0 = *(uint8**)( pObj0 + sizeof( uint32 ) );
			pObj1 = *(uint8**)( pObj1 + sizeof( uint32 ) );
		}
		buf.Write( bNewArray );
	}
	if( !bNewArray )
	{
		CBufFile tempBuf;
		for( int i = 0; i < nCount; i++, pObj0 += nDataSize, pObj1 += nDataSize )
		{
			tempBuf.Clear();
			if( nType == eTypeTaggedPtr )
			{
				continue;
			}
			else if( nType == eTypeObjRef )
			{
				auto p0 = ( TObjRef<CReferenceObject>* )pObj0;
				auto p1 = ( TObjRef<CReferenceObject>* )pObj1;
				if( p0->_pt == p1->_pt && p0->_n == p1->_n )
					continue;
				tempBuf.Write( p1->_n );
			}
			else if( nType == eTypeCustomTaggedPtr )
			{
				auto p0 = ( TCustomTaggedRef<CReferenceObject>* )pObj0;
				auto p1 = ( TCustomTaggedRef<CReferenceObject>* )pObj1;
				if( !CClassMetaDataMgr::Inst().GetClassData<CString>()->DiffData( (uint8*)(CString*)p0, ( uint8* )&(CString&)p1, tempBuf ) )
					continue;
			}
			else if( nType == eTypeClass )
			{
				if( !pTypeData || !pTypeData->DiffData( pObj0, pObj1, tempBuf ) )
					continue;
			}
			else if( nType == eTypeEnum )
			{
				if( !pEnumData || !pEnumData->DiffData( pObj0, pObj1, tempBuf ) )
					continue;
			}
			else
			{
				if( !memcmp( pObj0, pObj1, nDataSize ) )
					continue;
				tempBuf.Write( pObj1, GetDataSize() );
			}
			bNewArray = true;
			if( !!( nFlag & 2 ) )
				buf.Write( i );
			buf.Write( tempBuf.GetBuffer(), tempBuf.GetBufLen() );
		}
		if( bNewArray && !!( nFlag & 2 ) )
			buf.Write( -1 );
	}
	else
		PackData( p1, buf, true );
	return bNewArray;
}

void SClassMetaData::SMemberData::PatchData( uint8* pObj1, IBufReader& buf, void* pContext )
{
	auto nDataSize = GetDataSize();
	int8 bNewArray = false;
	int32 nCount = 1;
	if( !!( nFlag & 2 ) )
	{
		buf.Read( bNewArray );
		if( bNewArray )
		{
			auto pArray = pObj1 + nOffset;
			int32 nCount = *(uint32*)pArray;
			nCount = 0;
			auto& pObj0 = *(uint8**)( pArray + sizeof( uint32 ) );
			if( pObj0 )
			{
				delete pObj0;
				pObj0 = NULL;
			}
			UnpackData( pObj1, buf, true, pContext );
			return;
		}
		pObj1 += nOffset;
		nCount = *(uint32*)pObj1;
		pObj1 = *(uint8**)( pObj1 + sizeof( uint32 ) );
	}

	for( int i = 0;; i++ )
	{
		uint32 i1 = 0;
		if( !!( nFlag & 2 ) )
		{
			buf.Read( i1 );
			if( i1 >= nCount )
				break;
		}

		if( nType == eTypeTaggedPtr )
		{
		}
		else if( nType == eTypeObjRef )
		{
			auto p1 = ( TObjRef<CReferenceObject>* )( pObj1 + nDataSize * i1 );
			buf.Read( p1->_n );
			p1->_pt = pContext;
			/*auto& pObj1 = *(uint8**)pObj;
			bool bObj = buf.Read<uint8>();
			if( pTypeData && bObj )
			pObj1 = pTypeData->NewObjFromData( buf, bWithMetaData );
			else
			pObj1 = 0;*/
		}
		else if( nType == eTypeCustomTaggedPtr )
		{
			auto p1 = ( TCustomTaggedRef<CReferenceObject>* )( pObj1 + nDataSize * i1 );
			CClassMetaDataMgr::Inst().GetClassData<CString>()->PatchData( (uint8*)(CString*)p1, buf, pContext );
		}
		else if( nType == eTypeClass )
		{
			if( pTypeData )
				pTypeData->PatchData( pObj1 + nDataSize * i1, buf, pContext );
		}
		else if( nType == eTypeEnum )
		{
			if( pEnumData )
				pEnumData->PatchData( pObj1 + nDataSize * i1, buf, pContext );
		}
		else
			buf.Read( pObj1 + nDataSize * i1, nDataSize );
		if( !( nFlag & 2 ) )
			break;
	}
	return;
}

void SClassMetaData::SBaseClassData::PackData( uint8* pObj, CBufFile& buf, bool bWithMetaData )
{
	pObj += nOffset;
	pBaseClass->PackData( pObj, buf, bWithMetaData );
}

void SClassMetaData::SBaseClassData::UnpackData( uint8* pObj, IBufReader& buf, bool bWithMetaData, void* pContext )
{
	pObj += nOffset;
	pBaseClass->UnpackData( pObj, buf, bWithMetaData, pContext );
}

bool SClassMetaData::SBaseClassData::ExtractData( bool bDiffMode, IBufReader& bufIn, CBufFile& bufOut, function<bool( SClassMetaData*, SMemberData*, int32, IBufReader&, CBufFile& )>& handler )
{
	return pBaseClass->ExtractData( bDiffMode, bufIn, bufOut, handler );
}

bool SClassMetaData::SBaseClassData::DiffData( uint8 * pObj0, uint8 * pObj1, CBufFile & buf )
{
	pObj0 += nOffset;
	pObj1 += nOffset;
	return pBaseClass->DiffData( pObj0, pObj1, buf );
}

void SClassMetaData::SBaseClassData::PatchData( uint8* pObj1, IBufReader& buf, void* pContext )
{
	pObj1 += nOffset;
	pBaseClass->PatchData( pObj1, buf, pContext );
}

void SClassMetaData::PackData( uint8* pObj, CBufFile& buf, bool bWithMetaData )
{
	if( !bWithMetaData )
	{
		if( !this )
			return;
		if( PackFunc )
			PackFunc( pObj, buf, bWithMetaData );
		for( auto& item : vecMemberData )
			item.PackData( pObj, buf, bWithMetaData );
		for( auto& item : vecBaseClassData )
			item.PackData( pObj, buf, bWithMetaData );
	}
	else
	{
		if( !this )
		{
			buf.Write( 0 );
			buf.Write( 0 );
			buf.Write( 0 );
			return;
		}

		CBufFile extraData;
		if( PackFunc )
			PackFunc( pObj, extraData, bWithMetaData );
		buf.Write( extraData );

		buf.Write( vecMemberData.size() );
		for( auto& item : vecMemberData )
		{
			bool bArray = !!( item.nFlag & 2 );
			if( bArray )
				buf.Write( item.strName + '#' );
			else
				buf.Write( item.strName );
			buf.Write( item.nType );
			if( item.nType >= SMemberData::eTypeClass )
				buf.Write( item.strTypeName );
			CBufFile tempBuf;
			item.PackData( pObj, tempBuf, bWithMetaData );
			buf.Write( tempBuf );
		}

		buf.Write( vecBaseClassData.size() );
		for( auto& item : vecBaseClassData )
		{
			buf.Write( item.strBaseClassName );
			CBufFile tempBuf;
			item.PackData( pObj, tempBuf, bWithMetaData );
			buf.Write( tempBuf );
		}
	}
}

void SClassMetaData::UnpackData( uint8* pObj, IBufReader& buf, bool bWithMetaData, void* pContext )
{
	if( !bWithMetaData )
	{
		if( !this )
			return;
		if( UnpackFunc )
			UnpackFunc( pObj, buf, bWithMetaData, pContext );
		for( auto& item : vecMemberData )
			item.UnpackData( pObj, buf, bWithMetaData, pContext );
		for( auto& item : vecBaseClassData )
			item.UnpackData( pObj, buf, bWithMetaData, pContext );
	}
	else
	{
		CBufReader extraData( buf );
		if( this && UnpackFunc )
			UnpackFunc( pObj, extraData, bWithMetaData, pContext );

		uint32 nMemberData = buf.Read<uint32>();
		for( int i = 0; i < nMemberData; i++ )
		{
			string strName;
			bool bArray = false;
			buf.Read( strName );
			if( strName.length() && strName.back() == '#' )
			{
				strName.pop_back();
				bArray = true;
			}
			uint32 nType = buf.Read<uint32>();
			string strTypeName;
			if( nType >= SMemberData::eTypeClass )
				buf.Read( strTypeName );
			CBufReader tempBuf( buf );

			if( this )
			{
				auto itr = mapMemberDataIndex.find( strName );
				if( itr == mapMemberDataIndex.end() )
					continue;
				auto& item = vecMemberData[itr->second];
				if( item.nType != nType )
					continue;
				if( !!( item.nFlag & 2 ) != bArray )
					continue;
				if( item.nType >= SMemberData::eTypeClass )
				{
					if( strTypeName != item.strTypeName )
						continue;
				}
				item.UnpackData( pObj, tempBuf, bWithMetaData, pContext );
			}
		}

		uint32 nBaseClassData = buf.Read<uint32>();
		for( int i = 0; i < nBaseClassData; i++ )
		{
			string strName;
			buf.Read( strName );
			CBufReader tempBuf( buf );

			if( this )
			{
				auto itr = mapBaseClassDataIndex.find( strName );
				if( itr == mapBaseClassDataIndex.end() )
					continue;
				auto& item = vecBaseClassData[itr->second];
				item.UnpackData( pObj, tempBuf, bWithMetaData, pContext );
			}
		}
	}
}

bool SClassMetaData::ExtractData( bool bDiffMode, IBufReader& bufIn, CBufFile& bufOut, function<bool( SClassMetaData*, SMemberData*, int32, IBufReader&, CBufFile& )>& handler )
{
	bool b = false;
	CBufReader tempExtraDataIn( bufIn );
	CBufReader tmp = tempExtraDataIn;
	CBufFile tempExtraDataOut;
	if( handler( this, NULL, 0, tempExtraDataIn, tempExtraDataOut ) )
	{
		b = true;
		bufOut.Write( tempExtraDataOut );
	}
	else
		bufOut.Write( tmp );

	uint32 nMemberData = bufIn.Read<uint32>();
	int32 nMemberDataPos = bufOut.GetBufLen();
	int32 nOutMemberData = 0;
	bufOut.Write( nOutMemberData );
	for( int i = 0; i < nMemberData; i++ )
	{
		string strName;
		bool bArray = false;
		bufIn.Read( strName );
		if( strName.length() && strName.back() == '#' )
		{
			strName.pop_back();
			bArray = true;
		}
		uint32 nType = bufIn.Read<uint32>();
		string strTypeName;
		if( nType >= SMemberData::eTypeClass )
		{
			bufIn.Read( strTypeName );
		}
		CBufReader tempBufIn( bufIn );
		CBufFile tempBufOut;

		if( this )
		{
			auto itr = mapMemberDataIndex.find( strName );
			if( itr == mapMemberDataIndex.end() )
				continue;
			auto& item = vecMemberData[itr->second];
			if( item.nType != nType )
				continue;
			if( !!( item.nFlag & 2 ) != bArray )
				continue;
			if( item.nType >= SMemberData::eTypeClass )
			{
				if( strTypeName != item.strTypeName )
					continue;
			}
			b = item.ExtractData( bDiffMode, this, tempBufIn, tempBufOut, handler ) || b;
		}
		nOutMemberData++;
		bufOut.Write( strName );
		bufOut.Write( nType );
		if( nType >= SMemberData::eTypeClass )
		{
			bufOut.Write( strTypeName );
		}
		bufOut.Write( tempBufOut );
	}
	*(int32*)( (uint8*)bufOut.GetBuffer() + nMemberDataPos ) = nOutMemberData;

	uint32 nBaseClassData = bufIn.Read<uint32>();
	int32 nBaseClassDataPos = bufOut.GetBufLen();
	int32 nOutBaseClassData = 0;
	bufOut.Write( nOutBaseClassData );
	for( int i = 0; i < nBaseClassData; i++ )
	{
		string strName;
		bufIn.Read( strName );
		CBufReader tempBufIn( bufIn );
		CBufFile tempBufOut;

		if( this )
		{
			auto itr = mapBaseClassDataIndex.find( strName );
			if( itr == mapBaseClassDataIndex.end() )
				continue;
			auto& item = vecBaseClassData[itr->second];
			b = item.ExtractData( bDiffMode, tempBufIn, tempBufOut, handler ) || b;
		}
		nOutBaseClassData++;
		bufOut.Write( strName );
		bufOut.Write( tempBufOut );
	}
	*(int32*)( (uint8*)bufOut.GetBuffer() + nBaseClassDataPos ) = nOutBaseClassData;

	return b;
}

bool SClassMetaData::DiffData( uint8 * pObj0, uint8 * pObj1, CBufFile & buf )
{
	if( !this )
	{
		buf.Write( 0 );
		buf.Write( 0 );
		buf.Write( 0 );
		return false;
	}

	bool bDiff = false;
	CBufFile extraData;
	if( DiffFunc )
		bDiff = DiffFunc( pObj0, pObj1, extraData );
	buf.Write( extraData );

	int32 nItemOfs = buf.GetBufLen();
	int32 nItems = 0;
	buf.Write( 0 );
	for( auto& item : vecMemberData )
	{
		CBufFile tempBuf;
		if( !item.DiffData( pObj0, pObj1, tempBuf ) )
			continue;
		bDiff = true;
		nItems++;
		bool bArray = !!( item.nFlag & 2 );
		if( bArray )
			buf.Write( item.strName + '#' );
		else
			buf.Write( item.strName );
		buf.Write( item.nType );
		if( item.nType >= SMemberData::eTypeClass )
			buf.Write( item.strTypeName );
		buf.Write( tempBuf );
	}
	*(int32*)( nItemOfs + (uint8*)buf.GetBuffer() ) = nItems;

	nItemOfs = buf.GetBufLen();
	nItems = 0;
	buf.Write( 0 );
	for( auto& item : vecBaseClassData )
	{
		CBufFile tempBuf;
		if( !item.DiffData( pObj0, pObj1, tempBuf ) )
			continue;
		bDiff = true;
		nItems++;
		buf.Write( item.strBaseClassName );
		buf.Write( tempBuf );
	}
	*(int32*)( nItemOfs + (uint8*)buf.GetBuffer() ) = nItems;
	return bDiff;
}

void SClassMetaData::PatchData( uint8 * pObj1, IBufReader & buf, void* pContext )
{
	CBufReader extraData( buf );
	if( this && PatchFunc )
		PatchFunc( pObj1, extraData, pContext );

	uint32 nMemberData = buf.Read<uint32>();
	for( int i = 0; i < nMemberData; i++ )
	{
		string strName;
		bool bArray = false;
		buf.Read( strName );
		if( strName.length() && strName.back() == '#' )
		{
			strName.pop_back();
			bArray = true;
		}
		uint32 nType = buf.Read<uint32>();
		string strTypeName;
		if( nType >= SMemberData::eTypeClass )
			buf.Read( strTypeName );
		CBufReader tempBuf( buf );

		if( this )
		{
			auto itr = mapMemberDataIndex.find( strName );
			if( itr == mapMemberDataIndex.end() )
				continue;
			auto& item = vecMemberData[itr->second];
			if( item.nType != nType )
				continue;
			if( !!( item.nFlag & 2 ) != bArray )
				continue;
			if( item.nType >= SMemberData::eTypeClass )
			{
				if( strTypeName != item.strTypeName )
					continue;
			}
			item.PatchData( pObj1, tempBuf, pContext );
		}
	}

	uint32 nBaseClassData = buf.Read<uint32>();
	for( int i = 0; i < nBaseClassData; i++ )
	{
		string strName;
		buf.Read( strName );
		CBufReader tempBuf( buf );

		if( this )
		{
			auto itr = mapBaseClassDataIndex.find( strName );
			if( itr == mapBaseClassDataIndex.end() )
				continue;
			auto& item = vecBaseClassData[itr->second];
			item.PatchData( pObj1, tempBuf, pContext );
		}
	}
}

bool SClassMetaData::ConvertDataUpFrom( SClassMetaData* pFrom, uint8* pObj, CBufFile& buf )
{
	if( this == pFrom )
	{
		PackData( pObj, buf, true );
		return true;
	}

	for( auto& item : vecBaseClassData )
	{
		CBufFile tempBuf;
		if( item.pBaseClass->ConvertDataUpFrom( pFrom, pObj, tempBuf ) )
		{
			buf.Write( 0 );
			buf.Write( 0 );
			buf.Write( 1 );
			buf.Write( item.strBaseClassName );
			buf.Write( tempBuf );
			return true;
		}
	}
	return false;
}

bool SClassMetaData::ConvertDataDownTo( SClassMetaData* pTo, uint8* pObj, CBufFile& buf )
{
	if( this == pTo )
	{
		PackData( pObj, buf, true );
		return true;
	}
	for( auto& item : vecBaseClassData )
	{
		if( item.pBaseClass->ConvertDataDownTo( pTo, pObj + item.nOffset, buf ) )
			return true;
	}
	return false;
}

SClassMetaData* SClassMetaData::FindCommonBaseClass( SClassMetaData* pClassA, SClassMetaData* pClassB )
{
	function<bool( SClassMetaData* pData )> FuncMark = [] ( SClassMetaData* pData ) { pData->nTempFlag = 1; return true; };
	pClassA->FindAllBaseClasses( FuncMark );
	SClassMetaData* pBaseClass = NULL;
	function<bool( SClassMetaData* pData )> FuncFind = [&pBaseClass] ( SClassMetaData* pData ) {
		if( pData->nTempFlag == 1 )
		{
			pBaseClass = pData;
			return false;
		}
		return true;
	};
	pClassB->FindAllBaseClasses( FuncFind );
	function<bool( SClassMetaData* pData )> FuncCleanUp = [] ( SClassMetaData* pData ) { pData->nTempFlag = 0; return true; };
	pClassA->FindAllBaseClasses( FuncCleanUp );
	return pBaseClass;
}

bool SClassMetaData::FindAllBaseClasses( function<bool( SClassMetaData* pData )>& func )
{
	if( !func( this ) )
		return false;
	for( auto& item : vecBaseClassData )
	{
		if( !item.pBaseClass->FindAllBaseClasses( func ) )
			return false;
	}
	return true;
}

void SClassMetaData::FindAllDerivedClasses( function<void( SClassMetaData* pData )>& func )
{
	func( this );
	for( auto& item : vecDerivedClasses )
	{
		item->FindAllDerivedClasses( func );
	}
}

void SClassMetaData::FindAllTaggedPtr( function<void( SMemberData* pData, uint32 nOfs )>& func, SClassMetaData* pBaseClass, uint32 nOfs )
{
	for( auto& item : vecMemberData )
	{
		if( item.nType == SMemberData::eTypeTaggedPtr )
		{
			if( !pBaseClass || item.pTypeData && item.pTypeData->Is( pBaseClass ) )
			{
				func( &item, nOfs + item.nOffset );
			}
		}
	}

	for( auto& item : vecBaseClassData )
	{
		if( item.pBaseClass )
			item.pBaseClass->FindAllTaggedPtr( func, pBaseClass, nOfs + item.nOffset );
	}
}

void SClassMetaData::FindAllObjRef( function<void( SMemberData* pData, uint32 nOfs )>& func, SClassMetaData* pBaseClass, uint32 nOfs )
{
	for( auto& item : vecMemberData )
	{
		if( item.nType == SMemberData::eTypeObjRef && item.pTypeData )
		{
			if( !pBaseClass || item.pTypeData->Is( pBaseClass ) )
			{
				func( &item, nOfs + item.nOffset );
			}
		}
		else if( item.nType == SMemberData::eTypeClass )
		{
			if( !!( item.nFlag & 2 ) )
			{
				func( &item, nOfs + item.nOffset );
				item.pTypeData->FindAllObjRef( func, 0 );
				func( NULL, nOfs + item.nOffset );
			}
			else
				item.pTypeData->FindAllObjRef( func, pBaseClass, nOfs + item.nOffset );
		}
	}

	for( auto& item : vecBaseClassData )
	{
		if( item.pBaseClass )
			item.pBaseClass->FindAllObjRef( func, pBaseClass, nOfs + item.nOffset );
	}
}

void SClassMetaData::FindAllCustomTaggedPtr( function<void( SMemberData* pData, uint32 nOfs )>& func, SClassMetaData * pBaseClass, uint32 nOfs )
{
	for( auto& item : vecMemberData )
	{
		if( item.nType == SMemberData::eTypeCustomTaggedPtr )
		{
			if( !pBaseClass || item.pTypeData && item.pTypeData->Is( pBaseClass ) )
			{
				func( &item, nOfs + item.nOffset );
			}
		}
		else if( item.nType == SMemberData::eTypeClass )
		{
			if( !!( item.nFlag & 2 ) )
			{
				func( &item, nOfs + item.nOffset );
				item.pTypeData->FindAllCustomTaggedPtr( func, 0 );
				func( NULL, nOfs + item.nOffset );
			}
			else
				item.pTypeData->FindAllCustomTaggedPtr( func, pBaseClass, nOfs + item.nOffset );
		}
	}

	for( auto& item : vecBaseClassData )
	{
		if( item.pBaseClass )
			item.pBaseClass->FindAllCustomTaggedPtr( func, pBaseClass, nOfs + item.nOffset );
	}
}

void SClassMetaData::FindAllResPtr( function<void( SMemberData* pData, uint32 nOfs )>& func, uint32 nOfs )
{
	for( auto& item : vecMemberData )
	{
		if( item.nType == SMemberData::eTypeClass )
		{
			if( item.pTypeData == CClassMetaDataMgr::Inst().GetClassData<CString>() && ( item.nFlag & 1 ) )
			{
				func( &item, nOfs + item.nOffset );
			}
			else
			{
				if( !!( item.nFlag & 2 ) )
				{
					func( &item, nOfs + item.nOffset );
					item.pTypeData->FindAllResPtr( func, 0 );
					func( NULL, nOfs + item.nOffset );
				}
				else
					item.pTypeData->FindAllResPtr( func, nOfs + item.nOffset );
			}
		}
	}

	for( auto& item : vecBaseClassData )
	{
		if( item.pBaseClass )
			item.pBaseClass->FindAllResPtr( func, nOfs + item.nOffset );
	}
}

SClassMetaData* CClassMetaDataMgr::RegisterClass( const char* strClassName )
{
	SClassMetaData* pData = &m_mapClasses[strClassName];
	pData->strClassName = strClassName;
	pData->nID = m_vecClasses.size();
	m_vecClasses.push_back( pData );
	return pData;
}

SEnumMetaData* CClassMetaDataMgr::RegisterEnum( const char* strEnumName )
{
	SEnumMetaData* pData = &m_mapEnums[strEnumName];
	pData->strName = strEnumName;
	return pData;
}

SClassMetaData* CClassMetaDataMgr::GetClassData( uint32 nID )
{
	Init();
	if( nID >= m_vecClasses.size() )
		return NULL;
	return m_vecClasses[nID];
}

SClassMetaData* CClassMetaDataMgr::GetClassData( const char* strClassName )
{
	Init();
	auto itr = m_mapClasses.find( strClassName );
	if( itr == m_mapClasses.end() )
		return NULL;
	return &itr->second;
}

#include "StringUtil.h"

void CClassMetaDataMgr::Init()
{
	if( m_bInited )
		return;

	REGISTER_CLASS_BEGIN( CBaseObject )
	REGISTER_CLASS_END()

	REGISTER_CLASS_BEGIN( CString )
		REGISTER_PACK_FUNC( PackData )
		REGISTER_UNPACK_FUNC( UnpackData )
		REGISTER_DIFF_FUNC( DiffData )
		REGISTER_PATCH_FUNC( PatchData )
	REGISTER_CLASS_END()

	uint32 nClasses = m_vecClasses.size();
	m_isDerivedClassArray.SetBitCount( nClasses * nClasses );
	for( auto& item : m_mapClasses )
	{
		auto& classData = item.second;
		for( auto& memberData : classData.vecMemberData )
		{
			if( memberData.nType == SClassMetaData::SMemberData::eTypeClass || memberData.nType == SClassMetaData::SMemberData::eTypeObjRef
				|| memberData.nType == SClassMetaData::SMemberData::eTypeTaggedPtr || memberData.nType == SClassMetaData::SMemberData::eTypeCustomTaggedPtr )
			{
				auto itr = m_mapClasses.find( memberData.strTypeName );
				if( itr != m_mapClasses.end() )
					memberData.pTypeData = &itr->second;
				else if( memberData.nType == SClassMetaData::SMemberData::eTypeClass )
				{
					auto itr1 = m_mapEnums.find( memberData.strTypeName );
					if( itr1 != m_mapEnums.end() )
					{
						memberData.pEnumData = &itr1->second;
						memberData.nType = SClassMetaData::SMemberData::eTypeEnum;
					}
				}
			}
		}
		for( auto& baseClassData : classData.vecBaseClassData )
		{
			auto itr = m_mapClasses.find( baseClassData.strBaseClassName );
			if( itr != m_mapClasses.end() )
			{
				baseClassData.pBaseClass = &itr->second;
				baseClassData.pBaseClass->vecDerivedClasses.push_back( &classData );
			}
		}
	}

	for( auto& item : m_mapClasses )
	{
		uint32 nID = item.second.nID;
		m_isDerivedClassArray.SetBit( GetIsDerivedClassIndex( nID, nID ), true );
		function<void( SClassMetaData* pData )> func = [nID, this] ( SClassMetaData* pData ) {
			m_isDerivedClassArray.SetBit( GetIsDerivedClassIndex( nID, pData->nID ), true );
		};
		item.second.FindAllDerivedClasses( func );
	}
	m_bInited = true;
}

bool CObjectPrototype::SetClassName( const char* szName )
{
	if( !szName || !szName[0] )
	{
		if( m_pClassMetaData )
		{
			m_pClassMetaData->DestroyFunc( m_pObj );
			m_pObj = NULL;
		}
		m_pClassMetaData = 0;
		m_nCastOffset = 0;
		m_bObjDataDirty = true;
		return true;
	}

	SClassMetaData* pData = CClassMetaDataMgr::Inst().GetClassData( szName );
	if( !pData || pData == m_pClassMetaData )
		return false;
	int32 nOfs = pData->GetBaseClassOfs( m_pBaseClass );
	if( nOfs < 0 )
		return false;

	auto pCommonBase = pData && m_pClassMetaData ? SClassMetaData::FindCommonBaseClass( m_pClassMetaData, pData ) : NULL;

	CBufFile buf;
	uint8* pObj = NULL;
	if( !pCommonBase )
		m_pClassMetaData->PackData( m_pObj, buf, true );
	else if( pCommonBase == m_pClassMetaData )
		pData->ConvertDataUpFrom( pCommonBase, m_pObj, buf );
	else if( pCommonBase == pData )
		m_pClassMetaData->ConvertDataDownTo( pCommonBase, m_pObj, buf );
	else
	{
		m_pClassMetaData->ConvertDataDownTo( pCommonBase, m_pObj, buf );
		pObj = pCommonBase->NewObjFromData( buf, true, NULL );
		buf.Clear();
		pData->ConvertDataUpFrom( pCommonBase, pObj, buf );
		pCommonBase->DestroyFunc( pObj );
		pObj = NULL;
	}

	if( pData )
		pObj = pData->NewObjFromData( buf, true, NULL );
	if( m_pClassMetaData )
		m_pClassMetaData->DestroyFunc( m_pObj );
	m_pClassMetaData = pData;
	m_nCastOffset = nOfs;
	m_bObjDataDirty = true;
	m_pObj = pObj;
	return true;
}

void CObjectPrototype::CopyData( CObjectPrototype& copyTo )
{
	copyTo.m_pBaseClass = m_pBaseClass;
	copyTo.m_pClassMetaData = m_pClassMetaData;
	copyTo.m_nCastOffset = m_nCastOffset;
	if( m_pClassMetaData )
	{
		static CBufFile g_buf;
		m_pClassMetaData->PackData( m_pObj, g_buf, false );
		copyTo.m_pObj = m_pClassMetaData->NewObjFromData( g_buf, false, NULL );
		g_buf.Clear();
	}
	copyTo.m_bObjDataDirty = true;
}

bool CObjectPrototype::DiffData( CObjectPrototype& obj1, CBufFile& buf )
{
	return m_pClassMetaData->DiffData( m_pObj, obj1.m_pObj, buf );
}

void CObjectPrototype::PatchData( CObjectPrototype& copyTo, IBufReader& patch, void* pContext )
{
	copyTo.m_pBaseClass = m_pBaseClass;
	copyTo.m_pClassMetaData = m_pClassMetaData;
	copyTo.m_nCastOffset = m_nCastOffset;
	if( m_pClassMetaData )
		copyTo.m_pObj = m_pClassMetaData->NewObjFromPatch( m_pObj, patch, pContext );
	copyTo.m_bObjDataDirty = true;
}

uint8* CObjectPrototype::CreateObject()
{
	if( !m_pClassMetaData )
		return NULL;
	if( m_bObjDataDirty )
	{
		m_objData.Clear();
		m_pClassMetaData->PackData( m_pObj, m_objData, false );
		m_bObjDataDirty = false;
	}
	m_objData.ResetCurPos();
	uint8* pData = m_pClassMetaData->NewObjFromData( m_objData, false, NULL );
	return pData + m_nCastOffset;
}

uint8 * CObjectPrototype::CreateObjectPatched( IBufReader& patch, void* pContext )
{
	if( !m_pClassMetaData )
		return NULL;
	uint8* pData = m_pClassMetaData->NewObjFromPatch( m_pObj, patch, pContext );
	return pData + m_nCastOffset;
}

void CObjectPrototype::Load( IBufReader& buf, bool bWithMetaData, void* pContext )
{
	string strClassName;
	buf.Read( strClassName );
	SetClassName( strClassName.c_str() );
	m_pObj = m_pClassMetaData->NewObjFromData( buf, bWithMetaData, pContext );
	m_bObjDataDirty = true;
}

void CObjectPrototype::Save( CBufFile& buf, bool bWithMetaData )
{
	string strClassName;
	if( m_pClassMetaData )
		strClassName = m_pClassMetaData->strClassName;
	buf.Write( strClassName );
	m_pClassMetaData->PackData( m_pObj, buf, bWithMetaData );
}

bool CObjectPrototype::ExtractData( IBufReader& bufIn, CBufFile& bufOut, function<bool( SClassMetaData*, SClassMetaData::SMemberData*, int32, IBufReader&, CBufFile& )>& handler )
{
	string strClassName;
	bufIn.Read( strClassName );
	bufOut.Write( strClassName );
	SClassMetaData* pData = CClassMetaDataMgr::Inst().GetClassData( strClassName.c_str() );
	return pData->ExtractData( false, bufIn, bufOut, handler );
}
