#include "Event.h"
#include "NodeFactory.h"
#include "TEEManager.h"
#include "NodeVerify.h"

using namespace TEE;

//////////////////////////////////////////////////////////////////////////
//EventRoot
IMPLEMENT_BLOCK_ALLOCATE(EventRoot, 1)
EventRoot::EventRoot()
{
	m_classType = (NT_EVENTROOT);
}

TiXmlElement*	EventRoot::ToXMLElement()
{
	TiXmlElement *pXmlElement = NodeBase::ToXMLElement();
	return pXmlElement;
}
bool			EventRoot::FromXMLElement( const TiXmlElement* pXMLElement )
{
	return NodeBase::FromXMLElement( pXMLElement );
}
bool	EventRoot::NewEventable()
{
	return true;
}
//////////////////////////////////////////////////////////////////////////
//Event
IMPLEMENT_BLOCK_ALLOCATE(Event, 64)
Event::Event()
{
	m_classType = (NT_EVENT);
}
bool	Event::FromString( const wstring& content )
{
	RemoveChildren();
	GNodeFactory.BuildNormalChild( content, this );
	NodeBase::CheckErrorTree(this);

	return true;
}


TiXmlElement*	Event::ToXMLElement()
{
	TiXmlElement *pXmlElement = NodeBase::ToXMLElement();
	return pXmlElement;
}
bool			Event::FromXMLElement( const TiXmlElement* pXMLElement )
{
	return NodeBase::FromXMLElement( pXMLElement );
}
bool			Event::GetNotesCodeString( list<TEE::RichText>& records )
{
	RichText record;
	record.content = m_CodeName + _T(" {\n");
	record.eColor = TEE::RC_BLACK;
	records.push_front( record );

	NodeBase::GetNotesCodeString( records );

	record.Clear();
	record.content = _T("\n}");
	record.eColor = TEE::RC_BLACK;
	records.push_back( record );
	return true;
}
bool	Event::GetNotesViewString( list<TEE::RichText>& records )
{
	RichText record;
	record.content = _T("Ô¤ÀÀ: ") + m_DisplayName + _T("\n");
	record.eColor = TEE::RC_BLACK;
	records.push_back( record );

	record.Clear();
	record.content = _T("±êÇ©: ");
	record.eColor = TEE::RC_BLACK;
	records.push_back( record );

	GTEEMgr->GetTagViewString( records, m_tag );

	return NodeBase::GetNotesViewString( records );
}
ErrorMask	Event::CheckError()
{
	ClearErrorFlag( ERR_EVERYTHING );

	ErrorMask flag = GTEEMgr->CheckTagValidate( m_tag );
	AddErrorFlag( flag );

	flag = GTEEMgr->CheckParametersValidate( this );
	AddErrorFlag( flag );

	flag = GTEEMgr->CheckNameValidate( this );
	AddErrorFlag( flag );

	if ( NodeBase* pContextRoot = FindFirstChild(VerifyNodeType(NT_CONTEXTROOT)) )
	{
		flag = GTEEMgr->CheckContextParamsValidate(pContextRoot);
		AddErrorFlag( flag );
	}
	return GetErrorMask();
}
bool	Event::NewEventable()
{
	return true;
}
bool	Event::Deleteable()
{
	return true;

}
bool	Event::Propertyable()
{
	return true;
}
bool	Event::GetOptionCode( wstring& valueType )
{
	valueType = _T("EVENT");
	return true;
}