#include "SForEach.h"
#include "NodeVerify.h"
#include "TEEManager.h"
#include "SpaceManager.h"
using namespace Space;

//////////////////////////////////////////////////////////////////////////
//SForEach
IMPLEMENT_BLOCK_ALLOCATE(SForEach, 32)
SForEach::SForEach()
{
	ClassType(SNT_FOR_EACH);
}
TiXmlElement*	SForEach::ToXMLElement()
{
	TiXmlElement *pXmlElement = SNode::ToXMLElement();
	pXmlElement->SetAttribute( "work", Working() );

	return pXmlElement;
}
bool	SForEach::FromXMLElement( const TiXmlElement* pXMLElement )
{
	SNode::FromXMLElement( pXMLElement );

	int bWork=1;
	pXMLElement->Attribute( "work", &bWork );
	Working( bWork != 0 );

	TEE::NodeBase* pTmpNode = TEE::GTEEMgr->FindFunctionFromCode(Name());
	assert( pTmpNode != NULL );
	TEETmp( pTmpNode );

	return true;
}
bool	SForEach::Cutable()
{
	return true;
}
bool	SForEach::Copyable()
{
	return true;
}
bool	SForEach::Pasteable( const SNode* pSrc )
{
	if ( pSrc == NULL )
		return false;
	if( Space::IsActionType(pSrc->ClassType()) )
		return true;
	return false;
}
bool	SForEach::Deleteable()
{
	return true;
}
bool	SForEach::GetLuaCode( wstring& luaCode, int depth )
{
	if( !Working() )
		return false;

	wstring varCode, beginCode, endCode, actionCode;
	SNode* pVar = NULL;
	SNode* pBegin = NULL;
	SNode* pEnd = NULL;
	SNode* pLoop = NULL;
	if( Name() == _T("ForEachAActions") )
	{
		varCode = _T("at_forLoopAIndex");
		pBegin = FirstChild();
		pEnd = pBegin->Next();
		pLoop = pEnd->Next();
	}
	else if( Name() == _T("ForEachBActions") )
	{
		varCode = _T("at_forLoopBIndex");
		pBegin = FirstChild();
		pEnd = pBegin->Next();
		pLoop = pEnd->Next();
	}
	else if( Name() == _T("ForEachActions") )
	{
		pVar = FirstChild();
		pBegin = pVar->Next();
		pEnd = pBegin->Next();
		pLoop = pEnd->Next();
		if( pVar->GetLuaCode(varCode, 0) )
		{
		}
	}

	if( varCode.empty() || pBegin == NULL || pEnd == NULL || pLoop == NULL )
	{
		assert( false );
		return false;
	}
	for( int k = 0; k < depth; ++k )
		luaCode += _T("\t");
	
	if( pBegin->GetLuaCode(beginCode, 0) )
	{
		luaCode += varCode + _T(" = ") + beginCode;
		luaCode += _T("\n");
	}
	if( pEnd->GetLuaCode(endCode, 0) )
	{
		for( int k = 0; k < depth; ++k )
			luaCode += _T("\t");
		luaCode += _T("while ") + varCode + _T(" <= ") + endCode + _T(" do");
		luaCode += _T("\n");
	}
	if( pLoop->GetLuaCode(actionCode, depth+1) )
	{
		luaCode += actionCode;

		for( int k = 0; k < depth+1; ++k )
			luaCode += _T("\t");
		luaCode += varCode + _T(" = ") + varCode + _T(" + 1");
		luaCode += _T("\n");
		for( int k = 0; k < depth; ++k )
			luaCode += _T("\t");
		luaCode += _T("end");
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
//SLoop
IMPLEMENT_BLOCK_ALLOCATE(SLoop, 32)
SLoop::SLoop()
{
	ClassType(SNT_LOOP);
}

TiXmlElement*	SLoop::ToXMLElement()
{
	TiXmlElement *pXmlElement = SNode::ToXMLElement();

	return pXmlElement;
}
bool	SLoop::FromXMLElement( const TiXmlElement* pXMLElement )
{
	SNode::FromXMLElement( pXMLElement );

	return true;
}
bool	SLoop::Pasteable( const SNode* pSrc )
{
	if( pSrc == NULL )
		return false;

	if( Space::IsActionType(pSrc->ClassType()) )
		return true;

	return false;
}
bool	SLoop::Deleteable()
{
	return false;
}
bool	SLoop::Permitable()
{
	return false;
}
bool	SLoop::GetLuaCode( wstring& luaCode, int depth )
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