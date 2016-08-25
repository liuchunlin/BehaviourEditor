#include "NodeBase.h"
#include "NodeFactory.h"
#include "TEEManager.h"
#include "NodeVerify.h"
#include "Option.h"
#include "Function.h"

using namespace TEE;

bool g_isGenCsCode = true; // is generating c-sharp code.

ErrorMask DoCheckErrorTree(NodeBase* pSrc, bool bRecursively);

ErrorMask	NodeBase::CheckErrorTree(NodeBase* pSrc, bool bRecursively)
{
	if (pSrc == NULL) return ERR_EVERYTHING;

	DoCheckErrorTree(pSrc, bRecursively);
	for (NodeBase* pParent = pSrc->Parent(); pParent != NULL; pParent = pParent->Parent())
	{
		DoCheckErrorTree(pParent, false);
	}

	return pSrc->GetErrorMask();
}

ErrorMask DoCheckErrorTree(NodeBase* pSrc, bool bRecursively /*= true*/)
{
	ErrorMask nValid = ERR_OK;
	if (pSrc == NULL) return nValid;

	pSrc->ClearErrorFlag(ERR_EVERYTHING);

	NodeBase* pChild = pSrc->FirstChild();
	for (; pChild != NULL; pChild = pChild->Next())
	{
		if (bRecursively)
		{
			nValid |= DoCheckErrorTree(pChild, bRecursively);
		}
		else
		{
			nValid |= pChild->GetErrorMask();
		}
	}
	nValid |= pSrc->CheckError();
	pSrc->m_errorMask = (nValid);

	return nValid;
}

bool	NodeBase::HasPrevTypeNode(const NodeBase* pSrc, NODE_TYPE eType)
{
	if (pSrc == NULL)
		return false;

	NodeBase* pPrev = pSrc->Prev();
	while (pPrev != NULL)
	{
		if (pPrev->m_classType == eType)
		{
			return true;
		}
		pPrev = pPrev->Prev();
	}
	return false;
}

bool	NodeBase::HasNextTypeNode(const NodeBase* pSrc, NODE_TYPE eType)
{
	if (pSrc == NULL)
		return false;

	NodeBase* pNext = pSrc->Next();
	while (pNext != NULL)
	{
		if (pNext->m_classType == eType)
		{
			return true;
		}
		pNext = pNext->Next();
	}
	return false;
}

NodeBase::NodeBase()
:m_errorMask(ERR_OK) 
{
}
NodeBase::~NodeBase()
{
}

NodeBase*	NodeBase::Clone(const NodeBase* pSrc, NodeBase* pParent)
{
	if (pSrc == NULL)
		return NULL;

	NodeBase* pClone = pSrc->DoClone();
	if (pParent) pParent->AddChild(pClone);
	for (NodeBase* pChild = pSrc->FirstChild(); pChild != NULL; pChild = pChild->Next())
	{
		Clone(pChild, pClone);
	}

	return pClone;
}

NodeBase* NodeBase::DoClone() const
{
	NodeBase* pClone = NodeFactory::CreateNode(ClassType());
	pClone->m_DisplayName = m_DisplayName;
	pClone->m_CodeName = m_CodeName;
	pClone->m_tag = m_tag;
	pClone->m_classType = m_classType;
	pClone->m_errorMask = m_errorMask;
	return pClone;
}

TiXmlElement*	NodeBase::CreateNodeXml( NodeBase* pSrc, TiXmlElement* pXmlParent )
{
	if( pSrc == NULL )
		return NULL;

	TiXmlElement* pXmlNode = pSrc->ToXMLElement( );
	if ( pXmlParent )
	{
		pXmlParent->LinkEndChild( pXmlNode );
	}

	CreateNodeXml( pSrc->FirstChild(), pXmlNode );

	CreateNodeXml( pSrc->Next(), pXmlParent );

	return pXmlNode;
}

TiXmlElement*	NodeBase::ToXMLElement()
{
	TiXmlElement *pXmlElement = new TiXmlElement( "" );
	if( !m_DisplayName.empty() )
		pXmlElement->SetAttribute( "Name", FTCHARToUTF8(m_DisplayName.c_str()) );
	if( !m_CodeName.empty() )
		pXmlElement->SetAttribute( "Code", FTCHARToUTF8(m_CodeName.c_str()) );
	pXmlElement->SetAttribute( "State", (int)m_errorMask );
	if( !m_tag.empty() )
		pXmlElement->SetAttribute( "Tags", FTCHARToUTF8(m_tag.c_str()) );

	pXmlElement->SetValue( TypeName() );

	return pXmlElement;
}
bool	NodeBase::FromXMLElement( const TiXmlElement* pXMLElement )
{
	if( pXMLElement == NULL )
		return false;

	if( pXMLElement != NULL )
	{
		GetXMLElementAttribute( pXMLElement,	"Name", m_DisplayName );
		GetXMLElementAttribute( pXMLElement,	"Code", m_CodeName );
		GetXMLElementAttribute( pXMLElement,	"Tags", m_tag );
#if _DEBUG & 0
		GetXMLAttributeInfo<int>( pXMLElement,	"State", *(int*)(&m_errorMask) );
#endif
	}

	if( pXMLElement->FirstChild() )
	{
		const TiXmlElement	*pXMLChild = pXMLElement->FirstChild()->ToElement();
		for( ; pXMLChild; pXMLChild = pXMLChild->NextSiblingElement() )
		{
			wstring clsName = FUTF8ToTCHAR(pXMLChild->Value());
			TEE::NodeBase* pNode = TEE::GNodeFactory.CreateNode( NodeFactory::NodeNameToType(clsName) );
			AddChild( pNode );
			pNode->FromXMLElement( pXMLChild );
		}
	}

	TEE::GTEEMgr->RegisterLoadedNode( this );

	return true;
}

bool	NodeBase::FromString( const wstring& content )
{
	return true;
}

bool	NodeBase::ToRichString( list<TEE::RichText>& records )
{
	NodeBase* pChild = FirstChild();
	for( ; pChild != NULL; pChild = pChild->Next() )
	{
		pChild->ToRichString( records );
	}
	return true;
}
bool	NodeBase::GetNotesViewString( list<TEE::RichText>& records )
{
	NodeBase* pChild = FirstChild();
	for( ; pChild != NULL; pChild = pChild->Next() )
	{
		pChild->GetNotesViewString( records );
	}
	return true;
}
bool	NodeBase::GetNotesCodeString( list<TEE::RichText>& records )
{
	NodeBase* pChild = FirstChild();
	for( ; pChild != NULL; pChild = pChild->Next() )
	{
		pChild->GetNotesCodeString( records );
	}
	return true;
}

wstring	NodeBase::GetReadableString() const
{
	return m_DisplayName;
}
wstring	NodeBase::DisplayName( bool bWithTag /*= false*/ ) const
{
	if (this == NULL)
	{
		return _T("???");
	}
	if( !bWithTag )
		return m_DisplayName;

	wstring name = m_tag;
	if( name.empty() )
		name += _T(" "); 
	else
		name += _T(" - "); 
	name += m_DisplayName;

	return name;
}

NodeBase*	NodeBase::Ancestor( NODE_TYPE eType )	const
{
	NodeBase* pAncestor = Parent();
	while ( pAncestor != NULL && pAncestor->ClassType() != eType )
	{
		pAncestor = pAncestor->Parent();
	}
	return pAncestor;
}
int	NodeBase::ChildCount( NODE_TYPE eType ) const
{
	int iCount = 0;
	for( NodeBase* pChild = FirstChild(); pChild != NULL; pChild = pChild->Next() )
	{
		if( NT_NULL == eType || pChild->ClassType() == eType )
		{
			++iCount;
		}
	}
	return iCount;
}

bool NodeBase::IsVoidFunction() const
{
	if (!IsA(NT_FUNCTION)) return false;
	const Function* function = static_cast<const Function*>(this);
	return function->m_ReturnCode.empty() || function->m_ReturnCode == VOID_TEE;
}

ErrorMask	NodeBase::GetErrorMask() const
{
	return m_errorMask;
}

ErrorMask NodeBase::CheckError()
{
	return GetErrorMask();
}

void	NodeBase::AddErrorFlag( ErrorMask val ) 
{ 
	m_errorMask |= val; 
}
void	NodeBase::ClearErrorFlag( ErrorMask val ) 
{ 
	m_errorMask &= ~val; 
}
bool	NodeBase::GetOptionCode( wstring& valueType )
{
	assert( false );
	return false;
}
bool	NodeBase::GetCPPCode( wstring& cppCode ) const
{
	assert( false );
	return false;
}

bool	NodeBase::NewEventable()
{
	return false;
}
bool	NodeBase::NewConditionable()
{
	return false;
}
bool	NodeBase::NewFunctionable()
{
	return false;
}
bool	NodeBase::NewOptionable()
{
	return false;
}
bool	NodeBase::NewSpaceable()
{
	return false;
}
bool	NodeBase::Deleteable()
{
	return false;
}
bool	NodeBase::Propertyable()
{
	return false;
}
bool	NodeBase::Editable()
{
	return true;
}
bool	NodeBase::Dragable()
{
	return false;
}
bool	NodeBase::Includeable()
{
	return false;
}

std::string NodeBase::TypeName()
{
	return (ANSICHAR*)FTCHARToANSI(NodeFactory::NodeTypeToName(ClassType()).c_str());
}

//////////////////////////////////////////////////////////////////////////
//RootNode
IMPLEMENT_BLOCK_ALLOCATE(RootNode, 4)
RootNode::RootNode()
{
	m_classType = (NT_ROOT);
}

TiXmlElement*	RootNode::ToXMLElement()
{
	TiXmlElement *pXmlElement = NodeBase::ToXMLElement();

	return pXmlElement;
}
bool			RootNode::FromXMLElement( const TiXmlElement* pXMLElement )
{
	NodeBase::FromXMLElement( pXMLElement );
	return true;
}

bool			RootNode::Save()
{
	return GTEEMgr->SaveNode(m_fileName, this);
}

bool	RootNode::NewEventable()
{
	return true;
}
bool	RootNode::NewConditionable()
{
	return true;
}
bool	RootNode::NewFunctionable()
{
	return true;
}
bool	RootNode::NewOptionable()
{
	return true;
}
bool	RootNode::NewSpaceable()
{
	return true;
}

bool	RootNode::Propertyable()
{
	return true;
}

//Others
//////////////////////////////////////////////////////////////////////////
namespace TEE
{
	const map<TEE::VALUE_TYPE, pair<wstring,wstring> >&	GetValueTypeNames()
	{
		static map<VALUE_TYPE, pair<wstring,wstring> >		s_ValueMaps;
		if(s_ValueMaps.empty())
		{
			//StringName, CodeName
			s_ValueMaps.insert( make_pair( VT_NULL,			make_pair( _T("无类型"), _T("NULL") ) ) );
			s_ValueMaps.insert( make_pair( VT_INT,			make_pair( _T("数值型"), _T("INT") ) ) );
			s_ValueMaps.insert( make_pair( VT_REAL,			make_pair( _T("真值型"), _T("REAL") ) ) );
			s_ValueMaps.insert( make_pair( VT_BOOLEAN,		make_pair( _T("布尔型"), _T("BOOL") ) ) );
			s_ValueMaps.insert( make_pair( VT_STRING,		make_pair( _T("字符串"), _T("STRING") ) ) );
			s_ValueMaps.insert( make_pair( VT_CUSTOM,		make_pair( _T("自定义"), _T("CUSTOM") ) ) );
		}

		return s_ValueMaps;
	}

	TEE::VALUE_TYPE	GetValueTypeEnum( const wstring& codeType )
	{
		const map<VALUE_TYPE, pair<wstring,wstring> >&		s_ValueMaps = GetValueTypeNames();
		map<VALUE_TYPE, pair<wstring,wstring> >::const_iterator cIter(s_ValueMaps.begin());
		for( ; cIter != s_ValueMaps.end(); ++cIter )
		{
			const pair<wstring,wstring>& value = (*cIter).second;
			if( value.second == codeType )
			{
				return (*cIter).first;
			}
		}
		//assert(false);
		return VT_NULL;
	}

	const pair<wstring,wstring>&	GetValueTypeName( VALUE_TYPE eType )
	{
		const map<VALUE_TYPE, pair<wstring,wstring> >&		s_ValueMaps = GetValueTypeNames();
		assert( s_ValueMaps.find(eType) != s_ValueMaps.end() );
		if( s_ValueMaps.find(eType) == s_ValueMaps.end() )
		{
			return (*s_ValueMaps.find(VT_NULL)).second;
		}

		return (*s_ValueMaps.find(eType)).second;
	}
	const map<RETURN_DETAIL, wstring>&	GetReturnDetailNames()
	{
		static map<RETURN_DETAIL, wstring>		s_DetailMaps;
		if( s_DetailMaps.empty() )
		{
			s_DetailMaps.insert( make_pair(RD_NORMAL,		_T("一维值") ) );
			s_DetailMaps.insert( make_pair(RD_ARRAY,		_T("多维值") ) );
			s_DetailMaps.insert( make_pair(RD_CUSTOM,		_T("自定义") ) );
		}
		return s_DetailMaps;
	}
	const wstring&	GetReturnDetailName( RETURN_DETAIL eType )
	{
		const map<RETURN_DETAIL, wstring>&		s_DetailMaps = GetReturnDetailNames();
		assert( s_DetailMaps.find(eType) != s_DetailMaps.end() );

		return (*s_DetailMaps.find(eType)).second;
	}

	const char* GetValueTypeCodeName(VALUE_TYPE eValueType, const Option* pOption)
	{
		switch(eValueType)
		{
		case VT_BOOLEAN:
			return "bool";
		case VT_INT:
			return "int";
		case VT_REAL:
			return "double";
		case VT_STRING:
			return "string";
		case VT_NULL:
			return "void";
		case VT_CUSTOM:
			if (pOption)
				return FTCHARToANSI(pOption->CodeName().c_str());
			else
				return "UnkownCustomType";
		default:
			assert( false );
			return "";
		}
	}

	const char* GetValueTypeCodeName(const NodeBase* pNode)
	{
		if (pNode && pNode->ClassType() == NT_OPTION)
		{
			const Option* pOption = static_cast<const Option*>(pNode);
			return GetValueTypeCodeName(pOption->m_eValueType, pOption);
		}
		else if (pNode && (pNode->ClassType() == NT_PARAMETER || pNode->ClassType() == NT_CONTEXTPARAM))
		{
			Option* pOption = (Option*)GTEEMgr->FindOptionFromCode(pNode->DisplayName());
			if (pOption)
			{
				return GetValueTypeCodeName(pOption->m_eValueType, pOption);
			}
		}
		return "";
	}

	bool	IsInternalType( const wstring& codeName )
	{
		static const wstring InternalPrefix = _T("_internal_");
		if(    codeName == _T("MultiIF") 
			|| codeName == _T("ForEachActions")  
			|| codeName == _T("ForEachAction")  
			|| codeName == _T("ForEachAActions")  
			|| codeName == _T("ForEachAAction")  
			|| codeName == _T("ForEachBActions")  
			|| codeName == _T("ForEachBAction")  
			|| codeName == _T("SetVariable")  
			|| codeName == _T("Comment")  
			|| codeName == _T("If_Then_Else") 
			|| codeName == _T("AddNewEvent") 
			|| codeName == _T("Arithmetic_INT")
			|| codeName == _T("Arithmetic_REAL")
			|| codeName == _T("SkipRemainingActions")
			|| codeName == _T("StartTimer")
			|| codeName == _T("DoNothing")
			|| codeName == _T("While")
			|| codeName == _T("Return")
			|| codeName.substr(0, InternalPrefix.size()) == InternalPrefix
			)
			return true;
		return false;
	}
	bool	NoGenerateCode( const wstring& type )
	{
		if (type == _T("Wait"))
			return true;
		return false;
	}
	bool	IsInternalEvent( const wstring& type )
	{
		if( type == _T("Timer") 
			|| type == _T("PeriodsTimer")  
			)
			return true;
		return false;
	}

}
