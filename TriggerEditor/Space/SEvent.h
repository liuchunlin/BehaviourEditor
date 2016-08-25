#pragma once

#include "SNode.h"

namespace Space
{
	class SEventRoot: public SNode
	{
		DECLARE_BLOCK_ALLOCATE
	public:
		SEventRoot();
		virtual TiXmlElement*	ToXMLElement();
		virtual	bool			FromXMLElement( const TiXmlElement* pXMLElement );
		virtual bool			IsStatementRoot(){ return true; }

		//Menu
		virtual	bool			Pasteable( const SNode* pSrc );
		virtual bool			Deleteable();
		virtual bool			Permitable();
		virtual bool			Disableable(){ return false; }
	};

	class SEvent: public SNode
	{
		DECLARE_BLOCK_ALLOCATE
	public:
		SEvent();
		virtual TiXmlElement*	ToXMLElement();
		virtual	bool			FromXMLElement( const TiXmlElement* pXMLElement );
		virtual bool			PostXMLLoaded();
		virtual	bool			GetContextView( ContextView& cnxt );
		virtual wstring			GetReadText() const;
		virtual ErrorMask		CheckError();
		virtual SPACE_NODE_TYPE CommonParentType() const { return SNT_EVENTROOT; }

		virtual bool			Pasteable( const SNode* pSrc );
		virtual	bool			Cutable();
		virtual	bool			Copyable();
		virtual bool			Paramerterable(){ return true; }

		virtual bool			GetLuaCode( wstring& luaCode, int depth );
	private:
	};
}