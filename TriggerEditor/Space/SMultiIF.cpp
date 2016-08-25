#include "SMultiIF.h"
#include "NodeVerify.h"
#include "TEEManager.h"
#include "SpaceManager.h"
using namespace Space;

//////////////////////////////////////////////////////////////////////////
//SMultiIF
IMPLEMENT_BLOCK_ALLOCATE(SMultiIF, 32)
SMultiIF::SMultiIF()
{
	ClassType(SNT_MULTIPLE_IF);
}
TiXmlElement*	SMultiIF::ToXMLElement()
{
	TiXmlElement *pXmlElement = SNode::ToXMLElement();
	pXmlElement->SetAttribute( "work", Working() );

	return pXmlElement;
}
bool	SMultiIF::FromXMLElement( const TiXmlElement* pXMLElement )
{
	SNode::FromXMLElement( pXMLElement );

	TEE::NodeBase* pTmpNode = TEE::GTEEMgr->FindFunctionFromCode(Name());
	assert( pTmpNode != NULL );
	TEETmp( pTmpNode );

	int bWork=1;
	pXMLElement->Attribute( "work", &bWork );
	Working( bWork != 0 );

	return true;
}
bool	SMultiIF::Cutable()
{
	return true;
}
bool	SMultiIF::Copyable()
{
	return true;
}
bool	SMultiIF::Pasteable( const SNode* pSrc )
{
	if ( pSrc == NULL )
		return false;
	if( Space::IsActionType(pSrc->ClassType()) || pSrc->ClassType() == SNT_CONDITION )
		return true;
	return false;
}
bool	SMultiIF::Deleteable()
{
	return true;
}
bool	SMultiIF::GetLuaCode( wstring& luaCode, int depth )
{
	if( !Working() )
		return false;

	for( int k = 0; k < depth; ++k )
		luaCode += _T("\t");

	wstring childCode;
	//condition
	luaCode += _T("if ");
	SNode* pChild = FirstChild();
	if( pChild == NULL )
		return false;

	childCode.clear();
	if( pChild->GetLuaCode(childCode, 0) )
	{
		luaCode += _T("(");
		luaCode += childCode;
		luaCode += _T(") then\n");
	}

	//then
	pChild = pChild->Next();
	if( pChild == NULL )
		return false;

	childCode.clear();
	if( pChild->GetLuaCode(childCode, depth+1) )
	{
		luaCode += childCode;
	}

	//else
	pChild = pChild->Next();
	if( pChild == NULL )
		return false;

	childCode.clear();
	if( pChild->GetLuaCode(childCode, depth+1) )
	{
		luaCode += childCode;
	}

	//luaCode += _T("if ");
	//luaCode += _T(" then");
	//for ( INode* pChild = FirstChild(); pChild != NULL; pChild = pChild->Next() )
	//{
	//	wstring childCode;
	//	if( pChild->GetLuaCode(childCode, depth+1) )
	//	{
	//		luaCode += childCode;
	//		luaCode += _T("\n");
	//	}
	//}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//SIF
IMPLEMENT_BLOCK_ALLOCATE(SIF, 32)
SIF::SIF()
{
	ClassType(SNT_IF);
}

TiXmlElement*	SIF::ToXMLElement()
{
	TiXmlElement *pXmlElement = SNode::ToXMLElement();

	return pXmlElement;
}
bool	SIF::FromXMLElement( const TiXmlElement* pXMLElement )
{
	SNode::FromXMLElement( pXMLElement );

	return true;
}
bool	SIF::Pasteable( const SNode* pSrc )
{
	if( pSrc == NULL )
		return false;

	if( pSrc->ClassType() == SNT_CONDITION )
		return true;

	return false;
}
bool	SIF::Deleteable()
{
	return false;
}
bool	SIF::Permitable()
{
	return false;
}
bool	SIF::GetLuaCode( wstring& luaCode, int depth )
{
	Space::SNode* pChild = FirstChild();
	if( pChild == NULL )
	{
		luaCode += _T(" (true) ");
		return true;
	}
	for ( ; pChild != NULL; pChild = pChild->Next() )
	{
		if( pChild->Prev() )
		{
			luaCode += _T(" and ");
		}
		wstring childCode;
		if( pChild->GetLuaCode( childCode, 0 ) )
		{
			luaCode += _T("(") + childCode + _T(")");
		}
		else
		{
			luaCode += _T("(true)");
		}
	}
	return true;
}
//////////////////////////////////////////////////////////////////////////
//SThen
IMPLEMENT_BLOCK_ALLOCATE(SThen, 32)
SThen::SThen()
{
	ClassType(SNT_THEN);
}

TiXmlElement*	SThen::ToXMLElement()
{
	TiXmlElement *pXmlElement = SNode::ToXMLElement();

	return pXmlElement;
}
bool	SThen::FromXMLElement( const TiXmlElement* pXMLElement )
{
	SNode::FromXMLElement( pXMLElement );

	return true;
}
bool	SThen::Pasteable( const SNode* pSrc )
{
	if( pSrc == NULL )
		return false;

	if( Space::IsActionType(pSrc->ClassType()) )
		return true;

	return false;
}
bool	SThen::Deleteable()
{
	return false;
}
bool	SThen::Permitable()
{
	return false;
}
bool	SThen::GetLuaCode( wstring& luaCode, int depth )
{
	Space::SNode* pChild = FirstChild();
	for ( ; pChild != NULL; pChild = pChild->Next() )
	{
		wstring childCode;
		if( pChild->GetLuaCode( childCode, depth ) )
		{
			luaCode += childCode;
			luaCode += _T("\n");
		}
	}

	return true;
}
//////////////////////////////////////////////////////////////////////////
//SElse
IMPLEMENT_BLOCK_ALLOCATE(SElse, 32)
SElse::SElse()
{
	ClassType(SNT_ELSE);
}

TiXmlElement*	SElse::ToXMLElement()
{
	TiXmlElement *pXmlElement = SNode::ToXMLElement();

	return pXmlElement;
}
bool	SElse::FromXMLElement( const TiXmlElement* pXMLElement )
{
	SNode::FromXMLElement( pXMLElement );

	return true;
}
bool	SElse::Pasteable( const SNode* pSrc )
{
	if( pSrc == NULL )
		return false;

	if( Space::IsActionType(pSrc->ClassType()) )
		return true;

	return false;
}
bool	SElse::Deleteable()
{
	return false;
}
bool	SElse::Permitable()
{
	return false;
}
bool	SElse::GetLuaCode( wstring& luaCode, int depth )
{
	for( int k = 0; k < depth-1; ++k )
		luaCode += _T("\t");
	luaCode += _T("else\n");

	Space::SNode* pChild = FirstChild();
	for ( ; pChild != NULL; pChild = pChild->Next() )
	{
		wstring childCode;
		if( pChild->GetLuaCode( childCode, depth ) )
		{
			luaCode += childCode;
			luaCode += _T("\n");
		}
	}
	for( int k = 0; k < depth-1; ++k )
		luaCode += _T("\t");
	luaCode += _T("end");

	return true;
}