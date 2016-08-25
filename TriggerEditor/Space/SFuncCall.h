#pragma once

#include "SNode.h"
#include "NodeBase.h"

extern wstring SFuncTag;

namespace Space
{
	class STFuncCall: public SNode
	{
		DECLARE_BLOCK_ALLOCATE
	public:
		STFuncCall();
		virtual TiXmlElement*	ToXMLElement();
		virtual	bool			FromXMLElement( const TiXmlElement* pXMLElement );
		virtual bool			PostXMLLoaded();
		virtual	bool			GetContextView( ContextView& cnxt );
		virtual SPACE_NODE_TYPE CommonParentType() const { return SNT_ACTIONROOT; }
		virtual wstring			GetReadText() const;
		virtual ErrorMask		CheckError();
		virtual bool			GetRealParam( wstring& realParam ) const;
		virtual bool			GetLuaCode( wstring& luaCode, int depth );

		virtual bool			Paramerterable(){ return !IsVoidFunctionCall(); }
		virtual bool			Pasteable(const SNode* pSrc);
		virtual	bool			Cutable() { return true; }
		virtual	bool			Copyable() { return true; }
		virtual bool			NewActionable() { return true; }
	};

	//-------------------------------------------------------------------------------------
	class SSFuncCall : public SNode
	{
		DECLARE_BLOCK_ALLOCATE
	public:
		using SNode::TEETmp;
		using SNode::Definition;
		SSFuncCall();
		virtual TiXmlElement*	ToXMLElement();
		virtual	bool			FromXMLElement(const TiXmlElement* pXMLElement);
		virtual bool			PostXMLLoaded();
		virtual TEE::NodeBase *	TEETmp() const;
		virtual void			Definition(SNode* pImpl);
		virtual	bool			GetContextView(ContextView& cnxt);
		virtual void			GetContextViews(vector<ContextView>& cnxts);
		virtual SPACE_NODE_TYPE CommonParentType() const { return SNT_ACTIONROOT; }
		virtual wstring			GetReadText() const;
		virtual ErrorMask		CheckError();
		virtual bool			GetRealParam(wstring& realParam) const;
		virtual bool			GetLuaCode(wstring& luaCode, int depth);

		virtual bool			Paramerterable() { return !IsVoidFunctionCall(); }
		virtual bool			Pasteable(const SNode* pSrc);
		virtual	bool			Cutable() { return true; }
		virtual	bool			Copyable() { return true; }
		virtual bool			NewActionable() { return true; }
	};

}