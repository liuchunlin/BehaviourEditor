#include "Parameter.h"
#include "NodeFactory.h"
#include "TEEManager.h"
#include "NodeVerify.h"
using namespace TEE;

#include <strsafe.h>
//////////////////////////////////////////////////////////////////////////
//Parameter
IMPLEMENT_BLOCK_ALLOCATE(Parameter, 64)
Parameter::Parameter()
{
	m_classType = (NT_PARAMETER);
}

TiXmlElement*	Parameter::ToXMLElement()
{
	TiXmlElement *pXmlElement = NodeBase::ToXMLElement();
	pXmlElement->SetAttribute( "Default", FTCHARToUTF8( m_DefaultValue.c_str()) );
	return pXmlElement;
}
bool	Parameter::FromXMLElement( const TiXmlElement* pXMLElement )
{
	GetXMLElementAttribute( pXMLElement, "Default", m_DefaultValue );
	return NodeBase::FromXMLElement( pXMLElement );
}

NodeBase* Parameter::DoClone() const
{
	Parameter* pClone = (Parameter*)NodeBase::DoClone();
	pClone->m_DefaultValue = m_DefaultValue;
	return pClone;
}

bool	Parameter::ToRichString( list<TEE::RichText>& records )
{
	RichText record;
	record.content = _T("<") + m_DisplayName + _T(" ") + m_CodeName;
	if( !m_DefaultValue.empty() )
		record.content += _T(" = ") + m_DefaultValue;
	record.content += _T(">");
	records.push_back( record );

	return true;
}
bool	Parameter::GetNotesViewString( list<TEE::RichText>& records )
{
	bool bValidNode = true;
	if(m_errorMask & ERR_INVALID_FORMAT) 
		bValidNode = false;
	NodeBase* pOption = GTEEMgr->FindOptionFromCode(m_DisplayName);
	if ( pOption == NULL )
		bValidNode = false;

	RichText record;
	if( !m_DefaultValue.empty() )
	{
		record.eColor = !bValidNode ? TEE::RC_RED : TEE::RC_BLUE;
		record.content = _T(" ") + m_DefaultValue + _T(" ");
		record.bUnderLine = true;
		records.push_back( record );
	}
	else
	{
		if ( pOption == NULL )
		{
			record.eColor = TEE::RC_RED;
			record.content = _T(" ") + m_DisplayName + _T(" ");
		}
		else
		{
			record.eColor = !bValidNode ? TEE::RC_RED : TEE::RC_BLUE;
			record.content = _T(" ") + pOption->m_DisplayName + _T(" ");
		}
		record.bUnderLine = true;
		records.push_back( record );
	}
	return true;
}

ErrorMask	Parameter::CheckError()
{
	ClearErrorFlag(ERR_EVERYTHING);
	if (m_DisplayName.empty() || m_CodeName.empty())
	{
		AddErrorFlag( ERR_INVALID_FORMAT );
	}
	NodeBase* pOption = GTEEMgr->FindOptionFromCode(m_DisplayName);
	if ( pOption == NULL )
	{
		AddErrorFlag( ERR_INVALID_FORMAT );
	}
	return GetErrorMask();
}
wstring	Parameter::GetReadableString() const
{
	return NodeBase::GetReadableString();
}
bool	Parameter::GetOptionCode( wstring& valueType )
{
	valueType = m_DisplayName;
	return true;
}

extern bool g_isGenCsCode;
wstring Parameter::GetCppTypeCode() const
{
	if (g_isGenCsCode)
	{
		return wstring(FANSIToTCHAR(GetValueTypeCodeName(this)));
	}
	else
	{
		WCHAR temp[1024];
		StringCbPrintf(temp, 1024, TEXT("const RNT_%s::TYPE&"), m_DisplayName.c_str());
		return temp;
	}
}

bool	Parameter::GetNotesCodeString(list<TEE::RichText>& records)
{
	RichText record;
	record.content = GetCppTypeCode();
	record.eColor = TEE::RC_BLUE;
	records.push_back(record);

	record.Clear();
	record.content = _T(" ") + m_CodeName;
	record.eColor = TEE::RC_GREY;
	records.push_back(record);

	if (HasNextTypeNode(this, this->m_classType))
	{
		record.Clear();
		record.content = _T(",");
		records.push_back(record);
	}

	return true;
}

bool	Parameter::GetCPPCode( wstring& cppCode ) const
{
	wstring typeName = GetCppTypeCode();
	WCHAR temp[1024];
	ZeroMemory( temp, 1024 * sizeof(WCHAR) );
	StringCbPrintf(temp, 1024, _T("%s %s%s"), typeName.c_str(), m_CodeName.c_str(),
		(HasNextTypeNode(this, this->m_classType) ? _T(", ") : _T("")));

	cppCode += temp;

	return true;
}

//////////////////////////////////////////////////////////////////////////
// ContextParam

IMPLEMENT_BLOCK_ALLOCATE(ContextParam, 4)

ContextParam::ContextParam()
{
	m_classType = (NT_CONTEXTPARAM);
}

TiXmlElement*	ContextParam::ToXMLElement()
{
	TiXmlElement *pXmlElement = NodeBase::ToXMLElement();
	return pXmlElement;
}
bool	ContextParam::FromXMLElement( const TiXmlElement* pXMLElement )
{
	return NodeBase::FromXMLElement( pXMLElement );
}
bool	ContextParam::ToRichString( list<TEE::RichText>& records )
{
	RichText record;
	record.content = m_DisplayName + _T(" ") + m_CodeName;
	if (Next() != NULL)
	{
		record.content += _T(", ");
	}
	records.push_back( record );

	return true;
}

bool	ContextParam::GetNotesCodeString( list<TEE::RichText>& records )
{
	RichText record;
	record.content = _T("const RNT_") + m_DisplayName +_T("::TYPE&");
	record.eColor = TEE::RC_BLUE;
	records.push_back( record );

	record.Clear();
	record.content = _T(" ") + m_CodeName;
	record.eColor = TEE::RC_GREY;
	records.push_back( record );

	if (HasNextTypeNode(this, this->m_classType))
	{
		record.Clear();
		record.content = _T(",");
		records.push_back( record );
	}

	return true;
}

ErrorMask	ContextParam::CheckError()
{
	ClearErrorFlag(ERR_EVERYTHING);

	if( m_DisplayName.empty() || m_CodeName.empty() )
	{
		AddErrorFlag( ERR_INVALID_FORMAT );
	}
	NodeBase* pOption = GTEEMgr->FindOptionFromCode(m_DisplayName);
	if ( pOption == NULL )
	{
		AddErrorFlag( ERR_INVALID_FORMAT );
	}
	return GetErrorMask();
}
wstring	ContextParam::GetReadableString() const
{
	return CodeName();
}
bool	ContextParam::GetOptionCode( wstring& valueType )
{
	valueType = m_DisplayName;
	return true;
}

//////////////////////////////////////////////////////////////////////////
// ContextRoot

IMPLEMENT_BLOCK_ALLOCATE(ContextRoot, 1)

ContextRoot::ContextRoot()
{
	m_classType = (NT_CONTEXTROOT);
}

bool	ContextRoot::ToRichString( list<TEE::RichText>& records )
{
	RichText recordPre;
	recordPre.content = _T("context=(");
	records.push_back( recordPre );

	NodeBase::ToRichString(records);

	RichText recordPost;
	recordPost.content = _T(")");
	records.push_back( recordPost );

	return true;
}

bool	ContextRoot::GetNotesCodeString( list<TEE::RichText>& records )
{
	RichText recordPre;
	recordPre.content = _T("context=(");
	records.push_back( recordPre );

	NodeBase::GetNotesCodeString(records);

	RichText recordPost;
	recordPost.content = _T(")");
	records.push_back( recordPost );
	return true;
}

wstring	ContextRoot::GetReadableString() const
{
	wstring strRead = _T("(");
	NodeBase* pChild = FirstChild();
	for( ; pChild != NULL; pChild = pChild->Next() )
	{
		strRead += pChild->GetReadableString();
		if (pChild->Next() != NULL)
		{
			strRead += _T(",");
		}
	}
	strRead += _T(")");
	return strRead;
}

TiXmlElement*	ContextRoot::ToXMLElement()
{
	TiXmlElement *pXmlElement = NodeBase::ToXMLElement();
	return pXmlElement;
}
bool			ContextRoot::FromXMLElement( const TiXmlElement* pXMLElement )
{
	return NodeBase::FromXMLElement( pXMLElement );
}
