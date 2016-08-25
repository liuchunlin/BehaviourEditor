#include "SRoot.h"

#include "XmlUtility.h"

using namespace Space;

//////////////////////////////////////////////////////////////////////////
//SRoot
IMPLEMENT_BLOCK_ALLOCATE(SRoot, 1)
SRoot::SRoot()
{
	ClassType(SNT_ROOT);
}
TiXmlElement*	SRoot::ToXMLElement()
{
	TiXmlElement *pXmlElement = SNode::ToXMLElement();

	return pXmlElement;
}
bool	SRoot::FromXMLElement( const TiXmlElement* pXMLElement )
{
	SNode::FromXMLElement( pXMLElement );

	return true;
}

//Menu
bool	SRoot::Cutable()
{
	return false;
}
bool	SRoot::Copyable()
{
	return false;
}
bool	SRoot::Pasteable( const SNode* pSrc )
{
	if ( pSrc == NULL )
		return false;
	if( pSrc->ClassType() != Space::SNT_GROUP )
		return false;
	return true;
}
bool	SRoot::Deleteable()
{
	return false;
}
bool	SRoot::Disableable()
{
	return false;
}
bool	SRoot::Permitable()
{
	return false;
}
