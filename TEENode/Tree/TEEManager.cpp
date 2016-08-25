#include "TEEManager.h"
#include "NodeFactory.h"
#include "tinyXML/tinyxml.h"
#include "Option.h"
#include "NodeVerify.h"
#include <strsafe.h>
#include "wx/filename.h"
#include <regex>

wchar_t GConfigFileName[MAX_PATH];
wchar_t GTEELibPath[MAX_PATH];

std::wstring MakeValidPathName(const wchar_t* pPathName)
{
	int nStrlen = (int)wcslen(pPathName);
	wchar_t* pStrBuf = new wchar_t[nStrlen+2];
	wcscpy(pStrBuf,pPathName);
	pStrBuf[nStrlen+1] = TEXT('\0');
	while(nStrlen>0)
	{
		if(pStrBuf[nStrlen-1]==TEXT('\t') || pStrBuf[nStrlen-1]==TEXT(' '))
		{
			pStrBuf[nStrlen-1] = TEXT('\0');
			nStrlen--;
		}
		else if(pStrBuf[nStrlen-1]!=TEXT('/') && pStrBuf[nStrlen-1]!=TEXT('\\'))
		{
			pStrBuf[nStrlen] = TEXT('/');
			break;
		}
		else
		{
			break;
		}
	}
	std::wstring ResultPath = pStrBuf;
	delete [] pStrBuf;
	return ResultPath;

}

bool IsValidSymbolName(wstring name)
{
	wregex expression(_T("^([a-zA-Z_]+)([a-zA-Z0-9_]){0,}$"));
	wcmatch what;
	return regex_match(name.c_str(), what, expression);
}

namespace TEE
{
	TEEManager*	GTEEMgr = TEEManager::GetInstance();

	const vector<pair<NODE_TYPE,wstring> >& GetTEERootNodes()
	{
		static vector<pair<NODE_TYPE,wstring> > s_RootNodes;
		if( s_RootNodes.empty() )
		{
			s_RootNodes.push_back( make_pair( NT_INCLUDEROOT,	_T("包含")) );
			s_RootNodes.push_back( make_pair( NT_EVENTROOT,		_T("事件")) );
			s_RootNodes.push_back( make_pair( NT_CONDITIONROOT,	_T("条件")) );
			s_RootNodes.push_back( make_pair( NT_FUNCTIONROOT,	_T("函数")) );
			s_RootNodes.push_back( make_pair( NT_OPTIONROOT,	_T("选项")) );
			s_RootNodes.push_back( make_pair( NT_TAGROOT,		_T("标签")) );
		}
		return s_RootNodes;
	}

	TEEManager::TEEManager()
	{
		m_TEERoot = NULL;
		m_SpaceRoot = NULL;

		// get config file name
		wchar_t CurrentDir[MAX_PATH];
		GetCurrentDirectory(MAX_PATH, CurrentDir);
		swprintf(GConfigFileName, MAX_PATH, _T("%s\\%s"), CurrentDir, _T("Profile.ini"));

		wchar_t TEELibPath[MAX_PATH];
		GetPrivateProfileString( _T("TEELibPath"), _T("path"), _T(""), TEELibPath, sizeof(TEELibPath)/sizeof(TCHAR), GConfigFileName );
		std::wstring strTEELibPath = MakeValidPathName(TEELibPath);
		swprintf(GTEELibPath, sizeof(GTEELibPath)/sizeof(TCHAR), TEXT("%s"), strTEELibPath.c_str());
	}
	TEEManager::~TEEManager()
	{
		Flush();
	}

	void	TEEManager::Flush()
	{
		FlushTEERoot();
		FlushSpaceRoot();
		FlushIncludes();
	}
	void	TEEManager::FlushTEERoot()
	{
		m_RootTEEFileName.clear();
		SAFE_DELETE( m_TEERoot );
	}
	void	TEEManager::FlushSpaceRoot()
	{
		SAFE_DELETE( m_SpaceRoot );
	}
	void	TEEManager::FlushIncludes()
	{
		UnRegisterLoadedNode();
		RegisterHostNode( m_TEERoot );
		map<wstring,NodeBase*>::iterator iter(m_Includes.begin());
		for( iter; iter != m_Includes.end(); ++iter )
		{
			NodeBase* pRoot = (*iter).second;
			SAFE_DELETE( pRoot );
		}
		m_Includes.clear();
		m_InvalidIncludes.clear();
		m_SpaceIncludes.clear();
	}

	bool	TEEManager::SaveNode( const wstring& DstFileName, TEE::NodeBase* pNode )
	{
		TiXmlDocument TiDoc;
		TiXmlHandle TiHandle( &TiDoc );

		bool bSuccess = true;

		TiXmlElement* pRoot = NodeBase::CreateNodeXml( pNode, NULL );

		bSuccess &= pRoot != NULL;

		TiHandle.ToNode()->InsertEndChild( *pRoot );
		bSuccess &= TiDoc.SaveFile( FTCHARToANSI(DstFileName.c_str()) );
		assert( bSuccess != NULL );

		SAFE_DELETE( pRoot );

		return true;
	}
	NodeBase*	TEEManager::LoadNode( const wstring& fileName )
	{
		wstring FileNamePath = GTEELibPath + fileName;
		bool bSuccess = true;
		TiXmlDocument TiDoc( FTCHARToANSI(FileNamePath.c_str()) );
		TiXmlHandle TiHandle( &TiDoc );
		TiXmlBase::SetCondenseWhiteSpace(false);
		bSuccess &= TiDoc.LoadFile( TIXML_ENCODING_UTF8 );
		assert( bSuccess );
		if( !bSuccess )
			return NULL;

		TiXmlElement* pRootProperty = TiHandle.FirstChild( FTCHARToANSI(NodeFactory::NodeTypeToName(NT_ROOT).c_str()) ).ToElement();
		assert( bSuccess &= pRootProperty != NULL );
		if( !bSuccess )
			return NULL;


		RootNode* pRoot = (RootNode*)TEE::GNodeFactory.CreateNode( NT_ROOT );
		pRoot->FromXMLElement( pRootProperty );
		pRoot->m_fileName = FileNamePath;

		return pRoot;
	}
	NodeBase*	TEEManager::LoadTEENode( const wstring& fileName, bool bHostRoot )
	{
		NodeBase* pRoot = LoadNode( fileName );
		if (pRoot == NULL)
		{
			return NULL;
		}

		if( bHostRoot )
		{
			m_RootTEEFileName = fileName;
			SAFE_DELETE( m_TEERoot );
			m_TEERoot = pRoot;
		}
		else
		{
			if ( m_Includes.find(fileName) != m_Includes.end() )
			{
				SAFE_DELETE( m_Includes[fileName] );
				m_Includes.erase( fileName );
			}
			m_Includes[fileName] = pRoot;
		}

		VerifyNodeType verify( TEE::NT_INCLUDEROOT );
		NodeBase* pIncludeRoot = pRoot->FindFirstChild(verify);
		LoadTEEInclude( pIncludeRoot );

		return pRoot;
	}
	void	TEEManager::LoadTEEInclude( TEE::NodeBase* pIncludeRoot )
	{
		if( pIncludeRoot == NULL )
			return;

		vector<NodeBase*> includes;
		pIncludeRoot->FindChildren(VerifyAlwaysOk(), includes);
		vector<NodeBase*>::iterator iter( includes.begin() );
		for( ; iter != includes.end(); ++iter )
		{
			NodeBase* pInclude = (*iter);
			wstring keyName = pInclude->DisplayName();
			if ( m_Includes.find(keyName) == m_Includes.end() && m_RootTEEFileName != keyName )
			{
				NodeBase* pRoot = LoadTEENode( keyName, false );
			}
		}
	}

	NodeBase*	TEEManager::LoadSpaceNode( const wstring& fileName )
	{
		m_SpaceFileName = fileName;
		SAFE_DELETE( m_SpaceRoot );
		bool bSuccess = true;
		TiXmlDocument TiDoc( FTCHARToANSI(fileName.c_str()) );
		TiXmlHandle TiHandle( &TiDoc );
		TiXmlBase::SetCondenseWhiteSpace(false);
		bSuccess &= TiDoc.LoadFile( TIXML_ENCODING_UTF8 );
		assert( bSuccess );
		if( !bSuccess )
			return NULL;

		TiXmlElement* pRootProperty = TiHandle.FirstChild( "RootNode" ).ToElement();
		assert( bSuccess &= pRootProperty != NULL );
		if( !bSuccess )
			return NULL;


		m_SpaceRoot = TEE::GNodeFactory.CreateNode( NT_ROOT );
		m_SpaceRoot->FromXMLElement( pRootProperty );
		assert( m_SpaceRoot != NULL );
		if( m_SpaceRoot == NULL )
			return NULL;

		return m_SpaceRoot;
	}

	void	TEEManager::LoadSpaceIncludes( )
	{
		if( m_SpaceRoot == NULL )
			return;

		vector<TEE::NodeBase*> rootIncludes;
		GetNodesByType(NT_INCLUDE, rootIncludes);

		for( auto iter = rootIncludes.begin(); iter != rootIncludes.end(); ++iter )
		{
			NodeBase* pInclude = (*iter);
			wstring keyName = pInclude->DisplayName();
			if( m_RootTEEFileName != keyName )
			{
				if ( m_Includes.find(keyName) == m_Includes.end() )
				{
					LoadTEENode( keyName, false );
				}
			}
		}

		for( auto iter = rootIncludes.begin(); iter != rootIncludes.end(); ++iter )
		{
			NodeBase* pInclude = (*iter);
			wstring keyName = pInclude->DisplayName();
			if ( m_Includes.find(keyName) != m_Includes.end() )
			{
				NodeBase* pRoot = m_Includes[keyName];
				NodeBase* pRefSpace = pInclude->Parent();
				if( pRoot != NULL && pRefSpace != NULL )
				{
					set<TEE::NodeBase*>	includes;
					FindSubIncludes( pRoot, includes );
					set<TEE::NodeBase*>::const_iterator itInclude=(includes.begin());
					for( ;itInclude != includes.end(); ++itInclude )
					{
						m_SpaceIncludes[pRefSpace].insert(*itInclude);
					}
					m_SpaceIncludes[pRefSpace].insert(pRoot) ;
				}
			}
		}
	}

	NodeBase*	TEEManager::NewSpace( )
	{
		SAFE_DELETE( m_TEERoot );
		m_TEERoot = TEE::GNodeFactory.CreateNode( NT_ROOT );
		return m_TEERoot;
	}
	void	TEEManager::RefreshIncludes( TEE::NodeBase* pIncludeRoot )
	{
		FlushIncludes();
		LoadTEEInclude( pIncludeRoot );
		LoadSpaceIncludes();
	}

// 	//////////////////////////////////////////////////////////////////////////

	void TEEManager::GetNodesByType(NODE_TYPE eNodeType, vector<NodeBase*>& outNodes)
	{
		auto itrTypeNodes = m_LoadedNodes.find(eNodeType);
		if (itrTypeNodes != m_LoadedNodes.end())
		{
			const CodeNameMapType& TypeNodes = itrTypeNodes->second;
			for (CodeNameMapType::const_iterator itrNode = TypeNodes.begin(); itrNode != TypeNodes.end(); ++itrNode)
			{
				outNodes.push_back(itrNode->second);
			}
		}
	}

	NodeBase*	TEEManager::FindOptionFromName(const wstring& displayName)
	{
		NodeTypeMapType::const_iterator itrTypeNodes = m_LoadedNodes.find(NT_OPTION);
		if (itrTypeNodes != m_LoadedNodes.end())
		{
			const CodeNameMapType& TypeNodes = itrTypeNodes->second;
			for ( CodeNameMapType::const_iterator itrNode = TypeNodes.begin(); itrNode != TypeNodes.end(); ++itrNode)
			{
				if (itrNode->second->m_DisplayName == displayName)
				{
					return itrNode->second;
				}
			}
		}
		return NULL;
	}

	NodeBase*	TEEManager::FindNodeFromCode(NODE_TYPE eNodeType, const wstring& codeName)
	{
		NodeTypeMapType::const_iterator itrTypeNodes = m_LoadedNodes.find(eNodeType);
		if (itrTypeNodes != m_LoadedNodes.end())
		{
			const CodeNameMapType& TypeNodes = itrTypeNodes->second;
			CodeNameMapType::const_iterator itrNode = TypeNodes.find(codeName);
			if (itrNode != TypeNodes.end())
			{
				return itrNode->second;
			}
		}
		return NULL;
	}
	NodeBase*	TEEManager::FindOptionFromCode(const wstring& codeName)
	{
		return FindNodeFromCode(NT_OPTION, codeName);
	}

	NodeBase*	TEEManager::FindEventFromCode(const wstring& codeName)
	{
		return FindNodeFromCode(NT_EVENT, codeName);
	}

	NodeBase*	TEEManager::FindFunctionFromCode(const wstring& codeName)
	{
		return FindNodeFromCode(NT_FUNCTION, codeName);
	}

	NodeBase*	TEEManager::FindConditionFromCode(const wstring& codeName)
	{
		return FindNodeFromCode(NT_CONDITION, codeName);
	}

	set<wstring>	TEEManager::FindIncludes( TEE::NodeBase *pIncludeParent )
	{
		set<wstring> includes;
		if( pIncludeParent == NULL )
			return includes;
		NodeBase* pChild = pIncludeParent->FirstChild();
		while( pChild != NULL )
		{
			if( pChild->ClassType() == TEE::NT_INCLUDE )
			{
				includes.insert( pChild->DisplayName() );
			}
			pChild = pChild->Next();
		}
		return includes;
	}
	void	TEEManager::FindSubIncludes( TEE::NodeBase* pRoot, set<NodeBase*>& includes )
	{
		if( pRoot == NULL )
			return;

		vector<NodeBase*> newIncludes;
		pRoot->FindChildren(VerifyNodeType(TEE::NT_INCLUDE), newIncludes, true);

		vector<NodeBase*> requireParses;
		for (auto iter = newIncludes.begin(); iter != newIncludes.end(); ++iter )
		{
			NodeBase* includeNode = (*iter);
			const wstring& name = includeNode->DisplayName();
			if( m_Includes.find(name) != m_Includes.end() )
			{
				set<NodeBase*>::_Pairib result = includes.insert( m_Includes[name] );
				if( result.second )
				{
					requireParses.push_back(m_Includes[name]);
				}
			}
		}
		
		for (auto iter = requireParses.begin(); iter != requireParses.end(); ++iter )
		{
			FindSubIncludes((*iter), includes );
		}
	}


	ErrorMask	TEEManager::CheckTagValidate( const wstring& strTag )
	{
		ErrorMask flag = ERR_OK;
		if (!strTag.empty())
		{
			vector<NodeBase*> TagRoots;
			GetNodesByType(NT_TAGROOT, TagRoots);

			vector<NodeBase*> tags;
			FindChildren(TagRoots, VerifyLabelName(strTag), tags);
			if (tags.empty())
			{
				flag |= (ERR_INVALID_FORMAT);
			}
		}
		return flag;
	}
	bool	TEEManager::CheckEditValueValidate( const NodeBase* pNode, const wstring& value )
	{
		if( pNode == NULL )
			return false;

		if( pNode->ClassType() != TEE::NT_OPTION )
			return false;

		const TEE::Option* pOption = static_cast<const TEE::Option*>(pNode);
		//“真正”的类型。不是延伸出来的同类类型：如数值型的“道具”等
		if( pOption->m_eValueType != TEE::GetValueTypeEnum( pOption->CodeName() ) )
		{
			return false;
		}
		switch( pOption->m_eValueType )
		{
		case VT_INT:
			{
				wregex expression(_T("^[-]{0,1}(\\d+)$"));
				wcmatch what;
				const wchar_t* response = (value.c_str());
				return regex_match( response, what, expression );
			}
			break;
		case VT_REAL:
			{
				wregex subExpression(_T("^[-]{0,1}(\\d+)|[-]{0,1}(\\d+).(\\d){0,}$"));
				wcmatch what;
				const wchar_t* response = (value.c_str());
				return regex_match( response, what, subExpression );
			}
			break;
		case VT_BOOLEAN:
			break;
		case VT_STRING:
			{
				return true;
			}
			break;
		default:
			assert( false );
			break;
		}

		return false;
	}

	ErrorMask	TEEManager::CheckParametersValidate( NodeBase* pParent )
	{
		ErrorMask flag = ERR_OK;
		if( pParent == NULL )
			return flag;

		set<wstring>	ParamNames;
		NodeBase* pChild = pParent->FirstChild();
		while ( pChild != NULL )
		{
			if( pChild->ClassType() == NT_PARAMETER )
			{
				if( ParamNames.find(pChild->m_CodeName) == ParamNames.end() )
				{
					ParamNames.insert( pChild->m_CodeName );
				}
				else
				{
					pChild->AddErrorFlag( ERR_INVALID_FORMAT );
					flag |= ( ERR_INVALID_FORMAT );
				}
			}

			pChild = pChild->Next();
		}
		return flag;
	}
	ErrorMask	TEEManager::CheckContextParamsValidate( NodeBase* pParent )
	{
		ErrorMask flag = ERR_OK;
		if( pParent == NULL )
			return flag;

		set<wstring>	ParamNames;
		NodeBase* pChild = pParent->FirstChild();
		while ( pChild != NULL )
		{
			if( pChild->ClassType() == NT_CONTEXTPARAM )
			{
				if( ParamNames.find(pChild->m_CodeName) == ParamNames.end() )
				{
					ParamNames.insert( pChild->m_CodeName );
				}
				else
				{
					pChild->AddErrorFlag( ERR_INVALID_FORMAT );
					flag |= ( ERR_INVALID_FORMAT );
				}
			}

			pChild = pChild->Next();
		}
		return flag;
	}
	ErrorMask	TEEManager::CheckNameValidate( NodeBase* pNode )
	{
		ErrorMask flag = ERR_OK;
		if( pNode == NULL )
			return flag;

		multimap<wstring, NodeBase*>& nodes = m_LoadedNodes[pNode->ClassType()];
		int count = nodes.count( pNode->CodeName() );
		if( count > 1 )
		{
			flag = ERR_REPETITION_NAME;
		}
		return flag;
	}

	//TODO:不准确的判断：忽略了父包含关系，暂时无所谓
	ErrorMask	TEEManager::CheckIncludeValidate( NodeBase* pNode )
	{
		ErrorMask flag = ERR_OK;
		if( pNode == NULL )
			return flag;

		map<wstring,NodeBase*>::iterator iter( m_Includes.begin() );
		for( ; iter != m_Includes.end(); ++iter )
		{
			const wstring& name = (*iter).first;
			NodeBase* pRoot = (*iter).second;
			if( name == pNode->DisplayName() )
			{
				if( m_InvalidIncludes.find( pRoot ) != m_InvalidIncludes.end() )
				{
					flag = ERR_REPETITION_NAME;
					return flag;
				}
			}
		}
		return flag;
	}

	bool	TEEManager::GetTagViewString( list<TEE::RichText>& records, const wstring& tag )
	{
		RichText record;
		record.content = tag + _T("\n");
		record.eColor = CheckTagValidate(tag) == TEE::ERR_OK ? TEE::RC_BLUE : TEE::RC_RED;
		records.push_back( record );
		return true;
	}
	bool	TEEManager::GetParameterReadName( TEE::NodeBase *pNode, wstring& name )
	{
		if( pNode == NULL )
			return false;
		if( pNode->ClassType() != TEE::NT_PARAMETER )
			return false;

		NodeBase* pOption = GTEEMgr->FindOptionFromCode(pNode->DisplayName());
		if( pOption == NULL )
			return false;

		name = pOption->DisplayName();

		return true;
	}
	void	TEEManager::RegisterLoadedNode( TEE::NodeBase *pNode )
	{
		if( pNode == NULL )
			return;

		multimap<wstring, NodeBase*>& nodes = m_LoadedNodes[pNode->ClassType()];

		switch(pNode->ClassType())
		{
		case TEE::NT_EVENT:
		case TEE::NT_CONDITION:
		case TEE::NT_FUNCTION:
		case TEE::NT_OPTION:
			{
				auto itrSameName = nodes.find(pNode->CodeName());
				if(itrSameName != nodes.end() && itrSameName->second != pNode)
				{
					TCHAR temp[1024];
					StringCchPrintf( temp, 1024, _T("重复的名字%s"), pNode->CodeName().c_str() );
					MessageBox( NULL, temp, _T("ERROR"), 0 );

					NodeBase* pAncestor = pNode->Ancestor( TEE::NT_ROOT );
					if( pAncestor != NULL )
					{
						m_InvalidIncludes.insert( pAncestor );
					}
				}
			}
			break;
		default:
			break;
		}
		nodes.insert( make_pair( pNode->CodeName(), pNode ) );
	}
	void	TEEManager::UnRegisterLoadedNode()
	{
		m_LoadedNodes.clear();
	}
	void	TEEManager::RegisterHostNode( TEE::NodeBase *pHostNode )
	{
		if( pHostNode == NULL )
			return;

		multimap<wstring, NodeBase*>& nodes = m_LoadedNodes[pHostNode->ClassType()];
		nodes.insert( make_pair( pHostNode->CodeName(), pHostNode ) );

		RegisterHostNode( pHostNode->FirstChild() );
		RegisterHostNode( pHostNode->Next() );
	}
	//Space
	vector<wstring>	TEEManager::GetPertainSpaces( const TEE::NodeBase *pNode )
	{
		vector<wstring> spaces;
		if( pNode == NULL )
			return spaces;

		NodeBase* pAncestor = pNode->Ancestor( NT_ROOT );
		if( pAncestor == NULL )
		{
			assert( false );
			return spaces;
		}

		map<NodeBase*,set<NodeBase*> >::iterator iter( m_SpaceIncludes.begin() );
		for( ; iter != m_SpaceIncludes.end(); ++iter )
		{
			NodeBase* pSpace = (*iter).first;
			const set<NodeBase*>& spaceNodes = (*iter).second;
			if( spaceNodes.find(pAncestor) != spaceNodes.end() )
				spaces.push_back( pSpace->CodeName() );
		}
		return spaces;
	}
}


