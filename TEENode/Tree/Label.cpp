#include "Label.h"
#include "NodeFactory.h"
#include "TEEManager.h"
using namespace TEE;


//////////////////////////////////////////////////////////////////////////
//Label
IMPLEMENT_BLOCK_ALLOCATE(Label, 64)
Label::Label()
{
	m_classType = (NT_LABEL);
}

TiXmlElement*	Label::ToXMLElement()
{
	TiXmlElement *pXmlElement = NodeBase::ToXMLElement();

	if( !m_strLabel.empty() )
	{
		TiXmlText *pText = new TiXmlText( (char*)FTCHARToUTF8(m_strLabel.c_str()) );
		pText->SetCDATA( true );
		pXmlElement->LinkEndChild( pText );
	}

	return pXmlElement;
}
bool	Label::FromXMLElement( const TiXmlElement* pXMLElement )
{
	if( pXMLElement->FirstChild() != NULL && pXMLElement->FirstChild()->Type() == TiXmlNode::TEXT )
	{
		const TiXmlText* pText = pXMLElement->FirstChild()->ToText();
		m_strLabel = (TCHAR*)FUTF8ToTCHAR(pText->Value());
	}
	return NodeBase::FromXMLElement( pXMLElement );
}

NodeBase* Label::DoClone() const
{
	Label* pClone = (Label*)NodeBase::DoClone();
	pClone->m_strLabel = m_strLabel;
	return pClone;
}

void	Label::AddExtraNotesContent( wstring& content )
{
	if( Parent() == NULL )
		return ;
	if( Parent()->m_classType == NT_TAGROOT )
		content += _T("\n");
}
bool	Label::ToRichString( list<TEE::RichText>& records )
{
	if( !m_strLabel.empty() )
	{
		RichText record;
		record.content = m_strLabel;
		AddExtraNotesContent( record.content );
		records.push_back( record );
	}
	return true;
}


bool	Label::GetNotesViewString( list<TEE::RichText>& records )
{
	bool bValidNode = m_errorMask == ERR_OK;

	if( !m_strLabel.empty() )
	{
		RichText record;
		record.content = m_strLabel;
		AddExtraNotesContent( record.content );
		record.eColor = !bValidNode ? TEE::RC_RED : TEE::RC_BLACK;
		records.push_back( record );
	}

	return true;
}
bool	Label::GetNotesCodeString( list<TEE::RichText>& records )
{
	return true;
}
wstring	Label::GetReadableString() const
{
	return m_strLabel;
}

bool	Label::GetCPPCode( wstring& cppCode ) const
{
	return true;
}
