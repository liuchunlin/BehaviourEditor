#include "SActionRoot.h"
#include "NodeVerify.h"
#include "TEEManager.h"
#include "SpaceManager.h"

using namespace Space;
extern void Log(const wchar_t* pszFormat, ...);

//////////////////////////////////////////////////////////////////////////
//SActionRoot
IMPLEMENT_BLOCK_ALLOCATE(SActionRoot, 32)
SActionRoot::SActionRoot()
{
	ClassType(SNT_ACTIONROOT);
}

TiXmlElement*	SActionRoot::ToXMLElement()
{
	TiXmlElement *pXmlElement = SNode::ToXMLElement();
	return pXmlElement;
}
bool	SActionRoot::FromXMLElement( const TiXmlElement* pXMLElement )
{
	SNode::FromXMLElement( pXMLElement );

	return true;
}
bool	SActionRoot::Pasteable( const SNode* pSrc )
{
	if( pSrc == NULL )
		return false;

	if( IsActionType(pSrc->ClassType()) || pSrc->IsA(SNT_VARIABLE))
		return true;

	return false;
}
bool	SActionRoot::Deleteable()
{
	return false;
}
bool	SActionRoot::Permitable()
{
	return false;
}
