#include "SEvent.h"
#include "NodeVerify.h"
#include "TEEManager.h"
#include "SpaceManager.h"

using namespace Space;

//////////////////////////////////////////////////////////////////////////
//SEventRoot
IMPLEMENT_BLOCK_ALLOCATE(SEventRoot, 32)
SEventRoot::SEventRoot()
{
	ClassType(SNT_EVENTROOT);
}
TiXmlElement*	SEventRoot::ToXMLElement()
{
	TiXmlElement *pXmlElement = SNode::ToXMLElement();

	return pXmlElement;
}
bool	SEventRoot::FromXMLElement( const TiXmlElement* pXMLElement )
{
	SNode::FromXMLElement( pXMLElement );

	return true;
}
bool	SEventRoot::Pasteable( const SNode* pSrc )
{
	if( pSrc == NULL )
		return false;

	if( pSrc->ClassType() == SNT_EVENT || pSrc->ClassType() == SNT_CONDITION || pSrc->ClassType() == SNT_TFUNCCALL  )
		return true;

	return false;
}
bool	SEventRoot::Deleteable()
{
	return false;
}
bool	SEventRoot::Permitable()
{
	return false;
}

//////////////////////////////////////////////////////////////////////////
//SEvent
IMPLEMENT_BLOCK_ALLOCATE(SEvent, 64)
SEvent::SEvent()
{
	ClassType(SNT_EVENT);
}
TiXmlElement*	SEvent::ToXMLElement()
{
	TiXmlElement *pXmlElement = SNode::ToXMLElement();
	pXmlElement->SetAttribute( "work", Working() );

	return pXmlElement;
}
bool	SEvent::FromXMLElement( const TiXmlElement* pXMLElement )
{
	SNode::FromXMLElement( pXMLElement );

	TEE::NodeBase* pTmpNode = TEE::GTEEMgr->FindEventFromCode(Name());
	assert( pTmpNode != NULL );
	TEETmp( pTmpNode );

	int bWork=1;
	pXMLElement->Attribute( "work", &bWork );
	Working( bWork != 0 );
	return true;
}

bool	SEvent::GetContextView( ContextView& cnxt )
{
	cnxt.pNode = this;
	cnxt.bValid = GetErrorMask() == VS_OK;
	cnxt.bPreferences = true;
	cnxt.text = GetReadText();

	if( cnxt.text.empty() )
	{
		cnxt.text = _T("Event");
		cnxt.bValid = false;
	}
	else
	{
		cnxt.text = _T("(") + cnxt.text + _T(")") ;
	}

	return true;
}

wstring	SEvent::GetReadText() const
{
	if( TEETmp() == NULL || TEETmp()->m_tag.empty() )
		return SNode::GetReadText();

	return _T("(") + TEETmp()->m_tag + _T(" - ") + SNode::GetReadText() + _T(")");
}
bool	SEvent::PostXMLLoaded()
{
	Space::GSpaceMgr->VerifyParameters( this );
	return true;
}

ErrorMask	SEvent::CheckError()
{
	Space::GSpaceMgr->CheckParametersValidate( this );
	return GetErrorMask();
}
bool	SEvent::Pasteable( const SNode* pSrc )
{
	if( pSrc == NULL )
		return false;

	if( pSrc->ClassType() == SNT_EVENT || pSrc->ClassType() == SNT_CONDITION || pSrc->ClassType() == SNT_TFUNCCALL  )
		return true;

	return false;
}
bool	SEvent::Cutable()
{
	return true;
}
bool	SEvent::Copyable()
{
	return true;
}
bool	SEvent::GetLuaCode( wstring& luaCode, int depth )
{
	if( !Working() )
		return false;

	return true;
}
