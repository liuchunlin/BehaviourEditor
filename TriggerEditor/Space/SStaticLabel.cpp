#include "SStaticLabel.h"

using namespace Space;


//////////////////////////////////////////////////////////////////////////
//SStaticLabel
IMPLEMENT_BLOCK_ALLOCATE(SStaticLabel, 32)
SStaticLabel::SStaticLabel()
{
	ClassType(SNT_STATICLABEL);
}
TiXmlElement*	SStaticLabel::ToXMLElement()
{
	TiXmlElement *pXmlElement = SNode::ToXMLElement();

	return pXmlElement;
}
bool	SStaticLabel::FromXMLElement( const TiXmlElement* pXMLElement )
{
	SNode::FromXMLElement( pXMLElement );

	return true;
}
bool	SStaticLabel::GetContextView( ContextView& cnxt )
{
	cnxt.pNode = this;
	cnxt.bValid = true;
	cnxt.bPreferences = false;
	cnxt.text = Name();
	return true;
}