#pragma once

#include "SNode.h"
#include "SNodeVerify.h"
#include "Vek/Base/Singleton.h"
#include "Vek/Base/ObjLink.h"
#include "STrigger.h"

namespace TEE
{
	class NodeBase;
}
namespace Space
{
	extern bool IsActionType(SPACE_NODE_TYPE eType);
	extern SNode* BuildStatementSOrT(Vek::TreeNodeBase* base);

	class SpaceManager: public Vek::Singleton<SpaceManager>
	{
		friend class Vek::Singleton<SpaceManager>;
	public:
		static SPACE_NODE_TYPE		NodeNameToType( const wstring& nodeName );
		static const wstring&		NodeTypeToName( SPACE_NODE_TYPE eType );
		static const wstring&		GetTypeBaseName( SPACE_NODE_TYPE eType );
		static const bool			IsInternalType( const wstring& name );
		static SNode*				CreateNode( const wstring& strType );
		static SNode*				CreateNode( SPACE_NODE_TYPE eType );
	public:
		//build
		template<SPACE_NODE_TYPE TYPE>
		SNode*						BuildDefault( TEE::NodeBase* pTemplate = NULL, SNode* pParent = NULL, SNode* pPrev = NULL );
		SNode*						BuildStatement(TEE::NodeBase* pDef, SNode* pParent = NULL, SNode* pPrev = NULL);
		SNode*						BuildStatement(SNode* pDef, SNode* pParent = NULL, SNode* pPrev = NULL);
		void						BuildParameters(TEE::NodeBase* pTemplate, SNode* pParent);
		void						BuildParameters(SNode* pDef, SNode* pParent);
		SNode*						BuildParameter(TEE::NodeBase* pOption, wstring defaultValue = wstring(), SNode* pParent = nullptr);
		SNode*						ReBuildVariableIndex( SNode* pSrc );
		SNode*						ReBuildVariableDefault( SNode* pSrc );
		SNode*						ReBuildVarChoice( SNode* pSrc );
		//Find
		SNode*						ChiefRoot(){ return m_pRoot; }
		SNode*						FindRoot( SPACE_NODE_TYPE eRootType, const SNode* pParent );
		void						FindTriggers(vector<SNode*>& triggers);
		template<class Pr>
		SNode*						FindTrigger( const Pr& pr );
		void						FindFunctions(vector<SNode*>& functions);
		SNode*						FindFunctionFromName(const wstring& name);

		//Name/ID
		wstring						CreateDefaultName( SPACE_NODE_TYPE eType, const wstring& base );
		template<SPACE_NODE_TYPE TYPE>
		wstring						CreateDefaultName( const wstring& base );
		DWORD						CreateTriggerID();
		bool						AddTriggerID( DWORD id );
		void						RemoveTriggerID( DWORD id );
		void						UniqueName( SNode* pSrc );

		//Check
		bool						CheckNameValid( SNode* pSrc, const wstring& newName );
		void						VerifyParameters( SNode* pSrc, bool bCheckError = false );
		void						CheckParametersValidate( SNode* pSrc );

	public:
		//Xml File
		bool						SaveNode( const wstring& DstFileName, SNode* pNode );
		SNode*						LoadNode( const wstring& fileName );
		void						Flush();
		wstring						GetTriggerFileName() { return m_TriggerFileName; }

	private:
		SpaceManager();
		~SpaceManager();
	private:
		wstring						m_TriggerFileName;
		SNode						*m_pRoot;
		set<DWORD>					m_TriggerIDs;
	public:
	};

	extern SpaceManager				*GSpaceMgr;
	extern bool IsNodeInSpace(SNode* pNode);

	template<class Pr>
	SNode*	SpaceManager::FindTrigger(const Pr& pr)
	{
		Vek::ObjLink<STrigger>* link = Vek::ObjLink<STrigger>::Head();
		for (; link; link = link->Next())
		{
			if (pr(link->Obj()) && IsNodeInSpace(link->Obj()))
				return link->Obj();
		}
		return nullptr;
	}

}