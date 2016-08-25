#include "Function.h"
#include "NodeFactory.h"
#include "TEEManager.h"
#include "NodeVerify.h"
using namespace TEE;

#include <strsafe.h>
//////////////////////////////////////////////////////////////////////////
//FunctionRoot
IMPLEMENT_BLOCK_ALLOCATE(FunctionRoot, 1)
FunctionRoot::FunctionRoot()
{
	m_classType = (NT_FUNCTIONROOT);
}

TiXmlElement*	FunctionRoot::ToXMLElement()
{
	TiXmlElement *pXmlElement = NodeBase::ToXMLElement();

	return pXmlElement;
}
bool			FunctionRoot::FromXMLElement( const TiXmlElement* pXMLElement )
{
	return NodeBase::FromXMLElement( pXMLElement );
}
bool	FunctionRoot::NewFunctionable()
{
	return true;
}
//////////////////////////////////////////////////////////////////////////
//Function
IMPLEMENT_BLOCK_ALLOCATE(Function, 64)
Function::Function()
	: m_eReturnDetail(RD_NORMAL)
	, m_canBeAction(false)
{
	m_classType = (NT_FUNCTION);
}
bool	Function::FromString( const wstring& content )
{
	RemoveChildren();
	GNodeFactory.BuildNormalChild( content, this );
	NodeBase::CheckErrorTree(this);

	return true;
}


TiXmlElement*	Function::ToXMLElement()
{
	TiXmlElement* pXmlElement = NodeBase::ToXMLElement();
	pXmlElement->SetAttribute("Return", FTCHARToUTF8(m_ReturnCode.c_str()));
	pXmlElement->SetAttribute("Detail", (int)m_eReturnDetail);
	pXmlElement->SetAttribute("CanBeAction", m_canBeAction ? 1 : 0);
	return pXmlElement;
}

bool			Function::FromXMLElement( const TiXmlElement* pXMLElement )
{
	GetXMLElementAttribute( pXMLElement, "Return",	m_ReturnCode );
	GetXMLAttributeInfo<int>( pXMLElement, "Detail", *(int*)(&m_eReturnDetail) );
	int canBeAction = 0;
	GetXMLAttributeInfo(pXMLElement, "CanBeAction", canBeAction);
	m_canBeAction = canBeAction == 0 ? false : true;

	return NodeBase::FromXMLElement( pXMLElement );
}

NodeBase* Function::DoClone() const
{
	Function* pClone = (Function*)NodeBase::DoClone();
	pClone->m_ReturnCode = m_ReturnCode;
	pClone->m_eReturnDetail = m_eReturnDetail;
	return pClone;
}

ErrorMask	Function::CheckError()
{
	ClearErrorFlag( ERR_EVERYTHING );

	if (!IsVoidFunction())
	{
		NodeBase* pOption = GTEEMgr->FindOptionFromCode(m_ReturnCode);
		if (pOption == NULL)
			AddErrorFlag(ERR_INVALID_FORMAT);
	}

	ErrorMask flag = GTEEMgr->CheckTagValidate( m_tag );
	AddErrorFlag( flag );

	flag = GTEEMgr->CheckParametersValidate( this );
	AddErrorFlag( flag );

	flag = GTEEMgr->CheckNameValidate( this );
	AddErrorFlag( flag );
	return GetErrorMask();
}

bool	Function::GetNotesViewString( list<TEE::RichText>& records )
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

bool	Function::NewFunctionable()
{
	return true;
}
bool	Function::Deleteable()
{
	return true;
}
bool	Function::Propertyable()
{
	return true;
}

bool	Function::GetOptionCode(wstring& valueType)
{
	if (IsVoidFunction())
		valueType = _T("ACTION");
	else
		valueType = m_ReturnCode;
	return true;
}

extern bool g_isGenCsCode;

wxString Function::GetReturnTypeCode() const
{
	if (IsVoidFunction())
		return VOID_TEE;

	NodeBase* pOption = GTEEMgr->FindOptionFromCode(m_ReturnCode);
	if (pOption == nullptr) return "??";

	wxString returnType;
	if (g_isGenCsCode)
	{
		wstring rawTypeName = FANSIToTCHAR(GetValueTypeCodeName(pOption));
		if (m_eReturnDetail == RD_ARRAY)
			returnType = wxString::Format(TEXT("List<%s>"), rawTypeName.c_str());
		else
			returnType = rawTypeName;
	}
	else
	{
		if (m_eReturnDetail == RD_ARRAY)
			returnType = wxString::Format(TEXT("vector<RNT_%s::TYPE>"), pOption->CodeName().c_str());
		else
			returnType = wxString::Format(TEXT("RNT_%s::TYPE"), pOption->CodeName().c_str());
	}
	return returnType;
}

bool	Function::GetNotesCodeString(list<TEE::RichText>& records)
{
	RichText record;
	record.content = _T("// ") + m_DisplayName + _T("\n");
	record.eColor = TEE::RC_BLACK;
	records.push_back(record);

	NodeBase* pOption = GTEEMgr->FindOptionFromCode(m_ReturnCode);
	record.Clear();
	record.content = GetReturnTypeCode().wc_str();
	record.eColor = (pOption == NULL && !IsVoidFunction()) ? TEE::RC_RED : TEE::RC_BLUE;
	records.push_back(record);

	record.Clear();
	record.content = _T(" r_") + m_CodeName + _T("(RanaState L") + ((FirstChild() != NULL && FirstChild()->ClassType() == NT_PARAMETER) || HasNextTypeNode(FirstChild(), NT_PARAMETER) ? _T(",") : _T(""));
	record.eColor = TEE::RC_BLACK;
	records.push_back(record);

	NodeBase::GetNotesCodeString(records);

	record.Clear();
	record.content = _T(" );");
	record.eColor = TEE::RC_BLACK;
	records.push_back(record);

	return true;
}

bool	Function::GetCPPCode( wstring& cppCode ) const
{
	WCHAR temp[2048];
	ZeroMemory( temp, sizeof(temp) );
	wstring returnTypeName = GetReturnTypeCode().wc_str();
	if (g_isGenCsCode)
	{
		StringCbPrintf( temp, 2048, _T("public static %s r_%s(IntPtr L%s"), 
			returnTypeName.c_str(),
			m_CodeName.c_str(), 
			( ( FirstChild()!=NULL && FirstChild()->ClassType() == NT_PARAMETER) || HasNextTypeNode(FirstChild(), NT_PARAMETER) ? _T(", ") : _T("") ) );
	}
	else
	{
		StringCbPrintf( temp, 2048, _T("extern %s r_%s(RanaState L%s"), 
			returnTypeName.c_str(),
			m_CodeName.c_str(), 
			( ( FirstChild()!=NULL && FirstChild()->ClassType() == NT_PARAMETER) || HasNextTypeNode(FirstChild(), NT_PARAMETER) ? _T(", ") : _T("") ) );
	}

	cppCode += temp;

	NodeBase* pChile = FirstChild();
	for( ; pChile != NULL; pChile = pChile->Next() )
	{
		pChile->GetCPPCode( cppCode );
	}
	cppCode += _T(")");

	return true;
}
