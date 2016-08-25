#pragma once

#include "NodeBase.h"
#include <tchar.h>
#include <Vek/Base/Singleton.h>

extern bool IsValidSymbolName(wstring name);

namespace TEE
{
	const vector<pair<NODE_TYPE,wstring> >& GetTEERootNodes();
	const vector<pair<NODE_TYPE,wstring> >& GetSpaceRootNodes();
	class Option;
	class Verify;

	class TEEManager: public Vek::Singleton<TEEManager>
	{
		friend class Vek::Singleton<TEEManager>;
	public:
		typedef multimap<wstring, NodeBase*> CodeNameMapType;
		typedef map<NODE_TYPE, CodeNameMapType > NodeTypeMapType;

		bool				SaveNode( const wstring& DstFileName, TEE::NodeBase* );
		NodeBase*			LoadNode( const wstring& fileName );
		NodeBase*			LoadTEENode( const wstring& fileName, bool bHostRoot );
		NodeBase*			LoadSpaceNode( const wstring& fileName );
		void				RefreshIncludes( TEE::NodeBase* pIncludeRoot );
		void				LoadTEEInclude( TEE::NodeBase* pIncludeRoot );
		void				LoadSpaceIncludes();
		void				Flush();
		void				FlushTEERoot();
		void				FlushSpaceRoot();
		void				FlushIncludes();
		NodeBase*			NewSpace( );
		NodeBase*			TEERoot( ){ return m_TEERoot; }
		NodeBase*			SpaceRoot( ){ return m_SpaceRoot; }

		template<class Pr>
 		void				FindChildren( const vector<NodeBase*>& Parents, const Pr& pr, vector<NodeBase*>& outChildren, bool bRecursive = false );

		template<class Pr>
		void				FindNodesOfType(NODE_TYPE eNodeType, const Pr& pr, vector<NodeBase*>& outNodes);
		void				GetNodesByType(NODE_TYPE eNodeType, vector<NodeBase*>& outNodes);
		NodeBase*			FindNodeFromCode(NODE_TYPE eNodeType, const wstring& codeName);
		NodeBase*			FindOptionFromName(const wstring& displayName);
		NodeBase*			FindOptionFromCode(const wstring& codeName);
		NodeBase*			FindEventFromCode(const wstring& codeName);
		NodeBase*			FindFunctionFromCode(const wstring& codeName);
		NodeBase*			FindConditionFromCode(const wstring& codeName);

		set<wstring>		FindIncludes( TEE::NodeBase *pInclude );
		void				FindSubIncludes( TEE::NodeBase *pRoot, set<NodeBase*>& includes );
		const map<wstring,NodeBase*>& GetIncludes() { return m_Includes; }

		bool				CheckEditValueValidate( const NodeBase* pNode, const wstring& value );
		ErrorMask			CheckTagValidate( const wstring& tag );
		ErrorMask			CheckParametersValidate( NodeBase* pParent );
		ErrorMask			CheckContextParamsValidate( NodeBase* pParent );
		ErrorMask			CheckNameValidate( NodeBase* pNode );
		ErrorMask			CheckIncludeValidate( NodeBase* pNode );
		
		bool				GetTagViewString( list<TEE::RichText>& records, const wstring& tag );
		//
		bool				GetParameterReadName( TEE::NodeBase *pNode, wstring& name );

		//Space
		vector<wstring>		GetPertainSpaces( const TEE::NodeBase *pNode );

		//Loaded
		void				RegisterLoadedNode( TEE::NodeBase *pNode );
		void				UnRegisterLoadedNode();
		void				RegisterHostNode( TEE::NodeBase *pHostNode );

	private:
		TEEManager();
		~TEEManager();

		wstring						m_SpaceFileName;
		wstring						m_RootTEEFileName;
		NodeBase*					m_TEERoot;
		NodeBase*					m_SpaceRoot;
		map<wstring,NodeBase*>		m_Includes;
		set<NodeBase*>				m_InvalidIncludes;
		map<NodeBase*, set<NodeBase*> >	m_SpaceIncludes;
		NodeTypeMapType				m_LoadedNodes;
	};

	template<class Pr>
	void	TEEManager::FindChildren( const vector<NodeBase*>& Parents, const Pr& pr, vector<NodeBase*>& outChildren, bool bRecursive )
	{
		vector<NodeBase*>::const_iterator	iter(Parents.begin());
		for ( ; iter != Parents.end(); ++iter )
		{
			if (NodeBase* pParent = (*iter))
			{
				pParent->FindChildren(pr, outChildren, bRecursive);
			}
		}
	}

	template<class Pr>
	void	TEEManager::FindNodesOfType(NODE_TYPE eNodeType, const Pr& pr, vector<NodeBase*>& outNodes)
	{
		auto itrTypeNodes = m_LoadedNodes.find(eNodeType);
		if (itrTypeNodes != m_LoadedNodes.end())
		{
			const CodeNameMapType& TypeNodes = itrTypeNodes->second;
			for (CodeNameMapType::const_iterator itrNode = TypeNodes.begin(); itrNode != TypeNodes.end(); ++itrNode)
			{
				if (pr(itrNode->second))
					outNodes.push_back(itrNode->second);
			}
		}
	}

	extern TEEManager*	GTEEMgr;
}