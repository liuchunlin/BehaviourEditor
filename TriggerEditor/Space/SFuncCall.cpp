#include "SFuncCall.h"
#include "NodeVerify.h"
#include "TEEManager.h"
#include "SpaceManager.h"
#include "SFunction.h"
#include "SVariable.h"

using namespace Space;

//-------------------------------------------------------------------------------------
//STFuncCall
IMPLEMENT_BLOCK_ALLOCATE(STFuncCall, 128)

STFuncCall::STFuncCall()
{
	ClassType(SNT_TFUNCCALL);
}

TiXmlElement*	STFuncCall::ToXMLElement()
{
	TiXmlElement *pXmlElement = SNode::ToXMLElement();

	return pXmlElement;
}

bool	STFuncCall::FromXMLElement( const TiXmlElement* pXMLElement )
{
	SNode::FromXMLElement( pXMLElement );

	TEE::NodeBase* pTmpNode = TEE::GTEEMgr->FindFunctionFromCode(Name());
	assert( pTmpNode != NULL );

	TEETmp( pTmpNode );

	return true;
}

bool	STFuncCall::PostXMLLoaded()
{
	Space::GSpaceMgr->VerifyParameters( this );
	return true;
}

bool	STFuncCall::GetContextView( ContextView& cnxt )
{
	cnxt.pNode = this;
	cnxt.bValid = GetErrorMask() == VS_OK;
	cnxt.bPreferences = true;
	cnxt.text = GetReadText();

	return true;
}

wstring	STFuncCall::GetReadText() const
{
	wstring text;
	if (IsStatementFunctionCall() && !TEETmp()->m_tag.empty())
	{
		text += TEETmp()->m_tag + _T(" - ") + SNode::GetReadText();
	}
	else
	{
		text += SNode::GetReadText();
	}
	if (text.empty())
		text = TEXT("Function");
	if (!IsStatementFunctionCall())
		text = _T("(") + text + _T(")");
	return text;
}

ErrorMask	STFuncCall::CheckError()
{
	Space::GSpaceMgr->CheckParametersValidate( this );
	return GetErrorMask();
}

bool	STFuncCall::Pasteable(const SNode* pSrc)
{
	if (pSrc == NULL)
		return false;

	if (IsActionType(pSrc->ClassType()))
		return true;

	return false;
}

bool	STFuncCall::GetRealParam( wstring& realParam ) const
{
	realParam += Name();
	realParam += _T("(");
	for ( SNode* pChild = FirstChild(); pChild != NULL; pChild = pChild->Next() )
	{
		wstring childParam;
		if( pChild->GetRealParam(childParam) )
		{
			realParam += childParam;
			break;
		}
	}
	realParam += _T(")");
	return true;
}

bool	STFuncCall::GetLuaCode( wstring& luaCode, int depth )
{
	if (!Working())	return false;

	if (Name() == _T("Comment"))
	{
		return false;
	}
	if (Name() == _T("DoNothing"))
	{
		return false;
	}

	if (IsStatementFunctionCall())
	{
		for (int k = 0; k < depth; ++k)
			luaCode += _T("\t");
	}

	if (Name() == _T("Arithmetic_REAL") || Name() == _T("Arithmetic_INT"))
	{
		luaCode += _T("(");

		for (SNode* pChild = FirstChild(); pChild != NULL; pChild = pChild->Next())
		{
			wstring childCode;
			if (pChild->GetLuaCode(childCode, depth + 1))
			{
				luaCode += childCode;
			}
		}

		luaCode += _T(")");
	}
	else if (Name() == _T("SkipRemainingActions"))
	{
		luaCode += _T("if 1 then OnSkipRemainingActions() return true end");
	}
	else if (Name() == _T("If_Then_Else"))
	{
		//condition
		luaCode += _T("if (");
		SNode* pChild = FirstChild();
		if (pChild == NULL)
			return false;

		wstring childCode;
		if (pChild->GetLuaCode(childCode, depth))
		{
			luaCode += childCode;
			luaCode += _T(") then\n");
		}

		//then
		pChild = pChild->Next();
		if (pChild == NULL)
			return false;
		childCode.clear();
		if (pChild->GetLuaCode(childCode, depth + 1))
		{
			luaCode += childCode;
		}

		//else
		luaCode += _T("\n");
		for (int k = 0; k < depth; ++k)
			luaCode += _T("\t");
		luaCode += _T("else\n");

		pChild = pChild->Next();
		if (pChild == NULL)
			return false;
		childCode.clear();
		if (pChild->GetLuaCode(childCode, depth + 1))
		{
			luaCode += childCode;
		}
		luaCode += _T("\n");
		//end
		for (int k = 0; k < depth; ++k)
			luaCode += _T("\t");
		luaCode += _T("end");
	}
	else if (Name() == _T("ForEachAction") || Name() == _T("ForEachAAction") || Name() == _T("ForEachBAction"))
	{
		wstring varCode, beginCode, endCode, actionCode;
		SNode* pVar = NULL;
		SNode* pBegin = NULL;
		SNode* pEnd = NULL;
		SNode* pAction = NULL;
		if (Name() == _T("ForEachAAction"))
		{
			varCode = _T("at_forLoopAIndex");
			pBegin = FirstChild();
			pEnd = pBegin->Next();
			pAction = pEnd->Next();
		}
		else if (Name() == _T("ForEachBAction"))
		{
			varCode = _T("at_forLoopBIndex");
			pBegin = FirstChild();
			pEnd = pBegin->Next();
			pAction = pEnd->Next();
		}
		else if (Name() == _T("ForEachAction"))
		{
			pVar = FirstChild();
			pBegin = pVar->Next();
			pEnd = pBegin->Next();
			pAction = pEnd->Next();
			if (pVar->GetLuaCode(varCode, 0))
			{
			}
		}

		if (varCode.empty() || pBegin == NULL || pEnd == NULL || pAction == NULL)
		{
			assert(false);
			return false;
		}

		if (pBegin->GetLuaCode(beginCode, 0))
		{
			luaCode += varCode + _T(" = ") + beginCode;
			luaCode += _T("\n");
		}
		if (pEnd->GetLuaCode(endCode, 0))
		{
			for (int k = 0; k < depth; ++k)
				luaCode += _T("\t");
			luaCode += _T("while ") + varCode + _T(" <= ") + endCode + _T(" do");
			luaCode += _T("\n");
		}
		if (pAction->GetLuaCode(actionCode, depth + 1))
		{
			luaCode += actionCode;
			luaCode += _T("\n");

			for (int k = 0; k < depth + 1; ++k)
				luaCode += _T("\t");
			luaCode += varCode + _T(" = ") + varCode + _T(" + 1");
			luaCode += _T("\n");
			for (int k = 0; k < depth; ++k)
				luaCode += _T("\t");
			luaCode += _T("end");
		}
	}
	else if (Name() == _T("AddNewEvent"))
	{
		//TODO:
		SNode* pFunction = FirstChild();
		SNode* pEvent = pFunction->Next();
		if (pFunction == NULL || pEvent == NULL)
			return false;

		SNode* pTriggerAnster = pFunction->Definition();
		//INode* pTriggerAnster = this->Ancestor( SNT_TRIGGER );
		DWORD TriggerId = 0;
		if (pTriggerAnster != NULL)
		{
			TriggerId = reinterpret_cast<DWORD>(pTriggerAnster->UserData("id"));
		}
		WCHAR temp[1024];
		StringCbPrintfW(temp, 1024, _T("TriggerRegister%sEvent( \"Trigger_%d\""), pEvent->Name().c_str(), TriggerId);
		luaCode += temp;

		for (Space::SNode* pRealParam = pEvent->FirstChild(); pRealParam != NULL; pRealParam = pRealParam->Next())
		{
			wstring realParam;
			if (pRealParam->GetRealParam(realParam))
			{
				StringCbPrintfW(temp, 1024, _T(", %s"), realParam.c_str());
				luaCode += temp;
			}
		}
		luaCode += _T(" );");

	}
	else
	{
		luaCode += Name();
		luaCode += _T("(");
		for (SNode* pChild = FirstChild(); pChild != NULL; pChild = pChild->Next())
		{
			wstring childCode;
			if (pChild->GetLuaCode(childCode, depth + 1))
			{
				luaCode += childCode;
				if (pChild->Next())
				{
					luaCode += _T(",");
				}
			}
		}
		luaCode += _T(")");
	}
	return true;
}

//-------------------------------------------------------------------------------------
//SSFuncCall
IMPLEMENT_BLOCK_ALLOCATE(SSFuncCall, 128)

SSFuncCall::SSFuncCall()
{
	ClassType(SNT_SFUNCCALL);
}

TiXmlElement*	SSFuncCall::ToXMLElement()
{
	return SNode::ToXMLElement();
}

bool	SSFuncCall::FromXMLElement(const TiXmlElement* pXMLElement)
{
	return SNode::FromXMLElement(pXMLElement);
}

bool	SSFuncCall::PostXMLLoaded()
{
	SNode* pNode = GSpaceMgr->FindFunctionFromName(Name());
	assert(pNode != NULL);
	Definition(pNode);

	Space::GSpaceMgr->VerifyParameters(this);
	return true;
}

TEE::NodeBase*	SSFuncCall::TEETmp() const
{
	SNode* function = Definition();
	return function ? function->TEETmp() : nullptr;
}

void	SSFuncCall::Definition(SNode* pImpl)
{
	SNode::Definition(pImpl);
	if (pImpl)
		Name(pImpl->Name());
}

bool	SSFuncCall::GetContextView(ContextView& cnxt)
{
	cnxt.pNode = this;
	cnxt.bValid = GetErrorMask() == VS_OK;
	cnxt.bPreferences = true;
	cnxt.text = GetReadText();
	return true;
}

void	SSFuncCall::GetContextViews(vector<ContextView>& cnxts)
{
	SFunction* function = dynamic_cast<SFunction*>(Definition());
	if (function == nullptr) return;

	static vector<SVariable*> sFormalParams(32);
	sFormalParams.clear();
	function->GetFormalParams(sFormalParams);

	size_t iFP = 0;
	for (SNode* pChild = FirstChild(); pChild; pChild = pChild->Next())
	{
		if (iFP < sFormalParams.size())
		{
			SVariable* pFP = sFormalParams[iFP++];
			ContextView cnxt1;
			cnxt1.text = pFP->Name() + TEXT("=");
			cnxts.push_back(cnxt1);
		}

		ContextView cnxt2;
		if (pChild->GetContextView(cnxt2))
		{
			cnxts.push_back(cnxt2);
		}
		pChild->GetChildViews(cnxts);

		if (pChild->Next())
		{
			ContextView cnxt3;
			cnxt3.text = TEXT(", ");
			cnxts.push_back(cnxt3);
		}
	}
}

wstring	SSFuncCall::GetReadText() const
{
	SFunction* pDef = dynamic_cast<SFunction*>(Definition());
	wstring text = IsStatementFunctionCall() ? SFuncTag + TEXT(" - ") : wxString();
	text += (pDef && !pDef->m_desc.empty()) ? pDef->m_desc : Name();
	text += TEXT(" ( ");
	for (SNode* pChild = FirstChild(); pChild; pChild = pChild->Next())
	{
		text += pChild->GetReadText();

		if (pChild->Next())
		{
			text += TEXT(", ");
		}
	}
	text += TEXT(" ) ");

	if (!IsStatementFunctionCall())
		text = _T("(") + text + _T(")");

	return text;
}

ErrorMask	SSFuncCall::CheckError()
{
	if (Name().empty())
	{
		AddErrorFlag(VS_REFERENCE_ERR);
	}
	Space::SNode* pDefinition = Definition();
	if (pDefinition == NULL || pDefinition->Parent() == NULL)
	{
		AddErrorFlag(VS_REFERENCE_ERR);
	}

	Space::GSpaceMgr->CheckParametersValidate(this);
	return GetErrorMask();
}

bool	SSFuncCall::Pasteable(const SNode* pSrc)
{
	if (pSrc == NULL)
		return false;

	if (IsActionType(pSrc->ClassType()))
		return true;

	return false;
}

bool	SSFuncCall::GetRealParam(wstring& realParam) const
{
	realParam += Name();
	realParam += _T("(");
	for (SNode* pChild = FirstChild(); pChild != NULL; pChild = pChild->Next())
	{
		wstring childParam;
		if (pChild->GetRealParam(childParam))
		{
			realParam += childParam;
			break;
		}
	}
	realParam += _T(")");
	return true;
}

bool	SSFuncCall::GetLuaCode(wstring& luaCode, int depth)
{
	if (!Working())	return false;

	if (IsStatementFunctionCall())
	{
		for (int k = 0; k < depth; ++k)
			luaCode += _T("\t");
	}

	luaCode += Name();
	luaCode += _T("(");
	for (SNode* pChild = FirstChild(); pChild != NULL; pChild = pChild->Next())
	{
		wstring childCode;
		if (pChild->GetLuaCode(childCode, depth + 1))
		{
			luaCode += childCode;
			if (pChild->Next())
			{
				luaCode += _T(",");
			}
		}
	}
	luaCode += _T(")");

	return true;
}
