#include "Adjust.h"
#include "NodeFactory.h"
#include "Option.h"
using namespace TEE;

//////////////////////////////////////////////////////////////////////////
//Adjust
IMPLEMENT_BLOCK_ALLOCATE(Adjust, 64)
Adjust::Adjust()
{
	m_classType = (NT_ADJUST);
}
TiXmlElement*	Adjust::ToXMLElement()
{
	//CodeName( _T("") );
	//if( m_DisplayName.empty() )
	//{
	//	assert(false);
	//	DisplayName( m_Key );
	//}
	TiXmlElement *pXmlElement = NodeBase::ToXMLElement();
	//pXmlElement->SetAttribute( "Key", FTCHARToUTF8( m_Key.c_str()) );
	//pXmlElement->SetAttribute( "Value", FTCHARToUTF8( m_Value.c_str()) );
	return pXmlElement;
}
bool	Adjust::FromXMLElement( const TiXmlElement* pXMLElement )
{
	//GetXMLElementAttribute( pXMLElement, "Key",		m_Key );
	//GetXMLElementAttribute( pXMLElement, "Value",	m_Value );

	bool bSuccess = NodeBase::FromXMLElement( pXMLElement );
	if( m_DisplayName.empty() )
	{
		GetXMLElementAttribute( pXMLElement, "Key",	m_DisplayName );
	}
	if( m_CodeName.empty() )
	{
		GetXMLElementAttribute( pXMLElement, "Value",	m_CodeName );
	}
	return bSuccess;
}

bool	Adjust::ToRichString( list<TEE::RichText>& records )
{
	RichText record;
	record.bUnderLine = false;
	record.eColor = TEE::RC_BLACK;
	record.content = m_DisplayName + _T("=") + m_CodeName;
	if( Next() != NULL )
		record.content += _T("\n");
	records.push_back(record);

	return true;
}
bool	Adjust::GetNotesViewString( list<TEE::RichText>& records )
{
	bool bValidNode = true;
	if(m_errorMask & ERR_INVALID_FORMAT) 
		bValidNode = false;

	RichText record;
	record.bUnderLine = false;
	record.eColor = bValidNode ? TEE::RC_BLACK : TEE::RC_RED;
	record.content = m_DisplayName + _T("=");
	records.push_back(record);
	record.Clear();

	Option* pParent = dynamic_cast<Option*>(Parent());
	if( !bValidNode || !pParent )
	{
		record.eColor = TEE::RC_RED;
	}
	else
	{
		//record.eColor = pParent->m_eValueType == VT_NUMERICAL ? TEE::RC_BLUE : 
		//				pParent->m_eValueType == VT_STRING ? TEE::RC_GREEN :
		//				pParent->m_eValueType == VT_BOOLEAN ? TEE::RC_GREY : RC_BLACK;
		record.eColor = TEE::RC_BLUE;
	}
	if( pParent != NULL && pParent->m_eValueType == VT_STRING )
	{
		record.content = _T("\"") + m_CodeName +_T("\"");
	}
	else if( pParent != NULL &&pParent->m_eValueType == VT_BOOLEAN  )
	{
		if( !m_CodeName.empty() )
		{
			//int iValue = _wtoi( m_CodeName.c_str() );
			//record.content = (iValue) != 0 ? _T("True") : _T("False");
			record.content = m_CodeName;
		}
	}
	else
	{
		record.content = m_CodeName;
	}
	if( Next() != NULL )
		record.content += _T("\n");
	records.push_back(record);
	return true;
}

bool	Adjust::GetOptionCode( wstring& valueType )
{
	if( Parent() == NULL )
		return false;
	valueType = Parent()->CodeName();
	return true;
}
wstring	Adjust::GetReadableString() const
{
	return m_DisplayName;
}
