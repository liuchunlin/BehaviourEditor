#include "SCondition.h"
#include "NodeVerify.h"
#include "TEEManager.h"
#include "SpaceManager.h"
using namespace Space;

//////////////////////////////////////////////////////////////////////////
//SConditionRoot
IMPLEMENT_BLOCK_ALLOCATE(SConditionRoot, 32)
SConditionRoot::SConditionRoot()
{
	ClassType(SNT_CONDITIONROOT);
}

TiXmlElement*	SConditionRoot::ToXMLElement()
{
	TiXmlElement *pXmlElement = SNode::ToXMLElement();
	return pXmlElement;
}
bool	SConditionRoot::FromXMLElement( const TiXmlElement* pXMLElement )
{
	SNode::FromXMLElement( pXMLElement );

	return true;
}
bool	SConditionRoot::Pasteable( const SNode* pSrc )
{
	if( pSrc == NULL )
		return false;

	if( pSrc->ClassType() == SNT_EVENT || pSrc->ClassType() == SNT_CONDITION || pSrc->ClassType() == SNT_TFUNCCALL  )
		return true;

	return false;
}
bool	SConditionRoot::Deleteable()
{
	return false;
}
bool	SConditionRoot::Permitable()
{
	return false;
}
//////////////////////////////////////////////////////////////////////////
//SCondition
IMPLEMENT_BLOCK_ALLOCATE(SCondition, 64)
SCondition::SCondition()
{
	ClassType(SNT_CONDITION);
}

TiXmlElement*	SCondition::ToXMLElement()
{
	TiXmlElement *pXmlElement = SNode::ToXMLElement();
	pXmlElement->SetAttribute( "work", Working() );

	return pXmlElement;
}
bool	SCondition::FromXMLElement( const TiXmlElement* pXMLElement )
{
	SNode::FromXMLElement( pXMLElement );

	TEE::NodeBase* pTmpNode = TEE::GTEEMgr->FindConditionFromCode(Name());
	assert( pTmpNode != NULL );
	TEETmp( pTmpNode );

	int bWork=1;
	pXMLElement->Attribute( "work", &bWork );
	Working( bWork != 0 );

	return true;
}
bool	SCondition::PostXMLLoaded()
{
	Space::GSpaceMgr->VerifyParameters( this );
	return true;
}

ErrorMask	SCondition::CheckError()
{
	Space::GSpaceMgr->CheckParametersValidate( this );
	return GetErrorMask();
}
wstring	SCondition::GetReadText() const
{
	if( TEETmp() == NULL || TEETmp()->m_tag.empty() )
		return SNode::GetReadText();

	return _T("(") + TEETmp()->m_tag + _T(" - ") + SNode::GetReadText() + _T(")");
}

bool	SCondition::Pasteable( const SNode* pSrc )
{
	if( pSrc == NULL )
		return false;

	if( pSrc->ClassType() == SNT_EVENT || pSrc->ClassType() == SNT_CONDITION || pSrc->ClassType() == SNT_TFUNCCALL  )
		return true;

	return false;
}
bool	SCondition::Cutable()
{
	return true;
}
bool	SCondition::Copyable()
{
	return true;
}
bool	SCondition::GetContextView( ContextView& cnxt )
{
	cnxt.pNode = this;
	cnxt.bValid = GetErrorMask() == VS_OK;
	cnxt.bPreferences = true;
	cnxt.text = GetReadText();

	if( cnxt.text.empty() )
	{
		cnxt.text = _T("Condition");
		cnxt.bValid = false;
	}
	else
	{
		cnxt.text = _T("(") + cnxt.text + _T(")") ;
	}

	return true;
}
bool	SCondition::GetLuaCode( wstring& luaCode, int depth )
{
	if( !Working() )
		return false;
	if( Name() == _T("StringCompare") || Name() == _T("NumberCompare") )
	{
		for ( SNode* pChild = FirstChild(); pChild != NULL; pChild = pChild->Next() )
		{
			wstring childCode;
			if( pChild->GetLuaCode(childCode, 0) )
			{
				luaCode += childCode;
				if( pChild->Next() )
					luaCode += _T(" ");
			}
		}
	}
	else if( Name() == _T("and") || Name() == _T("or"))
	{
		wstring childCode;
		if( FirstChild()->GetLuaCode(childCode, 0) )
		{
			luaCode += childCode;
			luaCode += _T(" ");
			luaCode += Name();
			luaCode += _T(" ");
		}
		childCode.clear();
		if( FirstChild()->Next()->GetLuaCode(childCode, 0) )
		{
			luaCode += childCode;
			if( FirstChild()->Next()->Next() )
				luaCode += _T(" ");
		}
	}
	else
	{
		for ( SNode* pChild = FirstChild(); pChild != NULL; pChild = pChild->Next() )
		{
			wstring childCode;
			if( pChild->GetLuaCode(childCode, depth+1) )
			{
				luaCode += childCode;
				if( pChild->Next() )
				{
					luaCode += _T(" ");
				}
			}
		}
	}

	return true;
}
