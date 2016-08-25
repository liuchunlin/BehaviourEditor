#include "Space.h"
#include "NodeFactory.h"
#include "TEEManager.h"
#include "Adjust.h"
using namespace TEE;
//////////////////////////////////////////////////////////////////////////
//SpaceRoot
IMPLEMENT_BLOCK_ALLOCATE(SpaceRoot, 1)
SpaceRoot::SpaceRoot()
{
	m_classType = (NT_SPACEROOT);
}
TiXmlElement*	SpaceRoot::ToXMLElement()
{
	TiXmlElement *pXmlElement = NodeBase::ToXMLElement();
	return pXmlElement;
}
bool	SpaceRoot::FromXMLElement( const TiXmlElement* pXMLElement )
{
	return NodeBase::FromXMLElement( pXMLElement );
}
bool	SpaceRoot::ToRichString( list<TEE::RichText>& records )
{
	return true;
}
bool	SpaceRoot::GetNotesViewString( list<TEE::RichText>& records )
{
	NodeBase* pChile = FirstChild();
	for( ; pChile != NULL; pChile = pChile->Next() )
	{
		pChile->GetNotesViewString( records );
	}
	return true;
}
bool	SpaceRoot::NewSpaceable()
{
	return true;
}
