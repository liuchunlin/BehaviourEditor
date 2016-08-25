#include "NodeManager.h"
#include "PropertyDialogs.h"
#include "NodeFactory.h"
#include "Function.h"
#include "Option.h"
#include "Space.h"

#include "IncludeDialogs.h"
#include "TEEManager.h"

#include "GWIDs.h"

NodeManager*	GNodeMgr = NodeManager::GetInstance();

NodeManager::NodeManager()
{

}
TEE::NodeBase*	NodeManager::OnNodeProperty( TEE::NodeBase* pParent, TEE::NODE_TYPE eType, TEE::NodeBase* pNode )
{
	TEE::NodeBase* pNewNode = NULL;
	wxString displayName, codeName, tags;
	if( pNode != NULL )
	{
		displayName = pNode->m_DisplayName.c_str(); 
		codeName = pNode->m_CodeName.c_str();
		tags = pNode->m_tag.c_str();
		assert( eType == pNode->m_classType );
		eType = pNode->m_classType;
	}

	if( eType == TEE::NT_EVENT )
	{
		EventPropertyDialog dialog( NULL, pNode != NULL ? pNode->m_DisplayName : _T("新事件"), pNode );
		if( !dialog.DoModal( displayName, codeName, tags ) )
			return NULL;
		pNewNode = TEE::GNodeFactory.BuildEvent( displayName.wc_str(), codeName.wc_str(), tags.wc_str(), pNode );
	}
	else if( eType == TEE::NT_CONDITION )
	{
		ConditionPropertyDialog dialog( NULL, pNode != NULL ? pNode->m_DisplayName : _T("新条件"), pNode );
		if( !dialog.DoModal( displayName, codeName, tags ) )
			return NULL;
		pNewNode = TEE::GNodeFactory.BuildCondition( displayName.wc_str(), codeName.wc_str(), tags.wc_str(), pNode );
	}
	else if( eType == TEE::NT_FUNCTION )
	{
		FunctionPropertyDialog dialog( NULL, pNode != NULL ? pNode->m_DisplayName : _T("新功能"), pNode );
		wxString returnType;
		int eReturnDetail = TEE::RD_NORMAL;
		bool canBeAction = false;
		TEE::Function* pFunction = static_cast<TEE::Function*>(pNode);
		if( pFunction != NULL )
		{
			returnType = pFunction->m_ReturnCode;
			eReturnDetail = pFunction->m_eReturnDetail;
			canBeAction = pFunction->m_canBeAction;
		}
		if( !dialog.DoModal( displayName, codeName, returnType, eReturnDetail, tags, canBeAction ) )
			return NULL;

		pNewNode = TEE::GNodeFactory.BuildFunction( displayName.wc_str(), codeName.wc_str(), returnType.wc_str(), eReturnDetail, tags.wc_str(), canBeAction, pNode );
	}
	else if( eType == TEE::NT_OPTION )
	{
		OptionPropertyDialog dialog( NULL, pNode != NULL ? pNode->m_DisplayName : _T("新选项"), pNode  );
		int eValueType = 0;
		TEE::Option* pOption = static_cast<TEE::Option*>(pNode);
		if( pOption != NULL )
		{
			eValueType = pOption->m_eValueType;
		}
		if( !dialog.DoModal( displayName, codeName, eValueType, tags ) )
			return NULL;
		pNewNode = TEE::GNodeFactory.BuildOption( displayName.wc_str(), codeName.wc_str(), eValueType, tags.wc_str(), pNode );
	}
	else if( eType == TEE::NT_SPACEROOT )
	{
		SpacePropertyDialog dialog( NULL, pNode != NULL ? pNode->m_DisplayName : _T("新空间"), pNode );
		int eValueType = 0;
		TEE::SpaceRoot* pSpace = static_cast<TEE::SpaceRoot*>(pNode);
		if( !dialog.DoModal( displayName, codeName ) )
			return NULL;
		pNewNode = TEE::GNodeFactory.BuildSpace( displayName.wc_str(), codeName.wc_str(), pNode );
	}
	else if( eType == TEE::NT_ROOT )
	{
		RootPropertyDialog dialog( NULL, pNode != NULL ? pNode->m_DisplayName : _T("TEE Root"), pNode );
		if( !dialog.DoModal( displayName, codeName, tags ) )
			return NULL;
		pNewNode = TEE::GNodeFactory.BuildRoot( displayName.wc_str(), codeName.wc_str(), pNode );
	}
	else if( eType == TEE::NT_INCLUDEROOT )
	{
		set<wstring> includes =	TEE::GTEEMgr->FindIncludes( pNode );
		IncludeDialog dialog( includes );
		if( dialog.ShowModal() == CTRL_TEE_INCLUDE_OK )
		{
			pNewNode = TEE::GNodeFactory.BuildIncludesChild( dialog.GetIncludes(), pNode );
		}
	}
	else
	{
		return NULL;
		assert( false );
	}
	if( pParent != NULL && pNewNode != NULL && pNewNode != pNode && pNode == NULL )
	{
		pParent->AddChild(pNewNode);
	}
	return pNewNode;
}