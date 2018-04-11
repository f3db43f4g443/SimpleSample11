#include "stdafx.h"
#include "ItemDrop.h"
#include "Common/Rand.h"
#include <algorithm>

void SItemDropContext::Drop( bool bForceDrop )
{
	if( !dropPool.size() )
		return;

	float s = 0;
	for( int i = 0; i < dropPool.size(); i++ )
	{
		s += dropPool[i].p;
		dropPool[i].p *= nDrop;
	}

	bool b = false;
	for( int i = 0; i < dropPool.size(); i++ )
	{
		if( dropPool[i].p >= s )
		{
			b = true;
			break;
		}
	}
	if( b )
	{
		struct SLess
		{
			bool operator () ( const SItemDropPoolItem & left, const SItemDropPoolItem & right )
			{
				return left.p > right.p;
			}
		};
		std::sort( dropPool.begin(), dropPool.end(), SLess() );
		if( !bForceDrop )
			nDrop = Min( nDrop, dropPool.size() );
		for( int i = 0; i < nDrop; i++ )
		{
			dropItems.push_back( dropPool[i % dropPool.size()].drop );
		}
		return;
	}

	SRand::Inst().Shuffle( dropPool );
	float r = SRand::Inst().Rand( 0.0f, s );
	float s1 = 0;
	for( int i = 0; i < dropPool.size() && dropItems.size() < nDrop; i++ )
	{
		s1 += dropPool[i].p;
		if( s1 > r || i < dropPool.size() - 1 )
		{
			dropItems.push_back( dropPool[i].drop );
			r += s;
		}
	}
}

void CItemDropSimple::Load( TiXmlElement * pXml, struct SItemDropNodeLoadContext& context )
{
	m_pPrefab = CResourceManager::Inst()->CreateResource<CPrefab>( XmlGetAttr( pXml, "name", "" ) );
	m_nPrice = XmlGetAttr( pXml, "price", 0 );
}

void CItemDropSimple::Generate( SItemDropContext & context )
{
	context.dropPool.resize( context.dropPool.size() + 1 );
	context.dropPool.back().drop.pPrefab = m_pPrefab;
	context.dropPool.back().drop.nPrice = m_nPrice;
	context.dropPool.back().p = 1;
}


void CItemDropSimpleGroup::Load( TiXmlElement * pXml, struct SItemDropNodeLoadContext& context )
{
	m_bSector = XmlGetAttr( pXml, "sector", 1 );
	m_bAverage = m_bSector;

	for( auto pChild = pXml->FirstChildElement(); pChild; pChild = pChild->NextSiblingElement() )
	{
		auto pChildNode = CItemDropNode::CreateNode( pChild, context );
		if( pChildNode )
		{
			m_subItems.resize( m_subItems.size() + 1 );
			m_subItems.back().pNode = pChildNode;
			m_subItems.back().fChance = XmlGetAttr( pChild, "p", 1.0f );
			m_subItems.back().bNormalize = XmlGetAttr( pChild, "normalize", 1 );
			if( m_subItems.back().fChance != 1.0f || !m_subItems.back().bNormalize )
				m_bAverage = false;
		}
	}
}

void CItemDropSimpleGroup::Generate( SItemDropContext & context )
{
	if( m_bSector )
	{
		SItemDropContext context1;

		if( m_bAverage )
		{
			uint32 nSize = context.nDrop;
			context1.nDrop = 1;
			int32 n = 0;
			for( int i = 0; i < Min( m_subItems.size(), nSize ); i++ )
			{
				uint32 r = SRand::Inst().Rand<uint32>( i, m_subItems.size() );
				if( r != i )
					swap( m_subItems[r], m_subItems[i] );
				m_subItems[i].pNode->Generate( context1 );
				context1.Drop( false );

				if( context1.dropItems.size() )
				{
					context.dropPool.resize( context.dropPool.size() + 1 );
					context.dropPool.back().drop = context1.dropItems[0];
					n++;
				}
				context1.Clear();
			}

			for( int i = 0; i < n; i++ )
			{
				context.dropPool[context.dropPool.size() - 1 - i].p = 1.0f / n;
			}

			return;
		}

		context1.nDrop = context.nDrop;
		_Generate( context1 );
		context1.Drop( false );

		for( auto& drop : context1.dropItems )
		{
			context.dropPool.resize( context.dropPool.size() + 1 );
			context.dropPool.back().drop = drop;
			context.dropPool.back().p = 1.0f / context1.dropItems.size();
		}
	}
	else
		_Generate( context );
}

void CItemDropSimpleGroup::_Generate( SItemDropContext & context )
{
	for( auto& item : m_subItems )
	{
		uint32 nPreSize = context.dropPool.size();
		item.pNode->Generate( context );
		uint32 nCurSize = context.dropPool.size();

		if( item.bNormalize )
		{
			float s = 0;

			for( int i = nPreSize; i < nCurSize; i++ )
				s += context.dropPool[i].p;
			float fScale = item.fChance / s;
			for( int i = nPreSize; i < nCurSize; i++ )
				context.dropPool[i].p *= fScale;
		}
		else
		{
			for( int i = nPreSize; i < nCurSize; i++ )
				context.dropPool[i].p *= item.fChance;
		}
	}
}

#define REGISTER_GENERATE_NODE( Name, Class ) m_mapCreateFuncs[Name] = [] ( TiXmlElement* pXml ) { return new Class; };
CItemDropNodeFactory::CItemDropNodeFactory()
{
	REGISTER_GENERATE_NODE( "item", CItemDropSimple );
	REGISTER_GENERATE_NODE( "group", CItemDropSimpleGroup );
}

CItemDropNode * CItemDropNodeFactory::LoadNode( TiXmlElement * pXml, struct SItemDropNodeLoadContext& context )
{
	auto itr = m_mapCreateFuncs.find( XmlGetAttr( pXml, "type", pXml->Value() ) );
	if( itr != m_mapCreateFuncs.end() )
	{
		auto pNode = itr->second( pXml );
		pNode->Load( pXml, context );
		return pNode;
	}
	return NULL;
}

CItemDropNode * CItemDropNode::CreateNode( TiXmlElement * pXml, struct SItemDropNodeLoadContext& context )
{
	if( !strcmp( pXml->Value(), "ref" ) )
	{
		const char* szName = XmlGetAttr( pXml, "name", "" );
		auto pNode = context.FindNode( szName );
		return pNode;
	}
	else
	{
		auto pNode = CItemDropNodeFactory::Inst().LoadNode( pXml, context );
		if( !pNode )
			return NULL;
		const char* szName = XmlGetAttr( pXml, "name", "" );
		if( szName[0] )
			context.mapNodes[szName] = pNode;
		return pNode;
	}
	return NULL;
}

void CBonusStageDrop::Load( TiXmlElement * pXml, SItemDropNodeLoadContext & context )
{
	auto pNodes = pXml->FirstChildElement( "nodes" );
	for( auto pChild = pNodes->FirstChildElement(); pChild; pChild = pChild->NextSiblingElement() )
	{
		auto pChildNode = CItemDropNode::CreateNode( pChild, context );
		m_vecNodes.push_back( pChildNode );
	}

	uint32 nNodes = m_vecNodes.size();
	auto pItems = pXml->FirstChildElement( "items" );
	for( auto pItem = pItems->FirstChildElement(); pItem; pItem = pItem->NextSiblingElement() )
	{
		m_vecItems.resize( m_vecItems.size() + 1 );
		auto& item = m_vecItems.back();
		item.nPoint = XmlGetAttr( pItem, "point", 0 );
		item.vecChance.resize( nNodes );
		const char* szChance = pItem->GetText();
		const char* c = szChance;
		int i = 0;
		stringstream ss( c );
		for( int i = 0; i < nNodes; i++ )
		{
			ss >> item.vecChance[i];
			ss.ignore( 100, ',' );
		}
	}
}

void CBonusStageDrop::Drop( SItemDropContext& result, uint32 nPoint )
{
	result.nDrop = 0;
	vector<float> vecChance;
	vecChance.resize( m_vecNodes.size() );
	int32 n;
	for( n = 0; n < m_vecItems.size(); n++ )
	{
		if( nPoint < m_vecItems[n].nPoint )
			break;
	}
	if( n >= m_vecItems.size() )
	{
		auto& item = m_vecItems.back();
		for( int i = 0; i < vecChance.size(); i++ )
		{
			vecChance[i] = item.vecChance[i];
		}
	}
	else
	{
		auto& item = m_vecItems[n - 1];
		auto& item1 = m_vecItems[n];
		int32 n1 = nPoint - item.nPoint;
		int32 n2 = item1.nPoint - nPoint;
		for( int i = 0; i < vecChance.size(); i++ )
		{
			vecChance[i] = ( item.vecChance[i] * n2 + item1.vecChance[i] * n1 ) / ( n1 + n2 );
		}
	}

	vector<uint32> vecIndices;
	for( int i = 0; i < vecChance.size(); i++ )
		vecIndices.push_back( i );
	SRand::Inst().Shuffle( vecIndices );

	float s = 0;
	for( int i = 0; i < vecChance.size(); i++ )
	{
		uint32 iIndex = vecIndices[i];
		s += vecChance[iIndex];
		vecChance[iIndex] = s;
	}
	int32 nCount = ceil( s );

	float r = SRand::Inst().Rand( 0.0f, 1.0f );
	n = 0;
	vector<float> vecDropCount;
	vecDropCount.resize( m_vecNodes.size() );
	for( int i = 0; i < nCount; i++, r += 1 )
	{
		while( n < vecDropCount.size() && vecChance[vecIndices[n]] <= r )
			n++;
		if( n >= vecDropCount.size() )
			break;
		vecDropCount[vecIndices[n]]++;
	}

	result.dropItems.reserve( nCount );
	for( int i = 0; i < vecDropCount.size(); i++ )
	{
		if( !vecDropCount[i] )
			continue;
		SItemDropContext context;
		context.nDrop = vecDropCount[i];
		m_vecNodes[i]->Generate( context );
		context.Drop();
		for( auto& item : context.dropItems )
		{
			result.dropItems.push_back( item );
			result.nDrop++;
		}
	}
	SRand::Inst().Shuffle( result.dropItems );
}
