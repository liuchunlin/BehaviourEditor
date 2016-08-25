#pragma once

#include "SNode.h"
#include "Vek/Base/ObjLink.h"

extern wstring FunctionNameSeperator;
extern wstring SFuncTag;

namespace Space
{
	class SVariable;
	class SFunction : public SNode
	{
		DECLARE_BLOCK_ALLOCATE
	public:
		SFunction();
		~SFunction();
		virtual SPACE_NODE_TYPE CommonParentType() const { return SNT_GROUP; }
		virtual TiXmlElement*	ToXMLElement();
		virtual	bool			FromXMLElement(const TiXmlElement* pXMLElement);
		virtual wstring			GetReadText() const;
		virtual ErrorMask		CheckError();
		virtual TEE::NodeBase *	TEETmp() const;

		virtual	bool			Cutable() { return true; }
		virtual	bool			Copyable() { return true; }
		virtual	bool			Pasteable(const SNode* pSrc);
		virtual bool			UniqueName() const { return true; }
		virtual bool			NewActionable() { return true; }

		TEE::NodeBase*			GetReturnTEE();
		void					GetFormalParams(vector<SVariable*>& outParams);
	public:
		wstring m_desc;
	private:
		Vek::ObjLink<SFunction> m_objlink;
	};

	//-------------------------------------------------------------------------------------
	class SParamRoot : public SNode
	{
		DECLARE_BLOCK_ALLOCATE
	public:
		SParamRoot();
		virtual TiXmlElement*	ToXMLElement();
		virtual	bool			FromXMLElement(const TiXmlElement* pXMLElement);

		//Menu
		virtual	bool			Pasteable(const SNode* pSrc);
		virtual bool			Deleteable() { return false; }
		virtual bool			Disableable() { return false; }
	};

	//-------------------------------------------------------------------------------------
	class SReturnType : public SNode
	{
		DECLARE_BLOCK_ALLOCATE
	public:
		SReturnType();
		~SReturnType();
		virtual TiXmlElement*	ToXMLElement();
		virtual	bool			FromXMLElement(const TiXmlElement* pXMLElement);
		virtual void			TEETmp(TEE::NodeBase* pTNode);
		using					SNode::TEETmp;
		virtual wstring			GetReadText() const;
		virtual ErrorMask		CheckError();

		virtual	bool			Pasteable(const SNode* pSrc) { return false; }
		virtual bool			Deleteable() { return false; }
		virtual bool			Disableable() { return false; }
	};

	//-------------------------------------------------------------------------------------
	class SReturn : public SNode
	{
		DECLARE_BLOCK_ALLOCATE
	public:
		SReturn();
		virtual	bool			FromXMLElement(const TiXmlElement* pXMLElement);
		virtual SPACE_NODE_TYPE CommonParentType()const { return SNT_ACTIONROOT; }
		virtual	bool			GetContextView(ContextView& cnxt);
		virtual	void			GetContextViews(vector<ContextView>& cnxts);
		virtual wstring			GetReadText() const;
		virtual ErrorMask		CheckError();
		virtual bool			GetLuaCode(wstring& luaCode, int depth);

		virtual bool			Pasteable(const SNode* pSrc);
		virtual	bool			Cutable() { return true; }
		virtual	bool			Copyable() { return true; }
		virtual bool			NewActionable() { return true; }
	private:
		void					VerifyParameter(bool bCheckError);
	};

}