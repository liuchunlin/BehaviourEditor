#include "EventCode.h"
#include "NodeFactory.h"
#include "TEEManager.h"
using namespace TEE;


//////////////////////////////////////////////////////////////////////////
//EventCode
IMPLEMENT_BLOCK_ALLOCATE(EventCode, 4)

EventCode::EventCode()
{
	m_classType = (NT_EVENTCODE);
}

TiXmlElement*	EventCode::ToXMLElement()
{
	TiXmlElement *pXmlElement = NodeBase::ToXMLElement();

	if( !m_strCode.empty() )
	{
		TiXmlText *pText = new TiXmlText( (char*)FTCHARToUTF8(m_strCode.c_str()) );
		pText->SetCDATA( true );
		pXmlElement->LinkEndChild( pText );
	}

	return pXmlElement;
}
bool	EventCode::FromXMLElement( const TiXmlElement* pXMLElement )
{
	if( pXMLElement->FirstChild() != NULL && pXMLElement->FirstChild()->Type() == TiXmlNode::TEXT )
	{
		const TiXmlText* pText = pXMLElement->FirstChild()->ToText();
		m_strCode = (TCHAR*)FUTF8ToTCHAR(pText->Value());
	}
	return NodeBase::FromXMLElement( pXMLElement );
}

NodeBase* EventCode::DoClone() const
{
	EventCode* pClone = (EventCode*)NodeBase::DoClone();
	pClone->m_strCode = m_strCode;
	return pClone;
}

bool	EventCode::ToRichString( list<TEE::RichText>& records )
{
	RichText recordPre;
	recordPre.content = _T("\neventCode={");
	records.push_back( recordPre );

	RichText recordCode;
	recordCode.content = m_strCode;
	records.push_back( recordCode );

	RichText recordPost;
	recordPost.content = _T("}");
	records.push_back( recordPost );
	return true;
}

bool	EventCode::GetNotesViewString( list<TEE::RichText>& records )
{
	return true;
}
bool	EventCode::GetNotesCodeString( list<TEE::RichText>& records )
{
	return ToRichString( records );
}

//////////////////////////////////////////////////////////////////////////
//EventCode
IMPLEMENT_BLOCK_ALLOCATE(MsgEventCode, 4)

MsgEventCode::MsgEventCode()
{
	m_classType = (NT_MSGEVENTCODE);
}

bool	MsgEventCode::ToRichString( list<TEE::RichText>& records )
{
	RichText recordPre;
	recordPre.content = _T("\nmsgEventCode={");
	records.push_back( recordPre );

	RichText recordCode;
	recordCode.content = m_strCode;
	records.push_back( recordCode );

	RichText recordPost;
	recordPost.content = _T("}");
	records.push_back( recordPost );
	return true;
}
