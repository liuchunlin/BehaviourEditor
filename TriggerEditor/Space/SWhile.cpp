#include "SWhile.h"
#include "NodeVerify.h"
#include "TEEManager.h"
#include "SpaceManager.h"
using namespace Space;

//////////////////////////////////////////////////////////////////////////
//SWhile
IMPLEMENT_BLOCK_ALLOCATE(SWhile, 32)
SWhile::SWhile()
{
	ClassType(SNT_WHILE);
}
TiXmlElement*	SWhile::ToXMLElement()
{
	TiXmlElement *pXmlElement = SNode::ToXMLElement();
	pXmlElement->SetAttribute( "work", Working() );

	return pXmlElement;
}
bool	SWhile::FromXMLElement( const TiXmlElement* pXMLElement )
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
bool	SWhile::Cutable()
{
	return true;
}
bool	SWhile::Copyable()
{
	return true;
}
bool	SWhile::Pasteable( const SNode* pSrc )
{
	if ( pSrc == NULL )
		return false;
	if( IsActionType(pSrc->ClassType()) ||
		pSrc->ClassType() == SNT_CONDITION )
		return true;
	return false;
}
bool	SWhile::Deleteable()
{
	return true;
}
bool	SWhile::GetLuaCode( wstring& luaCode, int depth )
{
	if( !Working() )
		return false;

	wstring tabs;
	for( int k = 0; k < depth; ++k )
		tabs += _T("\t");
	luaCode += tabs;

	wstring childCode;

	//condition
	luaCode += _T("while ");
	SNode* pChild = FirstChild();
	assert(pChild);
	if( pChild == NULL )
		return false;
	childCode.clear();
	if( pChild->GetLuaCode(childCode, 0) )
	{
		luaCode += _T("(");
		luaCode += childCode;
		luaCode += _T(") do\n");
	}

	//loop statements
	pChild = pChild->Next();
	assert(pChild);
	if( pChild == NULL )
		return false;
	childCode.clear();
	if( pChild->GetLuaCode(childCode, depth+1) )
	{
		luaCode += childCode;
	}

	luaCode += tabs;
	luaCode += _T("end\n");

	return true;
}
