#pragma once

#include "SNode.h"

namespace Space
{
	class SConditionRoot: public SNode
	{
		DECLARE_BLOCK_ALLOCATE
	public:
		SConditionRoot();
		virtual TiXmlElement*	ToXMLElement();
		virtual	bool			FromXMLElement( const TiXmlElement* pXMLElement );
		virtual bool			IsStatementRoot(){ return true; }

		//Menu
		virtual	bool			Pasteable( const SNode* pSrc );
		virtual bool			Deleteable();
		virtual bool			Permitable();
		virtual bool			Disableable(){ return false; }
	};

	class SCondition: public SNode
	{
		DECLARE_BLOCK_ALLOCATE
	public:
		SCondition();
		virtual TiXmlElement*	ToXMLElement();
		virtual	bool			FromXMLElement( const TiXmlElement* pXMLElement );
		virtual bool			PostXMLLoaded();
		virtual SPACE_NODE_TYPE CommonParentType() const { return SNT_CONDITIONROOT; }
		virtual ErrorMask		CheckError();
		virtual wstring			GetReadText() const;
		virtual bool			GetContextView( ContextView& cnxt );
		virtual bool			GetLuaCode( wstring& luaCode, int depth );

		virtual	bool			Pasteable( const SNode* pSrc );
		virtual	bool			Cutable();
		virtual	bool			Copyable();
		virtual bool			Paramerterable(){ return true; }
	};
}