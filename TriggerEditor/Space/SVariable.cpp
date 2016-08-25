#include "SVariable.h"
#include "NodeVerify.h"
#include "TEEManager.h"
#include "SNodeVerify.h"
#include "SpaceManager.h"
#include "CmdManager.h"

using namespace Space;
extern void Log(const wchar_t* pszFormat, ...);

wstring GetVarName(SNode* pNode)
{
	if (IsGlobalVariable(pNode) || pNode->IsA(SNT_GLOBAL_VARREF))
		return GLOBAL_VAR_PREFIX + pNode->Name();
	else if (IsLocalVariable(pNode) || pNode->IsA(SNT_LOCAL_VARREF))
		return LOCAL_VAR_PREFIX + pNode->Name();
	else if (IsParamVariable(pNode) || pNode->IsA(SNT_PARAM_VARREF))
		return PARAM_VAR_PREFIX + pNode->Name();
	else if (pNode->IsA(SNT_CONTEXT_VARREF))
		return PARAM_VAR_PREFIX + pNode->Name();
	else
		return pNode->Name();
}

//////////////////////////////////////////////////////////////////////////
//SVarSet
IMPLEMENT_BLOCK_ALLOCATE(SVarSet, 1)
SVarSet::SVarSet()
{
	ClassType(SNT_VARSET);
}
TiXmlElement*	SVarSet::ToXMLElement()
{
	TiXmlElement *pXmlElement = SNode::ToXMLElement();

	return pXmlElement;
}
bool	SVarSet::FromXMLElement( const TiXmlElement* pXMLElement )
{
	SNode::FromXMLElement( pXMLElement );

	return true;
}

bool	SVarSet::GetContextView( ContextView& cnxt )
{
	cnxt.pNode = this;
	cnxt.bValid = GetErrorMask() == VS_OK;
	cnxt.bPreferences = true;
	cnxt.text = GetReadText();

	return true;
}

wstring	SVarSet::GetReadText() const
{
	wstring text;
	text += Name();
	return text;
}
ErrorMask	SVarSet::CheckError()
{
	assert( GetErrorMask() == VS_OK );

	return GetErrorMask();
}
//////////////////////////////////////////////////////////////////////////
//SVarItem
IMPLEMENT_BLOCK_ALLOCATE(SVariable, 32)
SVariable::SVariable()
{
	ClassType(SNT_VARIABLE);
}
SVariable::~SVariable()
{
	GCmdMgr->RemoveVarItemName( SNode::Name() );
}
void	SVariable::Name( const wstring& val )
{
	GCmdMgr->RemoveVarItemName( SNode::Name() );
	SNode::Name(val);
	GCmdMgr->AddVarItemName( SNode::Name() );
}

TiXmlElement*	SVariable::ToXMLElement()
{
	TiXmlElement *pXmlElement = SNode::ToXMLElement();

	assert( TEETmp() != NULL );
	if( TEETmp() != NULL )
	{
		pXmlElement->SetAttribute( "type", FTCHARToUTF8(TEETmp()->CodeName().c_str()) );
	}
	//wstring defaultVal = StringData("default");
	//if( !defaultVal.empty() )
	//{
	//	pXmlElement->SetAttribute( "default", FTCHARToUTF8(defaultVal.c_str()) );
	//}

	bool bAry = IsArray();
	if( bAry )
	{
		pXmlElement->SetAttribute( "bAry", bAry );
		int iSize = reinterpret_cast<int>(UserData("size"));
		pXmlElement->SetAttribute( "size", iSize );
	}

	return pXmlElement;
}
bool	SVariable::FromXMLElement( const TiXmlElement* pXMLElement )
{
	SNode::FromXMLElement( pXMLElement );

	wstring typeName;
	XmlUtility::GetXMLElementAttribute( pXMLElement, "type", typeName );
	assert( !typeName.empty() );

	wstring strbAry;
	XmlUtility::GetXMLElementAttribute( pXMLElement, "bAry", strbAry );
	if( strbAry == _T("1") )
	{
		IsArray(_wtoi(strbAry.c_str()) != 0);
		wstring strSize;
		XmlUtility::GetXMLElementAttribute( pXMLElement, "size", strSize );
		UserData( "size", reinterpret_cast<void*>(_wtoi(strSize.c_str()) ) );
	}

	//TEEtmp
	TEE::NodeBase* pTmpRootNode = TEE::GTEEMgr->FindOptionFromCode(typeName);
	assert( pTmpRootNode != NULL );
	TEETmp( pTmpRootNode );
	return true;
}

bool	SVariable::GetContextView( ContextView& cnxt )
{
	cnxt.pNode = this;
	cnxt.bValid = GetErrorMask() == VS_OK;
	cnxt.bPreferences = true;
	cnxt.text = GetReadText();

	return true;
}

wstring	SVariable::GetReadText() const
{
	TEE::NodeBase* pTmp = TEETmp();
	bool bAry = IsArray();
	int iSize = reinterpret_cast<int>(UserData("size"));

	wstring text;
	if (Parent() && Parent()->IsA(SNT_ACTIONROOT))
		text += TEXT("local ");
	text += pTmp ? pTmp->DisplayName() : TEXT("???");
	text += TEXT(" ");
	text += SNode::Name();
	if (bAry)
	{
		wchar_t buff[128];
		_itow_s(iSize, buff, 10);
		text += TEXT("[");
		text += buff;
		text += TEXT("]");
	}

	if( FirstChild() && FirstChild()->ClassType() != Space::SNT_PARAMETER )
	{
		wstring defaultValue;
		if (FirstChild()->GetRealParam( defaultValue ))
		{
			text += TEXT(" = ");
			text += defaultValue;
		}
	}
	return text;
}

ErrorMask	SVariable::CheckError()
{
	if (Parent() && Parent()->IsA(SNT_ACTIONROOT))
	{
		vector<SNode*> sameNameVars;
		Parent()->FindChildren(SNodeTypeAndNamePredicate(SVerifyNodeType(SNT_VARIABLE), SVerifyNameType(Name())), sameNameVars, false);
		if (sameNameVars.size() > 1)
			AddErrorFlag(VS_NAME_DUP_ERR);
	}
	return GetErrorMask();
}

bool SVariable::GetLuaCode( wstring& luaCode, int depth )
{
	bool bGlobal = IsGlobalVariable(this);
	wstring VarName = GetVarName(this);

	wstring defaultValue;
	if( FirstChild() && FirstChild()->ClassType() != Space::SNT_PARAMETER )
	{
		FirstChild()->GetRealParam( defaultValue );
	}
	else if( TEETmp() != NULL && TEETmp()->CodeName() == _T("TIMER") )
	{
		defaultValue = TEXT("RanaCreateTimer();");
	}
	else
	{
		defaultValue = TEXT("0");
	}
	bool bAry = IsArray();
	if( bAry )
	{
		wchar_t buffer[512];
		int iSize = reinterpret_cast<int>(UserData("size"));
		swprintf_s(buffer, 512, TEXT("\t%s%s = {}\n"), bGlobal?TEXT(""):TEXT("local "), VarName.c_str());	luaCode += buffer;
		swprintf_s(buffer, 512, TEXT("\tfor _i=1,%d do\n"), iSize);											luaCode += buffer;
		swprintf_s(buffer, 512, TEXT("\t\t%s[_i] = %s\n"), VarName.c_str(), defaultValue.c_str());			luaCode += buffer;
		swprintf_s(buffer, 512, TEXT("\tend"));															luaCode += buffer;
	}
	else
	{
		wchar_t buffer[512];
		swprintf_s(buffer, 512, TEXT("\t%s%s = %s"), bGlobal?TEXT(""):TEXT("local "), VarName.c_str(), defaultValue.c_str()); luaCode += buffer;
	}
	return true;
}

bool	SVariable::Pasteable(const SNode* pastingNode)
{
	if (pastingNode && pastingNode->IsA(SNT_VARIABLE))
		return true;
	return false;
}

wstring SVariable::GetDefaultValue()
{
	SNode* pDefaultNode = FirstChild();
	if (pDefaultNode == nullptr || pDefaultNode->IsA(SNT_PARAMETER))
		return wstring();
	return pDefaultNode->Name();
}

//////////////////////////////////////////////////////////////////////////
//SVariable
IMPLEMENT_BLOCK_ALLOCATE(SVarRef, 32)
SVarRef::SVarRef()
{
}
TiXmlElement*	SVarRef::ToXMLElement()
{
	TiXmlElement *pXmlElement = SNode::ToXMLElement();
	pXmlElement->SetAttribute( "work", Working() );

	if( TEETmp() != NULL )
	{
		pXmlElement->SetAttribute( "Op", FTCHARToUTF8(TEETmp()->CodeName().c_str()) );
	}

	if( bool bAry = IsArray() )
	{
		pXmlElement->SetAttribute( "bAry", bAry );
	}
	if( bool bTgr = UserData("bTgr") != nullptr )
	{
		pXmlElement->SetAttribute( "bTgr", bTgr );
	}

	return pXmlElement;
}
bool	SVarRef::FromXMLElement( const TiXmlElement* pXMLElement )
{
	SNode::FromXMLElement( pXMLElement );

	int bWork=1;
	pXMLElement->Attribute( "work", &bWork );
	Working( bWork != 0 );

	wstring opValue;
	XmlUtility::GetXMLElementAttribute( pXMLElement, "Op", opValue );
	TEE::NodeBase* pOptionNode = TEE::GTEEMgr->FindOptionFromCode(opValue);
	if (pOptionNode == NULL)
	{
		Log( TEXT("未知的可选项 %s"), opValue.c_str() );
	}
	TEETmp( pOptionNode );

	wstring strbAry;
	XmlUtility::GetXMLElementAttribute( pXMLElement, "bAry", strbAry );
	if( strbAry == _T("1") )
	{
		IsArray(_wtoi(strbAry.c_str()) != 0);
	}
	wstring strbTgr;
	XmlUtility::GetXMLElementAttribute( pXMLElement, "bTgr", strbTgr );
	if( strbTgr == _T("1") )
	{
		UserData( "bTgr", reinterpret_cast<void*>(_wtoi(strbTgr.c_str()) ) );
	}

	return true;
}

bool	SVarRef::GetContextView( ContextView& cnxt )
{
	cnxt.pNode = this;
	cnxt.bValid = Name() != _T("");
	cnxt.bPreferences = true;
	cnxt.bEnable = true;
	cnxt.text.clear();

	Space::SNode* pNodeNode = Definition();
	if( Name().empty() && TEETmp() != NULL )
	{
		cnxt.text += TEETmp()->DisplayName();
	}
	else if( pNodeNode != NULL )
	{
		cnxt.text += pNodeNode->Name();
	}

	if( cnxt.text == _T("") )
	{
		cnxt.text = Name();
		cnxt.bValid = false;
	}

	return true;
}
void	SVarRef::GetChildViews( vector<ContextView>& cnxts )
{
	bool bAry = IsArray();
	if( !bAry )
		return;

	for( Space::SNode* pChild = FirstChild(); pChild != NULL; pChild = pChild->Next() )
	{
		ContextView cnxt;
		cnxt.bPreferences = false;
		cnxt.text = _T("[");
		cnxts.push_back( cnxt );

		{
			cnxt.pNode = pChild;
			cnxt.bValid = pChild->GetErrorMask() == VS_OK && pChild->ClassType() != Space::SNT_PARAMETER;
			cnxt.bPreferences = true;
			Space::SNode* pVarItem = pChild->Definition();
			if( pChild->Name().empty() && pChild->TEETmp() != NULL )
			{
				cnxt.text = pChild->TEETmp()->DisplayName();
			}
			else if( pVarItem != NULL )
			{
				cnxt.text = pVarItem->Name();
			}
			else
			{
				cnxt.text = pChild->GetReadText();
			}
			cnxts.push_back( cnxt );
			pChild->GetChildViews(cnxts);
		}

		cnxt.pNode = NULL;
		cnxt.bValid = true;
		cnxt.bPreferences = false;
		cnxt.text = _T("]");
		cnxts.push_back( cnxt );
	}
}

void	SVarRef::GetContextViews( vector<ContextView>& cnxts )
{
	ContextView cnxt;
	GetContextView( cnxt );
	cnxts.push_back( cnxt );
}
wstring	SVarRef::GetReadText() const
{
	wstring text;

	Space::SNode* pVarItem = Definition();
	if( Name().empty() && TEETmp() != NULL )
	{
		text += TEETmp()->DisplayName();
	}
	else if( pVarItem != NULL )
	{
		text += pVarItem->Name();
	}

	if( pVarItem != NULL )
	{
		bool bAry = IsArray();
		if( bAry )
		{
			for( Space::SNode* pChild = FirstChild(); pChild != NULL; pChild = pChild->Next() )
			{
				text += _T("[");
				text += pChild->GetReadText();
				text += _T("]");
			}
		}
	}

	if( text.empty() )
	{
		text = Name();
	}

	return text;
}

ErrorMask	SVarRef::CheckError()
{
	assert( GetErrorMask() == VS_OK );
	TEE::NodeBase* pTEETmp = TEETmp();
	if( pTEETmp == NULL )
	{
		AddErrorFlag( VS_TEETEMP_ERR );
	}
	if( Name().empty() )
	{
		AddErrorFlag( VS_REFERENCE_ERR );
	}
	Space::SNode* pDefinition = Definition();
	if( pDefinition == NULL || pDefinition->Parent() == NULL )
	{
		AddErrorFlag( VS_REFERENCE_ERR );
	}
	else if( pDefinition->ClassType() != Space::SNT_TRIGGER )
	{
		if( pDefinition->TEETmp() != TEETmp() )
			AddErrorFlag(VS_TEETEMP_ERR);
	}

	return GetErrorMask();
}
void	SVarRef::DominoOffect()
{
	if( Parent() == NULL )
		return;
	if( Parent()->Name() != _T("SetVariable") )
		return;

	if( Next() == NULL )
		return;

	if( Next()->TEETmp() == NULL )
	{
		Next()->TEETmp( TEETmp() );
	}
	if( Next()->TEETmp() != NULL )
	{
		bool bEqualType = false;
		switch(Next()->TEETmp()->ClassType())
		{
		case TEE::NT_ADJUST:
			bEqualType = TEETmp()->CodeName() == Next()->TEETmp()->Parent()->CodeName();
			break;
		case TEE::NT_FUNCTION:
			bEqualType = TEETmp()->CodeName() == (static_cast<TEE::Function*>(Next()->TEETmp()))->m_ReturnCode;
			break;
		case TEE::NT_OPTION:
			bEqualType = TEETmp()->CodeName() == Next()->TEETmp()->CodeName();
			break;
		}
		if( Next()->TEETmp() != NULL && !bEqualType )
		{
			SNode* pNext = Space::GSpaceMgr->ReBuildVarChoice( this );
			assert( pNext == Next() );
			Next()->TEETmp( TEETmp() );
		}
	}
}

bool	SVarRef::GetLuaCode( wstring& luaCode, int depth )
{
	Space::SNode* pDefinition = Definition();
	wstring varName = GetVarName(this);
	if( Name().empty() && TEETmp() != NULL )
	{
		luaCode += TEETmp()->DisplayName();
	}
	else
	{
		if( pDefinition->IsA(Space::SNT_TRIGGER) )
		{
			//TODO:
			DWORD TriggerId = reinterpret_cast<DWORD>(pDefinition->UserData("id"));
			TCHAR temp[1024];
			StringCchPrintf( temp, 1024, _T("Trigger_%d"), TriggerId );
			luaCode = _T("\"") + wstring(temp) + _T("\"");
		}
		else
		{
			luaCode = varName;
		}
	}

	if (pDefinition && IsArray())
	{
		for (Space::SNode* pChild = FirstChild(); pChild != NULL; pChild = pChild->Next())
		{
			luaCode += _T("[");
			wstring childCode;
			if (pChild->GetLuaCode(childCode, 0))
			{
				luaCode += childCode;
			}
			luaCode += _T("]");
		}
	}
	return true;
}
bool	SVarRef::GetRealParam( wstring& realParam ) const
{
	const_cast<SVarRef*>(this)->GetLuaCode( realParam, 0 );
	return true;
}
//////////////////////////////////////////////////////////////////////////
// SParamVariable
IMPLEMENT_BLOCK_ALLOCATE(SContextVarRef, 32)

SContextVarRef::SContextVarRef()
{
	ClassType(SNT_CONTEXT_VARREF);
}

bool	SContextVarRef::PostXMLLoaded()
{
	UserData(EventParamName, NULL);
	// Find TEE::ContextParam.
	SNode* pOwnerTrigger = Ancestor(SNT_TRIGGER);
	if (pOwnerTrigger)
	{
		SNode* pEventRoot = pOwnerTrigger->FindFirstChild(SVerifyNodeType(SNT_EVENTROOT));
		if (pEventRoot != NULL)
		{
			SNode* pEvent = pEventRoot->FindFirstChild(SVerifyNodeType(SNT_EVENT));
			if (pEvent != NULL)
			{
				TEE::NodeBase* pTEEEvent = TEE::GTEEMgr->FindEventFromCode(pEvent->Name());
				if (pTEEEvent != NULL)
				{
					TEE::NodeBase* pTEEParam = pTEEEvent->FindFirstChild(TEE::NodeTypeAndCodeNamePredicate(TEE::VerifyNodeType(TEE::NT_CONTEXTPARAM),TEE::VerifyCodeName(Name())), true);
					UserData(EventParamName, reinterpret_cast<void*>(pTEEParam));
				}
			}
		}
	}

	return true;
}
bool	SContextVarRef::GetContextView( ContextView& cnxt )
{
	cnxt.pNode = this;
	cnxt.bValid = false;
	cnxt.bPreferences = true;
	cnxt.bEnable = true;
	cnxt.text.clear();

	if (Name().length() > 0)
	{
		cnxt.text = Name();
		cnxt.bValid = true;
	}
	else if( TEETmp() != NULL )
	{
		cnxt.text += TEETmp()->DisplayName();
	}

	return true;
}

ErrorMask	SContextVarRef::CheckError()
{
	assert( GetErrorMask() == VS_OK );
	TEE::NodeBase* pTEETmp = TEETmp();
	if( pTEETmp == NULL )
	{
		AddErrorFlag( VS_TEETEMP_ERR );
	}
	if( Name().empty() )
	{
		AddErrorFlag( VS_REFERENCE_ERR );
	}
	if( UserData(EventParamName) == NULL )
	{
		AddErrorFlag( VS_REFERENCE_ERR );
	}

	return GetErrorMask();
}
//////////////////////////////////////////////////////////////////////////
//SGlobalVariable
IMPLEMENT_BLOCK_ALLOCATE(SGlobalVarRef, 32)
SGlobalVarRef::SGlobalVarRef()
{
	ClassType(SNT_GLOBAL_VARREF);
}
bool	SGlobalVarRef::PostXMLLoaded()
{
	Space::SNode* pImpl = NULL;
	if( bool bTgr = UserData("bTgr") != nullptr )
	{
		Space::SVerifyNameType verify;
		verify.name = Name();
		pImpl = Space::GSpaceMgr->FindTrigger( verify );
		assert( pImpl != NULL );
		Definition( pImpl );
	}
	else
	{
		Space::SNode* pVarSet = Space::GSpaceMgr->FindRoot(Space::SNT_VARSET, NULL);
		Space::SVerifyNameType verify;
		verify.name = Name();
		pImpl = pVarSet ? pVarSet->FindFirstChild(verify) : NULL;
		assert( pImpl != NULL );
		if( pImpl && pImpl->TEETmp() == TEETmp() )
		{
			Definition( pImpl );
		}
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
//SLocalVariable
IMPLEMENT_BLOCK_ALLOCATE(SLocalVarRef, 32)
SLocalVarRef::SLocalVarRef()
{
	ClassType(SNT_LOCAL_VARREF);
}
bool	SLocalVarRef::PostXMLLoaded()
{
	Space::SNode* pImpl = NULL;
	if( bool bTgr = UserData("bTgr") != nullptr )
	{
		Space::SVerifyNameType verify;
		verify.name = Name();
		pImpl = Space::GSpaceMgr->FindTrigger( verify );
		assert( pImpl != NULL );
		Definition( pImpl );
	}
	else
	{
		Definition( NULL );
		if (SNode* pActionRoot = Ancestor(SNT_ACTIONROOT))
		{
			pImpl = pActionRoot->FindFirstChild( SNodeTypeAndNamePredicate(SVerifyNodeType(SNT_VARIABLE),SVerifyNameType(Name())) );
			if( pImpl && pImpl->TEETmp() == TEETmp() )
			{
				Definition( pImpl );
			}
		}
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
//SParamVarRef
IMPLEMENT_BLOCK_ALLOCATE(SParamVarRef, 32)

SParamVarRef::SParamVarRef()
{
	ClassType(SNT_PARAM_VARREF);
}
bool	SParamVarRef::PostXMLLoaded()
{
	Space::SNode* pImpl = NULL;
	if (bool bTgr = UserData("bTgr") != nullptr)
	{
		Space::SVerifyNameType verify;
		verify.name = Name();
		pImpl = Space::GSpaceMgr->FindTrigger(verify);
		assert(pImpl != NULL);
		Definition(pImpl);
	}
	else
	{
		Definition(NULL);
		if (SNode* pFunction = Ancestor(SNT_FUNCTION))
		{
			if (SNode* pParamRoot = pFunction->FindFirstChild(SVerifyNodeType(SNT_PARAMROOT)))
			{
				pImpl = pParamRoot->FindFirstChild(SNodeTypeAndNamePredicate(SVerifyNodeType(SNT_VARIABLE), SVerifyNameType(Name())));
				if (pImpl && pImpl->TEETmp() == TEETmp())
				{
					Definition(pImpl);
				}
			}
		}
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
//SSetVariable
IMPLEMENT_BLOCK_ALLOCATE(SSetVariable, 32)
SSetVariable::SSetVariable()
{
	ClassType(SNT_SET_VARIABLE);
}
TiXmlElement*	SSetVariable::ToXMLElement()
{
	TiXmlElement *pXmlElement = SNode::ToXMLElement();

	return pXmlElement;
}
bool	SSetVariable::FromXMLElement( const TiXmlElement* pXMLElement )
{
	SNode::FromXMLElement( pXMLElement );

	TEE::NodeBase* pTmpNode = TEE::GTEEMgr->FindFunctionFromCode(Name());
	assert( pTmpNode != NULL );
	TEETmp( pTmpNode );

	return true;
}

bool	SSetVariable::GetContextView( ContextView& cnxt )
{
	cnxt.pNode = this;
	cnxt.bValid = GetErrorMask() == VS_OK;
	cnxt.bPreferences = true;
	cnxt.text = GetReadText();

	if( !cnxt.text.empty() )
	{
		cnxt.text = _T("(") + cnxt.text + _T(")");
	}

	return true;
}

void	SSetVariable::GetContextViews( vector<ContextView>& cnxts )
{
	ContextView cnxt;
	cnxt.bPreferences = false;
	cnxt.text = _T("Set ");
	cnxts.push_back( cnxt );


	SNode* pChild = FirstChild();
	if( pChild != NULL )
	{
		cnxt.text.clear();
		pChild->GetContextView( cnxt );
		cnxts.push_back( cnxt );

		pChild->GetChildViews( cnxts );
	}

	if( pChild != NULL )
		pChild = pChild->Next();

	if( pChild != NULL )
	{
		cnxt.pNode = NULL;
		cnxt.bValid = true;
		cnxt.bPreferences = false;
		cnxt.bEnable = true;
		cnxt.text = _T(" = ");
		cnxts.push_back( cnxt );

		cnxt.text.clear();
		pChild->GetContextView( cnxt );
		cnxts.push_back( cnxt );

		pChild->GetChildViews( cnxts );
	}
}
wstring	SSetVariable::GetReadText() const
{
	wstring text;

	text += _T("Set ");

	SNode* pChild = FirstChild();
	text += pChild->GetReadText();

	text += _T(" = ");

	pChild = pChild->Next();
	if( pChild != NULL )
		text += pChild->GetReadText();

	return text;
}

ErrorMask	SSetVariable::CheckError()
{
	assert( GetErrorMask() == VS_OK );
	TEE::NodeBase* pTEETmp = TEETmp();
	if( pTEETmp == NULL )
	{
		AddErrorFlag( VS_TEETEMP_ERR );
	}
	if( Name().empty() )
	{
		AddErrorFlag( VS_REFERENCE_ERR );
	}
	for ( SNode* pChild = FirstChild(); pChild != NULL; pChild = pChild->Next() )
	{
		if( pChild->ClassType() == SNT_PARAMETER )
		{
			AddErrorFlag( VS_PARAM_PENDING );
			break;
		}
	}
	return GetErrorMask();
}
bool	SSetVariable::Pasteable( const SNode* pSrc )
{
	if( pSrc == NULL )
		return false;

	if( IsActionType(pSrc->ClassType()) )
		return true;

	return false;
}
bool	SSetVariable::GetLuaCode( wstring& luaCode, int depth )
{
	for( int k = 0; k < depth; ++k )
		luaCode += _T("\t");

	SNode* pChild = FirstChild();
	if( pChild == NULL )
		return false;

	wstring childCode;
	if( pChild->GetLuaCode( childCode, 0 ) )
	{
		luaCode += childCode;
	}
	luaCode += _T(" = ");

	pChild = pChild->Next();
	if( pChild == NULL )
		return false;

	childCode.clear();
	if( pChild->GetLuaCode( childCode, 0 ) )
	{
		luaCode += childCode;
	}

	return true;
}
