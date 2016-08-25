#pragma once

#include "SNode.h"
#include "NodeBase.h"

namespace Space
{
	class SAdjust : public SNode
	{
		DECLARE_BLOCK_ALLOCATE
	public:
		SAdjust();
		virtual TiXmlElement*	ToXMLElement();
		virtual	bool			FromXMLElement( const TiXmlElement* pXMLElement );
		virtual void			CopyFrom(const SNode* src);
		virtual void			TEETmp(TEE::NodeBase* pTNode);
		using SNode::TEETmp;
		virtual	bool			GetContextView( ContextView& cnxt );
		virtual wstring			GetReadText() const;
		virtual ErrorMask		CheckError();
		virtual bool			GetLuaCode( wstring& luaCode, int depth );
		virtual	bool			GetRealParam( wstring& realParam ) const;
		virtual void			FixTEETmp();

		virtual bool			Paramerterable(){ return true; }
	public:
		wstring					m_optionName;
	};
}