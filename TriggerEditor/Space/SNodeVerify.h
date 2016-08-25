#pragma once

#include "SNode.h"

namespace Space
{
	//////////////////////////////////////////////////////////////////////////
	struct SVerifyAlwaysOk
	{
		bool operator() ( SNode* pNode ) const
		{
			return true;
		}
	};
	//////////////////////////////////////////////////////////////////////////
	struct SVerifyNodeType
	{
		SVerifyNodeType(SPACE_NODE_TYPE InType)
			: eType(InType)
		{
		}
		bool operator() ( SNode* pNode ) const
		{
			if( pNode == NULL )
				return false;

			return pNode->ClassType() == eType;
		}
		SPACE_NODE_TYPE		eType;
	};
	//////////////////////////////////////////////////////////////////////////
	struct SVerifyNameType
	{
		SVerifyNameType( const wstring& InName = TEXT(""))
			: name(InName)
		{
		}
		bool operator() ( SNode* pNode ) const
		{
			if( pNode == NULL )
				return false;

			return pNode->Name() == name;
		}
		wstring		name;
	};
	struct SVerifyTEEType
	{
		SVerifyTEEType( const wstring& InCodeName = TEXT("") )
			: code(InCodeName)
		{
		}
		bool operator() ( SNode* pNode ) const
		{
			if( pNode == NULL || pNode->TEETmp() == NULL )
				return false;

			return pNode->TEETmp()->CodeName() == code;
		}
		wstring		code;
	};

	template<class Pr1, class Pr2>
	struct SVerifyAND
	{
		SVerifyAND(const Pr1& A, const Pr2& B)
			: pr1(A)
			, pr2(B)
		{
		}
		bool operator() (SNode* pNode) const
		{
			return pr1(pNode) && pr2(pNode);
		}

		const Pr1 pr1;
		const Pr2 pr2;
	};

	template<class Pr1, class Pr2>
	struct SVerifyOR
	{
		SVerifyOR(const Pr1& A, const Pr2& B)
			: pr1(A)
			, pr2(B)
		{
		}
		bool operator() (SNode* pNode) const
		{
			return pr1(pNode) || pr2(pNode);
		}

		const Pr1 pr1;
		const Pr2 pr2;
	};

	typedef SVerifyAND<SVerifyNodeType,SVerifyNameType> SNodeTypeAndNamePredicate;
	typedef SVerifyAND<SVerifyNodeType,SVerifyTEEType> NodeTypeAndTEETypePredicate;

}