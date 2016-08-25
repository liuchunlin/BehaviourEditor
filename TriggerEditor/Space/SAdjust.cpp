#include "SAdjust.h"
#include "NodeVerify.h"
#include "TEEManager.h"
#include "Option.h"

using namespace Space;
extern void Log(const wchar_t* pszFormat, ...);

//////////////////////////////////////////////////////////////////////////
//SAdjust
IMPLEMENT_BLOCK_ALLOCATE(SAdjust, 128)
SAdjust::SAdjust()
{
	ClassType(SNT_ADJUST);
}
TiXmlElement*	SAdjust::ToXMLElement()
{
	TiXmlElement *pXmlElement = SNode::ToXMLElement();

	if( TEETmp() != NULL && TEETmp()->Parent() != NULL )
	{
		pXmlElement->SetAttribute( "Op", FTCHARToUTF8(TEETmp()->Parent()->CodeName().c_str()) );
	}

	return pXmlElement;
}
bool	SAdjust::FromXMLElement( const TiXmlElement* pXMLElement )
{
	SNode::FromXMLElement( pXMLElement );
	XmlUtility::GetXMLElementAttribute( pXMLElement, "Op", m_optionName );

	TEE::NodeBase* pOptionNode = TEE::GTEEMgr->FindOptionFromCode(m_optionName);
	if (pOptionNode)
	{
		TEE::NodeBase* pAdjustNode = pOptionNode->FindFirstChild( TEE::VerifyDisplayName(Name()) );
		TEETmp( pAdjustNode );
	}
	else
	{
		Log( TEXT("未知的可选项 %s"), m_optionName.c_str() );
	}
	return true;
}

void SAdjust::CopyFrom(const SNode* src)
{
	SNode::CopyFrom(src);
	const SAdjust* srcNode = dynamic_cast<const SAdjust*>(src);
	m_optionName = srcNode->m_optionName;
}

void SAdjust::TEETmp(TEE::NodeBase* pTNode)
{
	SNode::TEETmp(pTNode);
	if (pTNode)
	{
		if (pTNode->ClassType() == TEE::NT_ADJUST && pTNode->Parent())
			m_optionName = pTNode->Parent()->CodeName();
		else if (pTNode->ClassType() == TEE::NT_OPTION)
			m_optionName = pTNode->CodeName();
	}
}

void SAdjust::FixTEETmp()
{
	SNode::FixTEETmp();
	
	TEE::NodeBase* pOptionNode = TEE::GTEEMgr->FindOptionFromCode(m_optionName);
	if (pOptionNode)
	{
		TEE::NodeBase* pAdjustNode = pOptionNode->FindFirstChild(TEE::VerifyDisplayName(Name()));
		if (pAdjustNode)
			TEETmp(pAdjustNode);
		else
			TEETmp(pOptionNode);
	}
	else
	{
		TEETmp(nullptr);
		Log(TEXT("未知的可选项 %s"), m_optionName.c_str());
	}
}

bool	SAdjust::GetContextView( ContextView& cnxt )
{
	cnxt.pNode = this;
	cnxt.bValid = GetErrorMask() == VS_OK;
	cnxt.bPreferences = true;
	cnxt.text = GetReadText();

	return true;
}
wstring	SAdjust::GetReadText() const
{
	wstring text;
	if( TEETmp() == NULL )
	{
		text = _T("??");
		return text;
	}

	text += TEETmp()->GetReadableString();
	return text;
}
ErrorMask	SAdjust::CheckError()
{
	assert( GetErrorMask() == VS_OK );

	if( TEETmp() == NULL || TEETmp()->ClassType() != TEE::NT_ADJUST )
	{
		AddErrorFlag( VS_REFERENCE_ERR );

		TEE::NodeBase* pTmpRootNode = TEE::GTEEMgr->FindOptionFromName(Name());
		assert( pTmpRootNode != NULL );
		TEETmp( pTmpRootNode );
	}

	return GetErrorMask();
}
bool	SAdjust::GetLuaCode( wstring& luaCode, int depth )
{
	return GetRealParam( luaCode );
}
bool	SAdjust::GetRealParam( wstring& realParam ) const
{
	if( TEETmp() != NULL && TEETmp()->Parent()->IsA(TEE::NT_OPTION) )
	{
		TEE::Option* pOption = static_cast<TEE::Option*>(TEETmp()->Parent());
		switch(pOption->m_eValueType)
		{
		case TEE::VT_STRING:
			realParam += _T("\"");
			realParam += TEETmp()->CodeName();
			realParam += _T("\"");
			break;
		default:
			realParam = TEETmp()->CodeName();
			break;
		}
	}
	return true;
}
