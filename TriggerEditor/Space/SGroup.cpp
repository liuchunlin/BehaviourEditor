#include "SGroup.h"

using namespace Space;

//////////////////////////////////////////////////////////////////////////
//SGroup
IMPLEMENT_BLOCK_ALLOCATE(SGroup, 16)
SGroup::SGroup()
{
	ClassType(SNT_GROUP);
}
TiXmlElement*	SGroup::ToXMLElement()
{
	TiXmlElement *pXmlElement = SNode::ToXMLElement();
	pXmlElement->SetAttribute( "work", Working() );

	return pXmlElement;
}
bool	SGroup::FromXMLElement( const TiXmlElement* pXMLElement )
{
	SNode::FromXMLElement( pXMLElement );
	int bWork=1;
	pXMLElement->Attribute( "work", &bWork );
	Working( bWork != 0 );
	return true;
}
bool	SGroup::Cutable()
{
	return true;
}
bool	SGroup::Copyable()
{
	return true;
}
bool	SGroup::Pasteable( const SNode* pSrc )
{
	if ( pSrc == NULL )
		return false;
	if( pSrc->ClassType() != Space::SNT_TRIGGER && pSrc->ClassType() != Space::SNT_GROUP )
		return false;
	return true;
}
bool	SGroup::Deleteable()
{
	return true;
}
bool	SGroup::UniqueName() const
{
	return true;
}
