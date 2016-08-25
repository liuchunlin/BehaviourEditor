#include "SFunction.h"
#include "SpaceManager.h"
#include "CmdManager.h"
#include "TEEManager.h"
#include "SVariable.h"
#include <regex>

using namespace Space;
extern SNode* GetEditingNode();

//-------------------------------------------------------------------------------------
//SFunction
IMPLEMENT_BLOCK_ALLOCATE(SFunction, 64)

SFunction::SFunction()
	: m_objlink(this)
{
	ClassType(SNT_FUNCTION);
}

SFunction::~SFunction()
{
}

TiXmlElement*	SFunction::ToXMLElement()
{
	TiXmlElement *pXmlElement = SNode::ToXMLElement();
	pXmlElement->SetAttribute("Desc", FTCHARToUTF8(m_desc.c_str()));
	return pXmlElement;
}
bool	SFunction::FromXMLElement(const TiXmlElement* pXMLElement)
{
	SNode::FromXMLElement(pXMLElement);
	XmlUtility::GetXMLElementAttribute(pXMLElement, "Desc", m_desc);
	return true;
}

wstring FunctionNameSeperator = TEXT("--");
wstring SFuncTag = TEXT("函数");

wstring	SFunction::GetReadText() const
{
	return Name() + FunctionNameSeperator + m_desc;
}

ErrorMask	SFunction::CheckError()
{
	if (!IsValidSymbolName(Name()))
	{
		AddErrorFlag(VS_NAME_FOMAT_ERR);
	}
	return GetErrorMask();
}

bool	SFunction::Pasteable( const SNode* pSrc )
{
	if ( pSrc == NULL )
		return false;
	return true;
}

TEE::NodeBase*	SFunction::TEETmp() const
{
	return const_cast<SFunction*>(this)->GetReturnTEE();
}

TEE::NodeBase* SFunction::GetReturnTEE()
{
	SNode* returnTypeNode = FindFirstChild(SVerifyNodeType(SNT_RETURNTYPE));
	if (returnTypeNode)
		return returnTypeNode->TEETmp();
	return nullptr;
}

void SFunction::GetFormalParams(vector<SVariable*>& outParams)
{
	SNode* paramRoot = FindFirstChild(SVerifyNodeType(SNT_PARAMROOT));
	if (paramRoot == nullptr) return;
	for (SNode* pParam = paramRoot->FirstChild(); pParam; pParam = pParam->Next())
	{
		if (pParam->IsA(SNT_VARIABLE))
			outParams.push_back(static_cast<SVariable*>(pParam));
	}
}

//-------------------------------------------------------------------------------------
//SParamRoot
IMPLEMENT_BLOCK_ALLOCATE(SParamRoot, 32)

SParamRoot::SParamRoot()
{
	ClassType(SNT_PARAMROOT);
}

TiXmlElement*	SParamRoot::ToXMLElement()
{
	return  SNode::ToXMLElement();
}
bool	SParamRoot::FromXMLElement(const TiXmlElement* pXMLElement)
{
	return SNode::FromXMLElement(pXMLElement);
}

bool	SParamRoot::Pasteable(const SNode* pSrc)
{
	if (pSrc && pSrc->IsA(SNT_VARIABLE))
		return true;
	return false;
}

//-------------------------------------------------------------------------------------
//SReturnType
IMPLEMENT_BLOCK_ALLOCATE(SReturnType, 32)
SReturnType::SReturnType()
{
	ClassType(SNT_RETURNTYPE);
}

SReturnType::~SReturnType()
{
}

TiXmlElement*	SReturnType::ToXMLElement()
{
	return SNode::ToXMLElement();
}
bool	SReturnType::FromXMLElement(const TiXmlElement* pXMLElement)
{
	SNode::FromXMLElement(pXMLElement);

	TEE::NodeBase* pTmpRootNode = TEE::GTEEMgr->FindOptionFromCode(Name());
	TEETmp(pTmpRootNode);
	return true;
}

void SReturnType::TEETmp(TEE::NodeBase* pTNode)
{
	SNode::TEETmp(pTNode);
	if (pTNode)
		Name(pTNode->CodeName());
}

wstring	SReturnType::GetReadText() const
{
	return TEXT("返回: ") + (TEETmp() ? TEETmp()->DisplayName() : Name());
}

ErrorMask	SReturnType::CheckError()
{
	if (TEETmp() == nullptr)
	{
		AddErrorFlag(VS_TEETEMP_ERR);
	}
	return GetErrorMask();
}

//-------------------------------------------------------------------------------------
//SReturn
IMPLEMENT_BLOCK_ALLOCATE(SReturn, 32)

SReturn::SReturn()
{
	ClassType(SNT_RETURN);
}

bool	SReturn::FromXMLElement(const TiXmlElement* pXMLElement)
{
	SNode::FromXMLElement(pXMLElement);
	TEE::NodeBase* pTEE = TEE::GTEEMgr->FindFunctionFromCode(TEXT("Return"));
	TEETmp(pTEE);
	return true;
}

bool	SReturn::GetContextView(ContextView& cnxt)
{
	cnxt.pNode = this;
	cnxt.bValid = GetErrorMask() == VS_OK;
	cnxt.bPreferences = true;
	cnxt.text = GetReadText();

	if (!cnxt.text.empty())
	{
		cnxt.text = _T("(") + cnxt.text + _T(")");
	}

	return true;
}

void	SReturn::GetContextViews(vector<ContextView>& cnxts)
{
	VerifyParameter(false);

	ContextView cnxt1;
	cnxt1.text = _T("return");
	cnxts.push_back(cnxt1);

	SNode* pChild = FirstChild();
	if (pChild)
	{
		ContextView cnxt2;
		if (pChild->GetContextView(cnxt2))
		{
			cnxts.push_back(cnxt2);
		}
		pChild->GetChildViews(cnxts);
	}
}

wstring	SReturn::GetReadText() const
{
	wstring text = _T("Return");
	if (SNode* pChild = FirstChild())
	{
		text += _T(" ( ") + pChild->GetReadText() + _T(" ) ");
	}
	return text;
}

void	SReturn::VerifyParameter(bool bCheckError)
{
	SFunction* pfunc = static_cast<SFunction*>(Ancestor(SNT_FUNCTION));
	//特殊处理刚创建出来的节点
	if (Parent() == nullptr)
	{
		pfunc = dynamic_cast<SFunction*>(GetEditingNode());
	}

	SNode* pSChild = FirstChild();

	wstring srcOptionCode = VOID_TEE;
	TEE::NodeBase* pSrcTEE = pSChild ? pSChild->TEETmp() : nullptr;
	if (pSrcTEE) pSrcTEE->GetOptionCode(srcOptionCode);

	wstring targetOptionCode = VOID_TEE;
	TEE::NodeBase* pTargetTEE = pfunc ? pfunc->GetReturnTEE() : nullptr;
	if (pTargetTEE) pTargetTEE->GetOptionCode(targetOptionCode);

	if (srcOptionCode != targetOptionCode)
	{
		if (bCheckError)
		{
			if (pSChild) pSChild->AddErrorFlag(VS_PARAM_TYPE_ERR);
			AddErrorFlag(VS_PARAM_TYPE_ERR);
		}
		else
		{
			if (pSChild) pSChild->Remove(true);
			pSChild = nullptr;

			if (SNode* pNewNode = GSpaceMgr->BuildParameter(pTargetTEE))
			{
				AddChild(pNewNode);
				pSChild = pNewNode;
			}
		}
	}

	if (!bCheckError && pSChild && pSChild->Next())
		pSChild->Next()->Remove(true);
}

ErrorMask	SReturn::CheckError()
{
	for (SNode* pChild = FirstChild(); pChild != NULL; pChild = pChild->Next())
	{
		if (pChild->ClassType() == SNT_PARAMETER)
		{
			AddErrorFlag(VS_PARAM_PENDING);
			break;
		}
	}
	VerifyParameter(true);
	return GetErrorMask();
}

bool	SReturn::Pasteable(const SNode* pSrc)
{
	if (pSrc && IsActionType(pSrc->ClassType()))
		return true;
	return false;
}

bool	SReturn::GetLuaCode(wstring& luaCode, int depth)
{
	for (int k = 0; k < depth; ++k)
		luaCode += _T("\t");
	luaCode += _T("do return ");

	if (SNode* pChild = FirstChild())
	{
		wstring childCode;
		if (pChild->GetLuaCode(childCode, 0))
		{
			luaCode += childCode;
		}
	}

	luaCode += _T(" end");

	return true;
}
