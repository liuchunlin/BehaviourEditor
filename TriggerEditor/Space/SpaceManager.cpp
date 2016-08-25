#include "SpaceManager.h"
#include "SRoot.h"
#include "SGroup.h"
#include "STrigger.h"
#include "SEvent.h"
#include "SCondition.h"
#include "SActionRoot.h"
#include "SParameter.h"
#include "SStaticLabel.h"
#include "SFuncCall.h"
#include "SAdjust.h"
#include "SVariable.h"
#include "SEditValue.h"
#include "SMultiIF.h"
#include "SForEach.h"
#include "SBoolean.h"
#include "SWhile.h"
#include "SFunction.h"

#include "TEEManager.h"
#include "NodeVerify.h"

#include "NodeBase.h"
#include "Parameter.h"

namespace Space
{
	SNode* BuildStatementSOrT(Vek::TreeNodeBase* base)
	{
		if (TEE::NodeBase* pTNode = dynamic_cast<TEE::NodeBase*>(base))
			return GSpaceMgr->BuildStatement(pTNode);
		else if (SNode* pSNode = dynamic_cast<SNode*>(base))
			return GSpaceMgr->BuildStatement(pSNode);
		return nullptr;
	}

	SpaceManager	*GSpaceMgr = SpaceManager::GetInstance();

	static const map<wstring, SPACE_NODE_TYPE>& NodeNameToTypeMap()
	{
		static map<wstring, SPACE_NODE_TYPE> s_NameToType;
		if( s_NameToType.empty() )
		{
			s_NameToType.insert( make_pair( _T("SpaceRoot"),		SNT_ROOT) );
			s_NameToType.insert( make_pair( _T("Group"),			SNT_GROUP) );
			s_NameToType.insert( make_pair( _T("Trigger"),			SNT_TRIGGER) );
			s_NameToType.insert( make_pair( _T("EventRoot"),		SNT_EVENTROOT) );
			s_NameToType.insert( make_pair( _T("Event"),			SNT_EVENT) );
			s_NameToType.insert( make_pair( _T("ConditionRoot"),	SNT_CONDITIONROOT) );
			s_NameToType.insert( make_pair( _T("Condition"),		SNT_CONDITION) );
			s_NameToType.insert( make_pair( _T("ActionRoot"),		SNT_ACTIONROOT) );
			s_NameToType.insert( make_pair( _T("Parameter"),		SNT_PARAMETER) );
			s_NameToType.insert( make_pair( _T("StaticLabel"),		SNT_STATICLABEL) );
			s_NameToType.insert( make_pair( _T("TFuncCall"),		SNT_TFUNCCALL));
			s_NameToType.insert( make_pair( _T("SFuncCall"),		SNT_SFUNCCALL));
			s_NameToType.insert( make_pair( _T("Adjust"),			SNT_ADJUST) );
			s_NameToType.insert( make_pair( _T("GlobalVarRef"),		SNT_GLOBAL_VARREF) );
			s_NameToType.insert( make_pair( _T("LocalVarRef"),		SNT_LOCAL_VARREF) );
			s_NameToType.insert( make_pair( _T("ContextVarRef"),	SNT_CONTEXT_VARREF));
			s_NameToType.insert( make_pair( _T("ParamVarRef"),		SNT_PARAM_VARREF));
			s_NameToType.insert( make_pair( _T("EditValue"),		SNT_EDIT_VALUE) );
			s_NameToType.insert( make_pair( _T("MultiIF"),			SNT_MULTIPLE_IF) );
			s_NameToType.insert( make_pair( _T("If"),				SNT_IF) );
			s_NameToType.insert( make_pair( _T("Then"),				SNT_THEN) );
			s_NameToType.insert( make_pair( _T("Else"),				SNT_ELSE) );
			s_NameToType.insert( make_pair( _T("ForEach"),			SNT_FOR_EACH) );
			s_NameToType.insert( make_pair( _T("Loop"),				SNT_LOOP) );
			s_NameToType.insert( make_pair( _T("Boolean"),			SNT_BOOLEAN) );
			s_NameToType.insert( make_pair( _T("VarSet"),			SNT_VARSET) );
			s_NameToType.insert( make_pair( _T("Variable"),			SNT_VARIABLE) );
			s_NameToType.insert( make_pair( _T("SetVariable"),		SNT_SET_VARIABLE) );
			s_NameToType.insert( make_pair( _T("While"),			SNT_WHILE));
			s_NameToType.insert( make_pair( _T("Function"),			SNT_FUNCTION));
			s_NameToType.insert( make_pair( _T("ParamRoot"),		SNT_PARAMROOT));
			s_NameToType.insert( make_pair( _T("ReturnType"),		SNT_RETURNTYPE));
			s_NameToType.insert( make_pair( _T("Return"),			SNT_RETURN));
		}
		return s_NameToType;
	}

	SNode*	SpaceManager::CreateNode( SPACE_NODE_TYPE eType )
	{
		switch (eType)
		{
		case SNT_ROOT: return new SRoot;
		case SNT_GROUP: return new SGroup;
		case SNT_TRIGGER: return new STrigger;
		case SNT_EVENTROOT: return new SEventRoot;
		case SNT_EVENT: return new SEvent;
		case SNT_CONDITIONROOT: return new SConditionRoot;
		case SNT_CONDITION: return new SCondition;
		case SNT_ACTIONROOT: return new SActionRoot;
		case SNT_PARAMETER: return new SParameter;
		case SNT_STATICLABEL: return new SStaticLabel;
		case SNT_TFUNCCALL: return new STFuncCall;
		case SNT_SFUNCCALL: return new SSFuncCall;
		case SNT_ADJUST: return new SAdjust;
		case SNT_GLOBAL_VARREF: return new SGlobalVarRef;
		case SNT_LOCAL_VARREF: return new SLocalVarRef;
		case SNT_CONTEXT_VARREF: return new SContextVarRef;
		case SNT_PARAM_VARREF: return new SParamVarRef;
		case SNT_EDIT_VALUE: return new SEditValue;
		case SNT_MULTIPLE_IF: return new SMultiIF;
		case SNT_IF: return new SIF;
		case SNT_THEN: return new SThen;
		case SNT_ELSE: return new SElse;
		case SNT_FOR_EACH: return new SForEach;
		case SNT_LOOP: return new SLoop;
		case SNT_BOOLEAN: return new SBoolean;
		case SNT_VARSET: return new SVarSet;
		case SNT_VARIABLE: return new SVariable;
		case SNT_SET_VARIABLE: return new SSetVariable;
		case SNT_WHILE: return new SWhile;
		case SNT_FUNCTION: return new SFunction;
		case SNT_PARAMROOT: return new SParamRoot;
		case SNT_RETURNTYPE: return new SReturnType;
		case SNT_RETURN: return new SReturn;
		}
		assert( false );
		return NULL;
	}

	bool IsActionType(SPACE_NODE_TYPE eType)
	{
		switch (eType)
		{
		case SNT_TFUNCCALL:
		case SNT_SFUNCCALL:
		case SNT_MULTIPLE_IF:
		case SNT_FOR_EACH:
		case SNT_SET_VARIABLE:
		case SNT_WHILE:
		case SNT_RETURN:
			return true;
		}
		return false;
	}

	static const map<SPACE_NODE_TYPE,wstring>& NodeTypeToNameMap()
	{
		static map<SPACE_NODE_TYPE,wstring> s_TypeToName;
		if (s_TypeToName.empty())
		{
			const map<wstring, SPACE_NODE_TYPE>& NameMap = NodeNameToTypeMap();
			map<wstring, SPACE_NODE_TYPE>::const_iterator itr = NameMap.begin();
			for ( ; itr != NameMap.end(); ++itr)
			{
				s_TypeToName[itr->second] = itr->first;
			}
		}
		return s_TypeToName;
	}
	SPACE_NODE_TYPE SpaceManager::NodeNameToType( const wstring& nodeName )
	{
		const map<wstring, SPACE_NODE_TYPE>& NameMap = NodeNameToTypeMap();
		map<wstring, SPACE_NODE_TYPE>::const_iterator itr = NameMap.find( nodeName );
		if( itr != NameMap.end() )
		{
			return itr->second;
		}
		assert( false );
		return SNT_NULL;
	}
	const wstring& SpaceManager::NodeTypeToName( SPACE_NODE_TYPE eType )
	{
		const map<SPACE_NODE_TYPE,wstring>& TypeMap = NodeTypeToNameMap();
		map<SPACE_NODE_TYPE,wstring>::const_iterator itr = TypeMap.find(eType);
		if (itr != TypeMap.end())
		{
			return itr->second;
		}
		assert( false );
		static wstring nullString = TEXT("");
		return nullString;
	}
	const wstring& SpaceManager::GetTypeBaseName( SPACE_NODE_TYPE eType )
	{
		static map<SPACE_NODE_TYPE,wstring> s_BaseNames;
		if( s_BaseNames.empty() )
		{
			s_BaseNames.insert( make_pair( SNT_NULL,				_T("")) );
			s_BaseNames.insert( make_pair( SNT_EVENT,				_T("事件")) );
			s_BaseNames.insert( make_pair( SNT_CONDITION,			_T("条件")) );
			s_BaseNames.insert( make_pair( SNT_TFUNCCALL,			_T("动作")));
			s_BaseNames.insert( make_pair( SNT_SFUNCCALL,			_T("动作")));
			s_BaseNames.insert( make_pair( SNT_MULTIPLE_IF,			_T("动作")));
			s_BaseNames.insert( make_pair( SNT_FOR_EACH,			_T("动作")) );
			s_BaseNames.insert( make_pair( SNT_SET_VARIABLE,		_T("动作")) );
			s_BaseNames.insert( make_pair( SNT_WHILE,				_T("动作")));
			s_BaseNames.insert( make_pair( SNT_RETURN,				_T("动作")));
		}
		if( s_BaseNames.find( eType ) == s_BaseNames.end() )
		{
			assert( false );
			return s_BaseNames[SNT_NULL];
		}
		return s_BaseNames[eType];
	}
	const bool	SpaceManager::IsInternalType( const wstring& name )
	{
		if( name == _T("Action") )
			return true;
		else if( name == _T("Condition") )
			return true;
		else if( name == _T("Event") )
			return true;
		return false;
	}

	SNode*	SpaceManager::CreateNode( const wstring& strType )
	{
		return CreateNode( NodeNameToType(strType) );
	}

	SpaceManager::SpaceManager()
	{
		m_pRoot = NULL;
	}
	SpaceManager::~SpaceManager()
	{
		Flush();
	}

	template<>
	wstring	SpaceManager::CreateDefaultName<SNT_ROOT>( const wstring& base )
	{
		return base;
	}
	template<>
	wstring	SpaceManager::CreateDefaultName<SNT_GROUP>( const wstring& base )
	{
		wstring defaultName;
		SNode*	pSpaceRoot = m_pRoot;
		if( pSpaceRoot == NULL )
			return defaultName;

		set<wstring> GroupNames;
		for ( SNode* pChild = pSpaceRoot->FirstChild(); pChild != NULL; pChild = pChild->Next() )
		{
			GroupNames.insert( pChild->Name() );
		}
		int iIndex = 1;
		TCHAR buf [512];
		for( int iIndex = 1; ; ++iIndex )
		{
			StringCbPrintfW( buf, 512, _T("%s %.3d"), base.c_str(), iIndex );
			if( GroupNames.find(buf) == GroupNames.end() )
			{
				defaultName = buf;
				break;
			}
		}
		return defaultName;
	}
	bool	SpaceManager::AddTriggerID( DWORD id )
	{
		set<DWORD>::_Pairib pair_ = m_TriggerIDs.insert( id );
		return pair_.second;
	}
	void	SpaceManager::RemoveTriggerID( DWORD id )
	{
		m_TriggerIDs.erase( id );
	}
	DWORD	SpaceManager::CreateTriggerID()
	{
		DWORD id = 1;
		for(  ; ; ++id )
		{
			if( m_TriggerIDs.find(id) == m_TriggerIDs.end() )
			{
				AddTriggerID( id );
				return id;
			}
		}
		assert( false );
		return id;
	}

	template<>
	wstring	SpaceManager::CreateDefaultName<SNT_TRIGGER>( const wstring& base )
	{
		set<wstring> names;
		Vek::ObjLink<STrigger>::VisitAll( [&names](STrigger* pTrigger)->void {
			names.insert(pTrigger->Name());
		});

		int iIndex = 1;
		TCHAR buf [512];
		for( int iIndex = 1; ; ++iIndex )
		{
			StringCbPrintfW( buf, 512, _T("%s %.3d"), base.c_str(), iIndex );
			if( names.find(buf) == names.end() )
			{
				return buf;
			}
		}
		return base;
	}

	template<>
	wstring	SpaceManager::CreateDefaultName<SNT_FUNCTION>(const wstring& base)
	{
		set<wstring> names;
		Vek::ObjLink<SFunction>::VisitAll([&names](SFunction* pFunction)->void {
			names.insert(pFunction->Name());
		});

		int iIndex = 1;
		TCHAR buf[512];
		for (int iIndex = 1; ; ++iIndex)
		{
			StringCbPrintfW(buf, 512, _T("%s%.3d"), base.c_str(), iIndex);
			if (names.find(buf) == names.end())
			{
				return buf;
			}
		}
		return base;
	}

	wstring	SpaceManager::CreateDefaultName( SPACE_NODE_TYPE eType, const wstring& base )
	{
		switch(eType)
		{
		case SNT_ROOT:
			return CreateDefaultName<SNT_ROOT>(base);
		case SNT_GROUP:
			return CreateDefaultName<SNT_GROUP>(base);
		case SNT_TRIGGER:
			return CreateDefaultName<SNT_TRIGGER>(base);
		case SNT_FUNCTION:
			return CreateDefaultName<SNT_FUNCTION>(base);
		default:
			assert( false );
			return base;
		}
		return base;
	}

	void	SpaceManager::UniqueName( SNode* pSrc )
	{
		if( pSrc == NULL )
			return;

		if( pSrc->UniqueName() )
			pSrc->Name( CreateDefaultName( pSrc->ClassType(), pSrc->Name() + _T(" 复制") ) );
		UniqueName( pSrc->FirstChild() );
		UniqueName( pSrc->Next() );
	}

	template<SPACE_NODE_TYPE TYPE>
	wstring	SpaceManager::CreateDefaultName( const wstring& base )
	{
		assert( false );
		return base;
	}

	template<>
	SNode*	SpaceManager::BuildDefault<SNT_VARSET>( TEE::NodeBase* pTemplate, SNode* pParent, SNode* pPrev )
	{
		SNode* pVarSet = CreateNode( SNT_VARSET );
		//pVarSet->Name( );

		if( pParent != NULL )
		{
			pParent->InsertOrAppend(pVarSet, pPrev);
		}

		return pVarSet;
	}
	template<>
	SNode*	SpaceManager::BuildDefault<SNT_GROUP>( TEE::NodeBase* pTemplate, SNode* pParent, SNode* pPrev )
	{
		SNode* pGroup = CreateNode( SNT_GROUP );
		pGroup->Name( CreateDefaultName<SNT_GROUP>( _T("未命名分组") ) );

		if( pParent != NULL )
		{
			pParent->InsertOrAppend(pGroup, pPrev);
		}

		return pGroup;
	}
	template<>
	SNode*	SpaceManager::BuildDefault<SNT_TRIGGER>( TEE::NodeBase* pTemplate, SNode* pParent, SNode* pPrev )
	{
		SNode* pTrigger = CreateNode( SNT_TRIGGER );
		pTrigger->Name( CreateDefaultName<SNT_TRIGGER>( _T("未命名触发器") ) );
		pTrigger->UserData( "id", reinterpret_cast<void*>(CreateTriggerID()) );

		SNode* pEventRoot = CreateNode( SNT_EVENTROOT );
		pEventRoot->Name( _T("事件") );
		pTrigger->AddChild(pEventRoot);
		SNode* pConditionRoot = CreateNode( SNT_CONDITIONROOT );
		pConditionRoot->Name( _T("环境") );
		pTrigger->AddChild(pConditionRoot);
		SNode* pActionRoot = CreateNode( SNT_ACTIONROOT );
		pActionRoot->Name( _T("动作") );
		pTrigger->AddChild(pActionRoot);

		if( pParent != NULL )
		{
			pParent->InsertOrAppend(pTrigger, pPrev);
		}

		return pTrigger;
	}
	template<>
	SNode*	SpaceManager::BuildDefault<SNT_FUNCTION>(TEE::NodeBase* pTemplate, SNode* pParent, SNode* pPrev)
	{
		SFunction* pFunction = dynamic_cast<SFunction*>( CreateNode(SNT_FUNCTION) );
		pFunction->Name(CreateDefaultName<SNT_FUNCTION>(TEXT("NewFunction")));
		pFunction->m_desc = TEXT("新建函数");

		SNode* pParamRoot = CreateNode(SNT_PARAMROOT);
		pParamRoot->Name(_T("参数列表"));
		pFunction->AddChild(pParamRoot);
		SNode* pReturnType = CreateNode(SNT_RETURNTYPE);
		pReturnType->Name(VOID_TEE);
		pFunction->AddChild(pReturnType);
		SNode* pActionRoot = CreateNode(SNT_ACTIONROOT);
		pActionRoot->Name(_T("动作"));
		pFunction->AddChild(pActionRoot);

		pReturnType->TEETmp(TEE::GTEEMgr->FindOptionFromCode(pReturnType->Name()));

		if (pParent != NULL)
		{
			pParent->InsertOrAppend(pFunction, pPrev);
		}

		return pFunction;
	}
	template<>
	SNode*	SpaceManager::BuildDefault<SNT_MULTIPLE_IF>( TEE::NodeBase* pTemplate, SNode* pParent, SNode* pPrev )
	{
		SNode* pMulIf = CreateNode( SNT_MULTIPLE_IF );
		pMulIf->Name( pTemplate->CodeName() );
		pMulIf->TEETmp( pTemplate );

		SNode* pIf = CreateNode( SNT_IF );
		pIf->Name( _T("If-环境") );
		pMulIf->AddChild(pIf);
		SNode* pThen = CreateNode( SNT_THEN );
		pThen->Name( _T("Then-动作") );
		pMulIf->AddChild(pThen);
		SNode* pElse = CreateNode( SNT_ELSE );
		pElse->Name( _T("Else-动作") );
		pMulIf->AddChild(pElse);

		if( pParent != NULL )
		{
			pParent->InsertOrAppend(pMulIf, pPrev);
		}

		return pMulIf;
	}

	template<>
	SNode*	SpaceManager::BuildDefault<SNT_WHILE>( TEE::NodeBase* pTemplate, SNode* pParent, SNode* pPrev )
	{
		SNode* pWhile = CreateNode( SNT_WHILE );
		pWhile->Name( pTemplate->CodeName() );
		pWhile->TEETmp( pTemplate );

		SNode* pCondition = CreateNode( SNT_IF );
		pCondition->Name( _T("while-环境") );
		pWhile->AddChild(pCondition);
		SNode* pWhileAction = CreateNode( SNT_LOOP );
		pWhileAction->Name( _T("loop-动作") );
		pWhile->AddChild(pWhileAction);

		if( pParent != NULL )
		{
			pParent->InsertOrAppend(pWhile, pPrev);
		}

		return pWhile;
	}

	template<>
	SNode*	SpaceManager::BuildDefault<SNT_ADJUST>( TEE::NodeBase* pTemplate, SNode* pParent, SNode* pPrev )
	{
		if( pTemplate == NULL )
		{
			assert( false );
			return NULL;
		}

		SNode* pNewNode = CreateNode( SNT_ADJUST );
		pNewNode->Name( pTemplate->DisplayName() );
		pNewNode->TEETmp( pTemplate );

		if (pParent) pParent->InsertOrAppend(pNewNode, pPrev);
		return pNewNode;
	}
	template<>
	SNode*	SpaceManager::BuildDefault<SNT_EDIT_VALUE>( TEE::NodeBase* pTemplate, SNode* pParent, SNode* pPrev )
	{
		if( pTemplate == NULL )
		{
			assert( false );
			return NULL;
		}

		SNode* pNewNode = CreateNode( SNT_EDIT_VALUE );
		pNewNode->TEETmp( pTemplate );

		if (pParent) pParent->InsertOrAppend(pNewNode, pPrev);
		return pNewNode;
	}


	template<>
	SNode*	SpaceManager::BuildDefault<SNT_BOOLEAN>( TEE::NodeBase* pTemplate, SNode* pParent, SNode* pPrev )
	{
		if( pTemplate == NULL )
		{
			assert( false );
			return NULL;
		}

		SNode* pNewNode = CreateNode( SNT_BOOLEAN );
		pNewNode->TEETmp( pTemplate );

		if (pParent) pParent->InsertOrAppend(pNewNode, pPrev);
		return pNewNode;
	}
	template<>
	SNode*	SpaceManager::BuildDefault<SNT_VARIABLE>( TEE::NodeBase* pTemplate, SNode* pParent, SNode* pPrev )
	{
		if( pTemplate == NULL )
		{
			assert( false );
			return NULL;
		}

		SNode* pNewNode = CreateNode( SNT_VARIABLE );
		pNewNode->TEETmp( pTemplate );
		{
			ReBuildVariableDefault( pNewNode );
		}

		if (pParent) pParent->InsertOrAppend(pNewNode, pPrev);
		return pNewNode;
	}
	template<>
	SNode*	SpaceManager::BuildDefault<SNT_PARAMETER>( TEE::NodeBase* pTemplate, SNode* pParent, SNode* pPrev )
	{
		if( pTemplate == NULL )
		{
			assert( false );
			return NULL;
		}

		SNode *pNewNode = CreateNode( SNT_PARAMETER );
		SParameter* pPref = static_cast<SParameter*>(pNewNode);
		pPref->Name( pTemplate->CodeName() );
		pPref->TEETmp( pTemplate );

		if (pParent) pParent->InsertOrAppend(pNewNode, pPrev);
		return pNewNode;
	}
	SNode*	SpaceManager::ReBuildVariableIndex( SNode* pSrc )
	{
		if( pSrc == NULL )
			return NULL;

		pSrc->RemoveChildren();

		TEE::NodeBase* pOption = TEE::GTEEMgr->FindOptionFromCode(_T("INT"));

		SNode* pIndex = BuildDefault<SNT_EDIT_VALUE>( pOption, pSrc );
		pIndex->Name( _T("1") );
		pIndex->CheckError();

		return pIndex;
	}
	SNode*	SpaceManager::ReBuildVariableDefault( SNode* pSrc )
	{
		if( pSrc == NULL || pSrc->TEETmp() == NULL )
			return NULL;

		pSrc->RemoveChildren();

		SParameter* pPref = static_cast<SParameter*>(CreateNode( SNT_PARAMETER ));
		pPref->Name( pSrc->TEETmp()->DisplayName() );
		pPref->TEETmp( pSrc->TEETmp() );
		pSrc->AddChild(pPref);

		return pPref;
	}
	SNode*	SpaceManager::ReBuildVarChoice( SNode* pSrc )
	{
		if( pSrc == NULL || (pSrc->ClassType() != SNT_GLOBAL_VARREF && pSrc->ClassType() != SNT_LOCAL_VARREF) )
			return NULL;

		if (pSrc->Next()) pSrc->Next()->Remove(true);

		SParameter* pValue = static_cast<SParameter*>(CreateNode( SNT_PARAMETER ));
		pValue->Name( _T("Value") );
		if (pSrc->Parent()) pSrc->Parent()->InsertOrAppend(pValue, pSrc);

		return pValue;
	}

	template<SPACE_NODE_TYPE TYPE>
	SNode*	SpaceManager::BuildDefault( TEE::NodeBase* pTemplate, SNode* pParent, SNode* pPrev )
	{
		if( pTemplate == NULL )
		{
			assert( false );
			return NULL;
		}

		SNode* pNewNode = CreateNode( TYPE );
		pNewNode->TEETmp( pTemplate );

		if (pParent) pParent->InsertOrAppend(pNewNode, pPrev);
		return pNewNode;
	}
	template<>
	SNode*	SpaceManager::BuildDefault<SNT_CONTEXT_VARREF>( TEE::NodeBase* pTemplate, SNode* pParent, SNode* pPrev )
	{
		if( pTemplate == NULL )
		{
			assert( false );
			return NULL;
		}

		SNode* pNewNode = CreateNode( SNT_CONTEXT_VARREF );
		pNewNode->TEETmp( pTemplate );

		if (pParent) pParent->InsertOrAppend(pNewNode, pPrev);
		return pNewNode;
	}

	template<>
	SNode*	SpaceManager::BuildDefault<SNT_LOCAL_VARREF>( TEE::NodeBase* pTemplate, SNode* pParent, SNode* pPrev )
	{
		SNode* pNewNode = CreateNode( SNT_LOCAL_VARREF );
		pNewNode->TEETmp( pTemplate );
		ReBuildVariableIndex( pNewNode );

		if (pParent) pParent->InsertOrAppend(pNewNode, pPrev);
		return pNewNode;
	}

	template<>
	SNode*	SpaceManager::BuildDefault<SNT_PARAM_VARREF>(TEE::NodeBase* pTemplate, SNode* pParent, SNode* pPrev)
	{
		SNode* pNewNode = CreateNode(SNT_PARAM_VARREF);
		pNewNode->TEETmp(pTemplate);
		ReBuildVariableIndex(pNewNode);

		if (pParent) pParent->InsertOrAppend(pNewNode, pPrev);
		return pNewNode;
	}

	SNode*	SpaceManager::BuildParameter( TEE::NodeBase* pOption, wstring defaultValue, SNode* pParent )
	{
		if (!pOption->IsA(TEE::NT_OPTION) || pOption->CodeName() == VOID_TEE)
			return nullptr;

		SNode* pNewNode = NULL;

		wstring optionCodeName = pOption->CodeName();
		//特殊类型
		if(optionCodeName == _T("CONDITION") )
		{
			pNewNode = CreateNode( SNT_CONDITION );
			if (pParent) pParent->AddChild(pNewNode);
			return pNewNode;
		}
		else if (optionCodeName == _T("ACTION"))
		{
			pNewNode = CreateNode(SNT_TFUNCCALL);
			if (pParent) pParent->AddChild(pNewNode);
			return pNewNode;
		}
		else if(optionCodeName == _T("EVENT") )
		{
			pNewNode = CreateNode( SNT_EVENT );
			if (pParent) pParent->AddChild(pNewNode);
			return pNewNode;
		}
		else
		{
			if (!defaultValue.empty())
			{
				TEE::NodeBase* pAdjust = pOption->FindFirstChild( TEE::VerifyDisplayName(defaultValue) );
				if( pAdjust != NULL )
				{
					pNewNode = BuildDefault<SNT_ADJUST>( pAdjust, pParent );
					return pNewNode;
				}
				else if( pOption != NULL )
				{
					if( TEE::VT_INT == TEE::GetValueTypeEnum(optionCodeName) ||
						TEE::VT_REAL == TEE::GetValueTypeEnum(optionCodeName) ||
						TEE::VT_STRING == TEE::GetValueTypeEnum(optionCodeName) )
					{
						pNewNode = BuildDefault<SNT_EDIT_VALUE>( pOption, pParent );
						pNewNode->Name( defaultValue );
						pNewNode->CheckError();
						return pNewNode;
					}
				}
			}
		}

		pNewNode = BuildDefault<SNT_PARAMETER>(pOption, pParent);
		return pNewNode;
	}

	void	SpaceManager::BuildParameters( TEE::NodeBase* pTemplate, SNode* pParent )
	{
		if( pTemplate == NULL )	return;

		TEE::NodeBase* pChild = pTemplate->FirstChild();
		for ( ; pChild ; pChild = pChild->Next() )
		{
			if( pChild->ClassType() == TEE::NT_PARAMETER )
			{
				TEE::Parameter* pParam = static_cast<TEE::Parameter*>(pChild);
				TEE::NodeBase* pOption = TEE::GTEEMgr->FindOptionFromCode(pParam->m_DisplayName);
				if (pOption == nullptr) continue;
				BuildParameter(pOption, pParam->m_DefaultValue, pParent);
			}
		}
	}

	void	SpaceManager::BuildParameters(SNode* pDef, SNode* pParent)
	{
		if (!pDef->IsA(SNT_FUNCTION)) return;

		static vector<SVariable*> sFormalParams(32);
		sFormalParams.clear();
		SFunction* function = dynamic_cast<SFunction*>(pDef);
		if (function) function->GetFormalParams(sFormalParams);

		for (size_t index = 0; index < sFormalParams.size(); ++index)
		{
			SVariable* pParam = sFormalParams[index];
			BuildParameter(pParam->TEETmp(), pParam->GetDefaultValue(), pParent);
		}
	}

	template<>
	SNode*	SpaceManager::BuildDefault<SNT_FOR_EACH>( TEE::NodeBase* pTemplate, SNode* pParent, SNode* pPrev )
	{
		if( pTemplate == NULL )
			return NULL;

		SNode* pForEach = CreateNode( SNT_FOR_EACH );
		pForEach->Name( pTemplate->CodeName() );
		pForEach->TEETmp( pTemplate );

		BuildParameters( pTemplate, pForEach );

		SNode* pLoop = CreateNode( SNT_LOOP );
		pLoop->Name( _T("Loop-动作") );
		pForEach->AddChild(pLoop);

		if( pParent != NULL )
		{
			pParent->InsertOrAppend(pForEach, pPrev);
		}

		return pForEach;
	}
	template<>
	SNode*	SpaceManager::BuildDefault<SNT_GLOBAL_VARREF>( TEE::NodeBase* pTemplate, SNode* pParent, SNode* pPrev )
	{
		SNode* pNewNode = CreateNode( SNT_GLOBAL_VARREF );
		pNewNode->TEETmp( pTemplate );
		ReBuildVariableIndex( pNewNode );

		if (pParent) pParent->InsertOrAppend(pNewNode, pPrev);
		return pNewNode;
	}
	template<>
	SNode*	SpaceManager::BuildDefault<SNT_SET_VARIABLE>( TEE::NodeBase* pTemplate, SNode* pParent, SNode* pPrev )
	{
		if( pTemplate == NULL )
			return NULL;

		SNode* pNewNode = CreateNode( SNT_SET_VARIABLE );
		pNewNode->Name( pTemplate->CodeName() );
		pNewNode->TEETmp( pTemplate );

		{
			SNode* pVarChoice = CreateNode( SNT_GLOBAL_VARREF );
			pVarChoice->Name( _T("Variable") );
			pNewNode->AddChild(pVarChoice);

			ReBuildVarChoice( pVarChoice );
		}

		if( pParent != NULL )
		{
			pParent->InsertOrAppend(pNewNode, pPrev);
		}

		return pNewNode;
	}

	//build Event, Condition, TFunctionCall
	//pDef -- definition
	SNode*	SpaceManager::BuildStatement( TEE::NodeBase* pDef, SNode* pParent, SNode* pPrev )
	{
		if( pDef == NULL ) return NULL;

		if( pDef->CodeName() == _T("MultiIF") )
		{
			return BuildDefault<SNT_MULTIPLE_IF>( pDef, pParent, pPrev );
		}
		else if( pDef->CodeName() == _T("ForEachActions") || pDef->CodeName() == _T("ForEachAActions") || pDef->CodeName() == _T("ForEachBActions") )
		{
			return BuildDefault<SNT_FOR_EACH>( pDef, pParent, pPrev );
		}
		else if( pDef->CodeName() == _T("SetVariable") )
		{
			return BuildDefault<SNT_SET_VARIABLE>( pDef, pParent, pPrev );
		}
		else if ( pDef->CodeName() == _T("While") )
		{
			return BuildDefault<SNT_WHILE>( pDef, pParent, pPrev );
		}
		else if (pDef->CodeName() == _T("Return"))
		{
			SNode* pReturn = CreateNode(SNT_RETURN);
			pReturn->TEETmp(pDef);
			return pReturn;
		}

		SPACE_NODE_TYPE eCreationType = SNT_NULL;
		switch(pDef->ClassType())
		{
		case TEE::NT_EVENT:
			eCreationType = SNT_EVENT;
			break;
		case TEE::NT_CONDITION:
			eCreationType = SNT_CONDITION;
			break;
		case TEE::NT_FUNCTION:
			eCreationType = SNT_TFUNCCALL;
			break;
		default:
			assert( false );
			break;
		}

		SNode* pNewNode = CreateNode( eCreationType );
		pNewNode->Name( pDef->CodeName() );
		pNewNode->TEETmp( pDef );

		BuildParameters( pDef, pNewNode );

		if (pParent) pParent->InsertOrAppend(pNewNode, pPrev);
		return pNewNode;
	}

	//build SFunctionCall
	//pDef -- definition
	SNode*	SpaceManager::BuildStatement(SNode* pDef, SNode* pParent, SNode* pPrev)
	{
		if (pDef == nullptr) return nullptr;
		if (!pDef->IsA(SNT_FUNCTION)) return nullptr;

		SNode* pNewNode = CreateNode(SNT_SFUNCCALL);
		pNewNode->Definition(pDef);

		BuildParameters(pDef, pNewNode);

		if (pParent) pParent->InsertOrAppend(pNewNode, pPrev);
		return pNewNode;
	}

	template<>
	SNode*	SpaceManager::BuildDefault<SNT_ROOT>( TEE::NodeBase* pTemplate, SNode* pParent, SNode* pPrev )
	{
		SAFE_DELETE( m_pRoot );

		m_pRoot = CreateNode( SNT_ROOT );
		m_pRoot->Name( CreateDefaultName<SNT_ROOT>( _T("未命名空间") ) );

		SNode* pVarSet = BuildDefault<SNT_VARSET>( NULL, m_pRoot, NULL );
		SNode* pGroup = BuildDefault<SNT_GROUP>( NULL, m_pRoot, pVarSet );
		BuildDefault<SNT_TRIGGER>( NULL, pGroup, NULL );

		return m_pRoot;
	}

	//////////////////////////////////////////////////////////////////////////
	//Find
	SNode*	SpaceManager::FindRoot( SPACE_NODE_TYPE eRootType, const SNode* pParent )
	{
		if( eRootType == SNT_ROOT )
			return m_pRoot;

		if ( pParent == NULL )
			pParent = m_pRoot;

		if( pParent == NULL )
			return NULL;

		SNode* pChild = pParent->FirstChild();
		for( ; pChild != NULL; pChild = pChild->Next() )
		{
			if( pChild->ClassType() == eRootType )
			{
				return pChild;
			}
		}
		return NULL;
	}

	bool IsNodeInSpace(SNode* pNode)
	{
		return pNode && pNode->Ancestor(SNT_ROOT);
	}

	void	SpaceManager::FindTriggers(vector<SNode*>& triggers)
	{
		Vek::ObjLink<STrigger>::VisitAll([&triggers](STrigger* pTrigger)->void {
			if (IsNodeInSpace(pTrigger))
				triggers.push_back(pTrigger);
		});
	}

	void	SpaceManager::FindFunctions(vector<SNode*>& functions)
	{
		Vek::ObjLink<SFunction>::VisitAll([&functions](SFunction* pFunc)->void {
			if (IsNodeInSpace(pFunc))
				functions.push_back(pFunc);
		});
	}

	SNode*	SpaceManager::FindFunctionFromName(const wstring& name)
	{
		Vek::ObjLink<SFunction>* link = Vek::ObjLink<SFunction>::Head();
		for ( ; link; link = link->Next())
		{
			if (link->Obj()->Name() == name && IsNodeInSpace(link->Obj()))
				return link->Obj();
		}
		return nullptr;
	}

	bool	SpaceManager::SaveNode( const wstring& DstFileName, SNode* pNode )
	{
		m_TriggerFileName = DstFileName;

		TiXmlDocument TiDoc;
		TiXmlHandle TiHandle( &TiDoc );

		bool bSuccess = true;
		TiXmlElement* pRoot = SNode::CreateNodeXml( pNode, NULL );
		bSuccess &= pRoot != NULL;
		assert( bSuccess );

		TiHandle.ToNode()->InsertEndChild( *pRoot );
		bSuccess &= TiDoc.SaveFile( FTCHARToANSI(DstFileName.c_str()) );
		assert( bSuccess );

		SAFE_DELETE( pRoot );

		return bSuccess;
	}
	SNode*	SpaceManager::LoadNode( const wstring& fileName )
	{
		m_TriggerFileName = fileName;

		bool bSuccess = true;
		TiXmlDocument TiDoc( FTCHARToANSI(m_TriggerFileName.c_str()) );
		TiXmlHandle TiHandle( &TiDoc );
		TiXmlBase::SetCondenseWhiteSpace(false);
		bSuccess &= TiDoc.LoadFile( TIXML_ENCODING_UTF8 );
		assert( bSuccess );
		if( !bSuccess )
			return NULL;

		TiXmlElement* pXmlRoot = TiHandle.FirstChild( "SpaceRoot" ).ToElement();
		assert( bSuccess &= pXmlRoot != NULL );
		if( !bSuccess )
			return NULL;

		SAFE_DELETE( m_pRoot );

		m_pRoot = CreateNode( SNT_ROOT );
		m_pRoot->FromXMLElement( pXmlRoot );
		SNode::PostLoadedSpace( m_pRoot );
		SNode::CheckErrorTree( m_pRoot );
		
		return m_pRoot;
	}

	void	SpaceManager::Flush()
	{
		SAFE_DELETE( m_pRoot );
		m_TriggerIDs.clear();
	}

	bool	SpaceManager::CheckNameValid(SNode* pSrc, const wstring& newName)
	{
		if (pSrc == NULL) return false;
		
		switch (pSrc->ClassType())
		{
		case SNT_TRIGGER:
		{
			if (pSrc->Name() != newName)
			{
				set<wstring> names;
				Vek::ObjLink<STrigger>::VisitAll([&names](STrigger* pTrigger)->void {
					names.insert(pTrigger->Name());
				});
				if (names.find(newName) != names.end())
					return false;
			}
		}
		break;
		case SNT_FUNCTION:
		{
			if (!IsValidSymbolName(newName))
				return false;
			if (pSrc->Name() != newName)
			{
				set<wstring> names;
				Vek::ObjLink<SFunction>::VisitAll([&names](SFunction* pFunction)->void {
					names.insert(pFunction->Name());
				});
				if (names.find(newName) != names.end())
					return false;
			}
		}
		break;
		}
		return true;
	}

	//检查参数模板
	// if bCheckError == true, we will not try to fix.
	void	SpaceManager::VerifyParameters(SNode* pSrc, bool bCheckError)
	{
		if( pSrc == NULL ) return;

		if (pSrc->IsA(SNT_SFUNCCALL))
		{
			SFunction* function = dynamic_cast<SFunction*>(pSrc->Definition());
			if (function == nullptr) return;

			static vector<SVariable*> sFormalParams(32);
			sFormalParams.clear();
			function->GetFormalParams(sFormalParams);

			SNode* pSChild = pSrc->FirstChild();
			for (size_t index = 0; index < sFormalParams.size(); ++index)
			{
				SVariable* pParam = sFormalParams[index];
				if (pSChild != NULL)
				{
					wstring srcOptionCode;
					TEE::NodeBase* pSrcTEE = pSChild->TEETmp();
					if (pSrcTEE) pSrcTEE->GetOptionCode(srcOptionCode);

					wstring targetOptionCode;
					TEE::NodeBase* pTargetTEE = pParam->TEETmp();
					if (pTargetTEE) pTargetTEE->GetOptionCode(targetOptionCode);

					if (srcOptionCode != targetOptionCode)
					{
						if (bCheckError)
						{
							pSChild->AddErrorFlag(VS_PARAM_TYPE_ERR);
							pSrc->AddErrorFlag(VS_PARAM_TYPE_ERR);
						}
						else
						{
							if (SNode* pNewNode = BuildParameter(pTargetTEE))
							{
								SNode::Swap(pSChild, pNewNode);
								pSChild->Remove();
								pSChild = pNewNode;
							}
						}
					}
					pSChild = pSChild->Next();
				}
				else
				{
					if (bCheckError)
						pSrc->AddErrorFlag(VS_PARAM_NUMBER_ERR);
					else
						BuildParameter(pParam->TEETmp(), wstring(), pSrc);
				}
			}

			if (pSChild && !bCheckError)
				pSChild->Remove(true);
		}
		else if (pSrc->IsA(SNT_EVENT) || pSrc->IsA(SNT_CONDITION) || pSrc->IsA(SNT_TFUNCCALL))
		{
			TEE::NodeBase* pTemplate = pSrc->TEETmp();
			if (pTemplate == nullptr) return;

			SNode* pSChild = pSrc->FirstChild();
			for (TEE::NodeBase* pParamTEE = pTemplate->FirstChild(); pParamTEE; pParamTEE = pParamTEE->Next())
			{
				if (!pParamTEE->IsA(TEE::NT_PARAMETER))
					continue;

				if (pSChild != NULL)
				{
					wstring srcOptionCode;
					TEE::NodeBase* pSrcTEE = pSChild->TEETmp();
					if (pSrcTEE) pSrcTEE->GetOptionCode(srcOptionCode);

					wstring targetOptionCode;
					pParamTEE->GetOptionCode(targetOptionCode);

					if (srcOptionCode != targetOptionCode)
					{
						if (bCheckError)
						{
							pSChild->AddErrorFlag(VS_PARAM_TYPE_ERR);
							pSrc->AddErrorFlag(VS_PARAM_TYPE_ERR);
						}
						else
						{
							TEE::NodeBase* pOption = TEE::GTEEMgr->FindOptionFromCode(pParamTEE->m_DisplayName);
							if (SNode* pNewNode = BuildParameter(pOption))
							{
								SNode::Swap(pSChild, pNewNode);
								pSChild->Remove();
								pSChild = pNewNode;
							}
						}
					}
					pSChild = pSChild->Next();
				}
				else
				{
					if (bCheckError)
					{
						pSrc->AddErrorFlag(VS_PARAM_NUMBER_ERR);
					}
					else
					{
						TEE::NodeBase* pOption = TEE::GTEEMgr->FindOptionFromCode(pParamTEE->m_DisplayName);
						BuildParameter(pOption, wstring(), pSrc);
					}
				}
			}

			if (pSChild && !bCheckError) pSChild->Remove(true);
		}
	}

	void	SpaceManager::CheckParametersValidate( SNode* pSrc )
	{
		if( pSrc == NULL ) return;

		assert( pSrc->GetErrorMask() == VS_OK );

		for ( SNode* pChild = pSrc->FirstChild(); pChild != NULL; pChild = pChild->Next() )
		{
			if( pChild->ClassType() == SNT_PARAMETER )
			{
				pSrc->AddErrorFlag( VS_PARAM_PENDING );
				break;
			}
		}
		if( pSrc->TEETmp() == NULL )
		{
			pSrc->AddErrorFlag( VS_TEETEMP_ERR );
		}
		else
		{
			VerifyParameters(pSrc, true);
		}
	}

}


