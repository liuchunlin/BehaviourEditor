#include "Include.h"
#include "NodeFactory.h"
#include "TEEManager.h"
using namespace TEE;
//////////////////////////////////////////////////////////////////////////
//IncludeRoot
IMPLEMENT_BLOCK_ALLOCATE(IncludeRoot, 1)
IncludeRoot::IncludeRoot()
{
	m_classType = (NT_INCLUDEROOT);
}
TiXmlElement*	IncludeRoot::ToXMLElement()
{
	TiXmlElement *pXmlElement = NodeBase::ToXMLElement();
	return pXmlElement;
}
bool	IncludeRoot::FromXMLElement( const TiXmlElement* pXMLElement )
{
	return NodeBase::FromXMLElement( pXMLElement );
}
bool	IncludeRoot::ToRichString( list<TEE::RichText>& records )
{
	return true;
}
bool	IncludeRoot::GetNotesViewString( list<TEE::RichText>& records )
{
	RichText record;
	record.content = _T("Ô¤ÀÀ: ") + m_DisplayName + _T("\n");
	record.eColor = TEE::RC_BLACK;
	records.push_back( record );

	NodeBase* pChile = FirstChild();
	for( ; pChile != NULL; pChile = pChile->Next() )
	{
		pChile->GetNotesViewString( records );
	}
	return true;
}
bool	IncludeRoot::Propertyable()
{
	return true;
}
//////////////////////////////////////////////////////////////////////////
//Include
IMPLEMENT_BLOCK_ALLOCATE(Include, 8)
Include::Include()
{
	m_classType = (NT_INCLUDE);
}

TiXmlElement*	Include::ToXMLElement()
{
	TiXmlElement *pXmlElement = NodeBase::ToXMLElement();

	return pXmlElement;
}

bool	Include::FromXMLElement( const TiXmlElement* pXMLElement )
{
	return NodeBase::FromXMLElement( pXMLElement );
}

bool	Include::ToRichString( list<TEE::RichText>& records )
{
	NodeBase::ToRichString( records );
	return true;
}
bool	Include::GetNotesViewString( list<TEE::RichText>& records )
{
	RichText record;
	record.bUnderLine = false;
	record.eColor = TEE::RC_BLUE;
	record.content = m_DisplayName + _T("\n");
	records.push_back(record);

	NodeBase::GetNotesViewString( records );
	return true;
}
wstring	Include::GetReadableString() const
{
	return wstring();
}
bool	Include::Deleteable()
{
	return true;
}
ErrorMask	Include::CheckError()
{
	ClearErrorFlag( ERR_EVERYTHING );

	ErrorMask flag = GTEEMgr->CheckIncludeValidate( this );
	AddErrorFlag( flag );
	return GetErrorMask();
}
