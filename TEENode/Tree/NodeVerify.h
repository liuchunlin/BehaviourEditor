#pragma once

#include "NodeBase.h"
#include "Label.h"
#include "Function.h"
namespace TEE
{
	struct VerifyAlwaysOk
	{
		bool operator() (NodeBase* pNode) const
		{
			return true;
		}
	};
	struct VerifyCodeName
	{
		VerifyCodeName(const wstring& InCodeName)
			: codeName(InCodeName)
		{
		}
		bool operator() (NodeBase* pNode) const
		{
			if( pNode == NULL )
				return false;
			return pNode->m_CodeName == codeName;
		}
		wstring		codeName;
	};
	struct VerifyDisplayName
	{
		VerifyDisplayName(const wstring& InDisplayName)
			: displayName(InDisplayName)
		{
		}
		bool operator() (NodeBase* pNode) const
		{
			if( pNode == NULL )
				return false;
			return pNode->m_DisplayName == displayName;
		}
		wstring		displayName;
	};
	struct VerifyNodeType
	{
		VerifyNodeType(NODE_TYPE eNodeType)
		{
			eTypes.insert(eNodeType);
		}
		bool operator() (NodeBase* pNode) const
		{
			if( pNode == NULL )
				return false;
			return eTypes.find(pNode->m_classType) != eTypes.end();
		}
		set<NODE_TYPE>		eTypes;
	};
	struct VerifyFunctionReturnType
	{
		bool operator() (NodeBase* pNode) const
		{
			if( pNode == NULL )
				return false;
			if( pNode->ClassType() != NT_FUNCTION )
				return false;
			Function* pFunction = (Function*)pNode;

			return pFunction->m_ReturnCode == ReturnCode;
		}
		wstring					ReturnCode;
	};
	struct VerifyLabelName
	{
		VerifyLabelName(const wstring& InLabelName)
			: labelName(InLabelName)
		{
		}
		bool operator() (NodeBase* pNode) const
		{
			if( pNode == NULL )
				return false;
			if( pNode->m_classType != NT_LABEL )
				return false;
			Label* pLabel = (Label*)pNode;

			return pLabel->m_strLabel == labelName;
		}

		wstring		labelName;
	};

	template<class Pr1, class Pr2>
	struct VerifyAND
	{
		VerifyAND(const Pr1& A, const Pr2& B)
			: pr1(A)
			, pr2(B)
		{
		}
		bool operator() (NodeBase* pNode) const
		{
			return pr1(pNode) && pr2(pNode);
		}

		const Pr1 pr1;
		const Pr2 pr2;
	};

	template<class Pr1, class Pr2>
	struct VerifyOR
	{
		VerifyOR(const Pr1& A, const Pr2& B)
			: pr1(A)
			, pr2(B)
		{
		}
		bool operator() (NodeBase* pNode) const
		{
			return pr1(pNode) || pr2(pNode);
		}

		const Pr1 pr1;
		const Pr2 pr2;
	};

	typedef VerifyAND<VerifyNodeType,VerifyDisplayName> NodeTypeAndDisplayNamePredicate;
	typedef VerifyAND<VerifyNodeType,VerifyCodeName> NodeTypeAndCodeNamePredicate;
}