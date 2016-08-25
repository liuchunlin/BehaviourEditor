#include "NodeFactory.h"
#include "Event.h"
#include "Condition.h"
#include "Function.h"
#include "Option.h"
#include "Label.h"
#include "Parameter.h"
#include "Adjust.h"
#include "Tag.h"
#include "Space.h"
#include "Include.h"
#include "EventCode.h"
#include "TEEManager.h"
#include <regex>

using namespace TEE;

namespace TEE
{
	static const map<wstring, NODE_TYPE>& NodeNameToTypeMap()
	{
		static map<wstring, NODE_TYPE> s_NameToType;
		if( s_NameToType.empty() )
		{
			s_NameToType.insert( make_pair( _T("Root"),			NT_ROOT) );
			s_NameToType.insert( make_pair( _T("IncludeRoot"),	NT_INCLUDEROOT) );
			s_NameToType.insert( make_pair( _T("Include"),		NT_INCLUDE) );
			s_NameToType.insert( make_pair( _T("EventRoot"),		NT_EVENTROOT) );
			s_NameToType.insert( make_pair( _T("Event"),			NT_EVENT) );
			s_NameToType.insert( make_pair( _T("ConditionRoot"),	NT_CONDITIONROOT) );
			s_NameToType.insert( make_pair( _T("Condition"),		NT_CONDITION) );
			s_NameToType.insert( make_pair( _T("FunctionRoot"),	NT_FUNCTIONROOT) );
			s_NameToType.insert( make_pair( _T("OptionRoot"),	NT_OPTIONROOT) );
			s_NameToType.insert( make_pair( _T("Function"),		NT_FUNCTION) );
			s_NameToType.insert( make_pair( _T("Option"),		NT_OPTION) );
			s_NameToType.insert( make_pair( _T("TagRoot"),		NT_TAGROOT) );
			s_NameToType.insert( make_pair( _T("SpaceRoot"),		NT_SPACEROOT) );
			s_NameToType.insert( make_pair( _T("Adjust"),		NT_ADJUST) );
			s_NameToType.insert( make_pair( _T("Label"),			NT_LABEL) );
			s_NameToType.insert( make_pair( _T("Parameter"),		NT_PARAMETER) );
			s_NameToType.insert( make_pair( _T("ContextParam"),	NT_CONTEXTPARAM) );
			s_NameToType.insert( make_pair( _T("ContextRoot"),	NT_CONTEXTROOT) );
			s_NameToType.insert( make_pair( _T("EventCode"),		NT_EVENTCODE) );
			s_NameToType.insert( make_pair( _T("MsgEventCode"),		NT_MSGEVENTCODE) );
		}
		return s_NameToType;
	}

	NodeBase*	NodeFactory::CreateNode( NODE_TYPE eType )
	{
		switch (eType)
		{
		case NT_ROOT:
			return new TEE::RootNode;
		case NT_INCLUDEROOT:
			return new TEE::IncludeRoot;
		case NT_INCLUDE:
			return new TEE::Include;
		case NT_EVENTROOT:
			return new TEE::EventRoot;
		case NT_EVENT:
			return new TEE::Event;
		case NT_CONDITIONROOT:
			return new TEE::ConditionRoot;
		case NT_CONDITION:
			return new TEE::Condition;
		case NT_FUNCTIONROOT:
			return new TEE::FunctionRoot;
		case NT_FUNCTION:
			return new TEE::Function;
		case NT_OPTIONROOT:
			return new TEE::OptionRoot;
		case NT_OPTION:
			return new TEE::Option;
		case NT_ADJUST:
			return new TEE::Adjust;
		case NT_LABEL:
			return new TEE::Label;
		case NT_PARAMETER:
			return new TEE::Parameter;
		case NT_TAGROOT:
			return new TEE::TagRoot;
		case NT_SPACEROOT:
			return new TEE::SpaceRoot;
		case NT_CONTEXTPARAM:
			return new TEE::ContextParam;
		case NT_CONTEXTROOT:
			return new TEE::ContextRoot;
		case NT_EVENTCODE:
			return new TEE::EventCode;
		case NT_MSGEVENTCODE:
			return new TEE::MsgEventCode;
		}

		assert( false );
		return NULL;
	}

	static const map<NODE_TYPE,wstring>& NodeTypeToNameMap()
	{
		static map<NODE_TYPE,wstring> s_TypeToName;
		if (s_TypeToName.empty())
		{
			const map<wstring, NODE_TYPE>& NameMap = NodeNameToTypeMap();
			map<wstring, NODE_TYPE>::const_iterator itr = NameMap.begin();
			for ( ; itr != NameMap.end(); ++itr)
			{
				s_TypeToName[itr->second] = itr->first;
			}
		}
		return s_TypeToName;
	}
	NODE_TYPE NodeFactory::NodeNameToType( const wstring& nodeName )
	{
		const map<wstring, NODE_TYPE>& NameMap = NodeNameToTypeMap();
		map<wstring, NODE_TYPE>::const_iterator itr = NameMap.find( nodeName );
		if( itr != NameMap.end() )
		{
			return itr->second;
		}
		assert( false );
		return NT_NULL;
	}
	const wstring& NodeFactory::NodeTypeToName( NODE_TYPE eType )
	{
		const map<NODE_TYPE,wstring>& TypeMap = NodeTypeToNameMap();
		map<NODE_TYPE,wstring>::const_iterator itr = TypeMap.find(eType);
		if (itr != TypeMap.end())
		{
			return itr->second;
		}
		assert( false );
		static wstring nullString = TEXT("");
		return nullString;
	}

	NodeBase*		NodeFactory::BuildEvent( const wstring& displayName, const wstring& codeName, const wstring& tags, NodeBase* pNode )
	{
		if( pNode == NULL )
			pNode =  CreateNode( TEE::NT_EVENT );

		if( pNode->m_classType != TEE::NT_EVENT )
			return NULL;

		Event* pEvent = (Event*)(pNode);

		pEvent->m_DisplayName = displayName;
		pEvent->m_CodeName = codeName;
		pEvent->m_tag = tags;
		TEE::GTEEMgr->RegisterLoadedNode( pEvent );
		return pNode;
	}
	NodeBase*		NodeFactory::BuildCondition( const wstring& displayName, const wstring& codeName, const wstring& tags, NodeBase* pNode )
	{
		if( pNode == NULL )
			pNode =  CreateNode( TEE::NT_CONDITION );

		if( pNode->m_classType != TEE::NT_CONDITION )
			return NULL;

		Condition* pCondition = (Condition*)(pNode);
		pCondition->m_DisplayName = displayName;
		pCondition->m_CodeName = codeName;
		pCondition->m_tag = tags;
		TEE::GTEEMgr->RegisterLoadedNode( pCondition );
		return pNode;
	}
	NodeBase*		NodeFactory::BuildFunction( const wstring& displayName, const wstring& codeName, const wstring& returnCode, int eAry, const wstring& tags, bool canBeAction, NodeBase* pNode )
	{
		if( pNode == NULL )
			pNode =  CreateNode( TEE::NT_FUNCTION );

		if( pNode->m_classType != TEE::NT_FUNCTION )
			return NULL;

		Function* pFunction = (Function*)(pNode);

		pFunction->m_DisplayName = displayName;
		pFunction->m_CodeName = codeName;
		pFunction->m_ReturnCode = returnCode;
		pFunction->m_tag = tags;
		pFunction->m_eReturnDetail = static_cast<TEE::RETURN_DETAIL>(eAry);
		pFunction->m_canBeAction = canBeAction;
		TEE::GTEEMgr->RegisterLoadedNode( pFunction );
		return pNode;
	}
	NodeBase*		NodeFactory::BuildOption( const wstring& displayName, const wstring& codeName, int eValueType, const wstring& tags, NodeBase* pNode )
	{
		if( pNode == NULL )
			pNode =  CreateNode( TEE::NT_OPTION );

		if( pNode->m_classType != TEE::NT_OPTION )
			return NULL;

		Option* pOption = (Option*)(pNode);
		pOption->m_DisplayName = displayName;
		pOption->m_CodeName = codeName;
		pOption->m_tag = tags;
		pOption->m_eValueType = static_cast<TEE::VALUE_TYPE>(eValueType);
		TEE::GTEEMgr->RegisterLoadedNode( pOption );
		return pNode;
	}
	NodeBase*	NodeFactory::BuildSpace( const wstring& displayName, const wstring& codeName, NodeBase* pNode /*= NULL*/ )
	{
		if( pNode == NULL )
			pNode = CreateNode( TEE::NT_SPACEROOT );

		if( pNode->m_classType != TEE::NT_SPACEROOT )
			return NULL;

		SpaceRoot* pSpace = (SpaceRoot*)(pNode);
		pSpace->m_DisplayName = displayName;
		pSpace->m_CodeName = codeName;
		TEE::GTEEMgr->RegisterLoadedNode( pSpace );
		return pSpace;
	}

	NodeBase*	NodeFactory::BuildRoot( const wstring& displayName, const wstring& codeName, NodeBase* pNode /*= NULL*/ )
	{
		if( pNode == NULL )
			pNode =  CreateNode( TEE::NT_ROOT );

		if( pNode->m_classType != TEE::NT_ROOT )
			return NULL;

		RootNode* pRoot = (RootNode*)(pNode);
		pRoot->m_DisplayName = displayName;
		pRoot->m_CodeName = codeName;
		TEE::GTEEMgr->RegisterLoadedNode( pRoot );
		return pNode;
	}
	//////////////////////////////////////////////////////////////////////////
	NodeBase*		NodeFactory::BuildNormalChild( const wstring& content, NodeBase* pParent )
	{
		assert( pParent != NULL );

		wstring ParamsContent;
		wstring ContextContent;
		wstring EventCodeContent;
		wstring MsgEventCodeContent;

		wsmatch mcContext;
		wregex reContext(_T("^\\s*Context\\s*\\=\\s*\\("), regex_constants::icase);
		bool bHasContext = regex_search(content, mcContext, reContext);
		if (bHasContext)
		{
			wstring ContextSuffix = mcContext.suffix();
			ContextContent = ContextSuffix.substr(0, ContextSuffix.find_first_of(_T(')')));
		}
		wstring ContextPrefix = bHasContext ? mcContext.prefix() : content;

		wsmatch mcEventCode;
		wregex reEventCode(_T("^\\s*EventCode\\s*\\=\\s*\\{"), regex_constants::icase);
		bool bHasEventCode = regex_search(content, mcEventCode, reEventCode);
		wsmatch mcMsgEventCode;
		wregex reMsgEventCode(_T("^\\s*MsgEventCode\\s*\\=\\s*\\{"), regex_constants::icase);
		bool bHasMsgEventCode = regex_search(content, mcMsgEventCode, reMsgEventCode);

		wstring EventCodePrefix = bHasEventCode ? mcEventCode.prefix() : content;
		wstring MsgEventCodePrefix = bHasMsgEventCode ? mcMsgEventCode.prefix() : content;
		if (bHasEventCode)
		{
			size_t EndPos = MsgEventCodePrefix.length() > EventCodePrefix.length() ? MsgEventCodePrefix.length() : content.length();
			EndPos = content.find_last_of(_T('}'), EndPos);
			size_t StartPos = content.length() - mcEventCode.suffix().length();
			EventCodeContent = content.substr(StartPos, EndPos - StartPos);
		}
		if (bHasMsgEventCode)
		{
			size_t EndPos = EventCodePrefix.length() > MsgEventCodePrefix.length() ? EventCodePrefix.length() : content.length();
			EndPos = content.find_last_of(_T('}'), EndPos);
			size_t StartPos = content.length() - mcMsgEventCode.suffix().length();
			MsgEventCodeContent = content.substr(StartPos, EndPos - StartPos);
		}

		ParamsContent = content.substr(0, min(ContextPrefix.length(), min(EventCodePrefix.length(), MsgEventCodePrefix.length())));

		bool bValid = true;

		// Parameter parse
		int iBegin = ParamsContent.find_first_of( _T("<") );
		int iEnd = ParamsContent.find_first_of( _T(">") );
		while ( iBegin != -1 && iEnd != -1 && iBegin <= iEnd )
		{
			wstring preStr = ParamsContent.substr( 0, iBegin );
			if ( !preStr.empty() )
			{
				Label* pNode = (Label*)TEE::GNodeFactory.CreateNode( NT_LABEL );
				pNode->m_strLabel = preStr;
				pParent->AddChild(pNode);
			}

			//Format:<Option paraName = defaultValue >
			static wregex subExpression(_T("^(\\s+){0,}([a-zA-Z0-9_]+)(\\s+)([a-zA-Z_]+)([a-zA-Z0-9_]){0,}(\\s+){0,}$"));
			static wregex subExpressionDefault(_T("^(\\s+){0,}([a-zA-Z0-9_]+)(\\s+)([a-zA-Z_]+)([a-zA-Z0-9_]){0,}(\\s+){0,}(=)(\\s+){0,}(([-]{0,1}(\\d+)|[-]{0,1}(\\d+).(\\d){0,})|(([\\w]+)|([\\+\\-\\*\\/])))(\\s+){0,}$"));//(\\s+){0,}([\\w]+)(\\s+){0,}
			wstring middleStr = ParamsContent.substr( iBegin+1, iEnd-iBegin-1 );
			wcmatch what;
			const wchar_t* response = (middleStr.c_str());
			if( regex_match( response, what, subExpression ) || regex_match( response, what, subExpressionDefault ) )
			{
				wregex re(_T("\\s+|=+"));
				wsregex_token_iterator iter( middleStr.begin(), middleStr.end(), re, -1 );
				wsregex_token_iterator end;

				vector<wstring>	paramFormat;
				while( iter != end )
				{
					if( (*iter) != _T("") )
						paramFormat.push_back( *iter ); 
					++iter;
				}
				assert( paramFormat.size() >= 2 );
				if( paramFormat.size() >= 2 )
				{
					Parameter* pNode = (Parameter*)TEE::GNodeFactory.CreateNode( NT_PARAMETER );
					pNode->m_DisplayName = paramFormat[0];
					pNode->m_CodeName = paramFormat[1];
					pNode->m_DefaultValue = paramFormat.size() > 2 ? paramFormat[2] : _T("");
					TEE::GTEEMgr->RegisterLoadedNode( pNode );
					pParent->AddChild(pNode);
				}
			}
			else
			{
				Parameter* pNode = (Parameter*)TEE::GNodeFactory.CreateNode( NT_PARAMETER );
				pNode->m_DisplayName = ParamsContent.substr( iBegin+1, iEnd-iBegin-1 );
				bValid = false;
				pParent->AddChild(pNode);
			}

			ParamsContent.erase( 0, iEnd+1 );

			iBegin = ParamsContent.find_first_of( _T("<") );
			iEnd = ParamsContent.find_first_of( _T(">") );
		}

		if( !ParamsContent.empty() )
		{
			Label* pNode = (Label*)TEE::GNodeFactory.CreateNode( NT_LABEL );
			pNode->m_strLabel = ParamsContent;
			pParent->AddChild(pNode);
		}

		// context parameter parse
		if (ContextContent.length() > 0)
		{
			ContextRoot* pCtxRoot = (ContextRoot*)TEE::GNodeFactory.CreateNode( NT_CONTEXTROOT );
			pParent->AddChild(pCtxRoot);

			wregex reParam(_T("\\s*[a-zA-Z0-9_]+\\s+[a-zA-Z0-9_]+\\s*"));
			wsregex_token_iterator paramIt(ContextContent.begin(), ContextContent.end(), reParam, 0);
			wsregex_token_iterator end;
			while(paramIt != end)
			{
				wstring strParam = *paramIt;
				wregex re(_T("\\s+"));
				wsregex_token_iterator iter( strParam.begin(), strParam.end(), re, -1 );
				wsregex_token_iterator end;
				vector<wstring>	paramFormat;
				while( iter != end )
				{
					if( (*iter) != _T("") )
						paramFormat.push_back( *iter ); 
					++iter;
				}
				if( paramFormat.size() >= 2 )
				{
					ContextParam* pNode = (ContextParam*)TEE::GNodeFactory.CreateNode( NT_CONTEXTPARAM );
					pNode->m_DisplayName = paramFormat[0];
					pNode->m_CodeName = paramFormat[1];
					pCtxRoot->AddChild(pNode);
				}

				++paramIt;
			}
		}

		// event code
		if (EventCodeContent.length() > 0)
		{
			EventCode* pNode = (EventCode*)TEE::GNodeFactory.CreateNode( NT_EVENTCODE );
			pNode->m_strCode = EventCodeContent;
			pParent->AddChild(pNode);
		}

		// msg event code
		if (MsgEventCodeContent.length() > 0)
		{
			MsgEventCode* pNode = (MsgEventCode*)TEE::GNodeFactory.CreateNode( NT_MSGEVENTCODE );
			pNode->m_strCode = MsgEventCodeContent;
			pParent->AddChild(pNode);
		}

		return pParent;
	}
	NodeBase*	NodeFactory::BuildTagChild( const wstring& content, NodeBase* pParent )
	{
		assert( pParent != NULL );
		if( pParent == NULL )
		{
			return NULL;
		}
		wstring newContent = content;

		vector<wstring>	records;
		int iRecord = newContent.find( _T("\n") ); 
		while ( iRecord != -1 )
		{
			wstring record = newContent.substr( 0, iRecord );
			records.push_back( record );

			newContent = newContent.substr( iRecord+1, newContent.size() );
			iRecord = newContent.find( _T("\n") ); 
		}
		if( !newContent.empty() )
		{
			records.push_back( newContent );
		}

		//½âÎörecords
		vector<wstring>::iterator iter(	records.begin() );
		for( ; iter != records.end(); ++iter )
		{
			const wstring& Cnt = (*iter);
			Label* pLabel = (Label*)GNodeFactory.CreateNode( NT_LABEL );
			pLabel->m_strLabel = Cnt;

			pParent->AddChild(pLabel);
		}
		return pParent;
	}
	NodeBase*	NodeFactory::BuildIncludesChild( const set<wstring>& contents, NodeBase* pParent )
	{
		assert( pParent != NULL );
		if( pParent == NULL )
		{
			return NULL;
		}
		pParent->RemoveChildren();
		set<wstring>::const_iterator iter( contents.begin() );
		for( ; iter != contents.end(); ++iter )
		{
			const wstring& Cnt = (*iter);
			Include* pNode = (Include*)GNodeFactory.CreateNode( NT_INCLUDE );
			pNode->DisplayName( Cnt );

			pParent->AddChild(pNode);
		}
		return pParent;
	}
	NodeBase*	NodeFactory::BuildOptionChild( const wstring& content, NodeBase* pParent )
	{
		if (pParent == NULL)
			return NULL;
		wstring newContent = content;
		if( newContent.empty() )
			return pParent;

		wregex re(_T("\\n+"));
		wsregex_token_iterator iter_( newContent.begin(), newContent.end(), re, -1 );
		wsregex_token_iterator end;

		vector<wstring>	records;
		while( iter_ != end )
		{
			if( (*iter_) != _T("") )
				records.push_back( *iter_ ); 
			++iter_;
		}

		//½âÎörecords
		bool bValidAdjust = true;
		vector<wstring>::iterator iter(	records.begin() );
		for( ; iter != records.end(); ++iter )
		{
			const wstring& adjustCnt = (*iter);
			wstring key, value;
			int iPos = adjustCnt.find( _T("=") );
			bool adjustvalid = true;
			if( iPos == -1 )
			{
				key = adjustCnt;
				value = _T("");
				adjustvalid = false;
			}
			else
			{
				key = adjustCnt.substr( 0, iPos );
				value = adjustCnt.substr( iPos+1, adjustCnt.size() );
			}
			Adjust* pAdjust = (Adjust*)GNodeFactory.CreateNode( NT_ADJUST );

			pAdjust->DisplayName( key );
			pAdjust->CodeName( value );
			//pAdjust->CodeName(pParent->CodeName());
			//pAdjust->DisplayName(pParent->DisplayName());
			TEE::GTEEMgr->RegisterLoadedNode( pAdjust );

			pParent->AddChild(pAdjust);
		}

		return pParent;
	}

}
