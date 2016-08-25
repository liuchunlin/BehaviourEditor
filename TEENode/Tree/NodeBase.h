#pragma once
#include <tchar.h>

#include <string>
#include <vector>
#include <map>
#include <set>
#include <list>
using namespace std;

#include "tinyXML/tinyxml.h"
#include "UnStringConv.h"
#include "BlockAllocator.h"
#include "Vek/Vek/Base/Tree.h"

#define VOID_TEE TEXT("void")

namespace TEE
{
	enum	VALUE_TYPE		//值类型
	{
		VT_NULL,			//无类型-ERR
		VT_INT,				//数值型
		VT_REAL,			//真值型
		VT_BOOLEAN,			//布尔型
		VT_STRING,			//字符串
		VT_CUSTOM,			//自定义

		VT_END,
	};
	typedef	VALUE_TYPE	RETURN_TYPE;
	const map<VALUE_TYPE, pair<wstring,wstring> >&	GetValueTypeNames();		//StringName, CodeName
	const pair<wstring,wstring>&	GetValueTypeName( VALUE_TYPE eType );
	VALUE_TYPE	GetValueTypeEnum( const wstring& codeType );
	const char* GetValueTypeCodeName(const class NodeBase* pNode);
	bool	IsInternalType( const wstring& type );
	bool	NoGenerateCode( const wstring& type );
	bool	IsInternalEvent( const wstring& type );
	enum	RETURN_DETAIL
	{
		RD_NORMAL,			//一维值
		RD_ARRAY,			//多维值
		RD_CUSTOM,			//自定义

		RD_END,
	};
	const map<RETURN_DETAIL, wstring>&	GetReturnDetailNames();
	const wstring&	GetReturnDetailName( RETURN_DETAIL eType );
	//////////////////////////////////////////////////////////////////////////
	enum	//node error mask
	{
		ERR_OK				= 0x00000000,
		ERR_INVALID_FORMAT	= 0x00000001,
		ERR_OUT_OF_DATE		= 0x00000002,
		ERR_REPETITION_NAME	= 0x00000004,
		ERR_EVERYTHING		= 0xFFFFFFFF,
	};
	typedef	DWORD	ErrorMask;
	enum	NODE_TYPE
	{
		NT_NULL,
		NT_ROOT,
		NT_INCLUDEROOT,
		NT_INCLUDE,
		NT_EVENTROOT,
		NT_EVENT,
		NT_CONDITIONROOT,
		NT_CONDITION,
		NT_FUNCTIONROOT,
		NT_FUNCTION,
		NT_OPTIONROOT,
		NT_OPTION,
		NT_TAGROOT,
		NT_SPACEROOT,
		NT_ADJUST,
		NT_LABEL,
		NT_PARAMETER,
		NT_CONTEXTPARAM,
		NT_CONTEXTROOT,
		NT_EVENTCODE,
		NT_MSGEVENTCODE,

		NT_END,
	};
	//////////////////////////////////////////////////////////////////////////
	enum RecordColor
	{
		RC_BLACK,
		RC_BLUE,
		RC_RED,
		RC_GREY,
		RC_GREEN,

		RC_END,
	};
	class RichText
	{
	public:
		RichText()
		{
			Clear();
		}
		void			Clear()
		{
			eColor = RC_BLACK;
			content.clear();
			bUnderLine = false;
		}
		RecordColor		eColor;
		wstring			content;
		bool			bUnderLine;
	};

	class NodeBase : public Vek::TreeNode<NodeBase>
	{
	public:
		static TiXmlElement*	CreateNodeXml( NodeBase* pSrc, TiXmlElement* pXmlParent );
		static bool				HasPrevTypeNode( const NodeBase* pSrc, NODE_TYPE eType );
		static bool				HasNextTypeNode( const NodeBase* pSrc, NODE_TYPE eType );
		static ErrorMask		CheckErrorTree(NodeBase* pSrc, bool bRecursively = true);
		static NodeBase*		Clone(const NodeBase* pSrc, NodeBase* pParent = NULL);

	public:
		NodeBase();
		virtual ~NodeBase();
		virtual TiXmlElement*	ToXMLElement();
		virtual	bool			FromXMLElement( const TiXmlElement* pXMLElement );
		virtual NodeBase*		DoClone() const;
		virtual bool			FromString( const wstring& content = _T("") );
		virtual bool			ToRichString( list<TEE::RichText>& records );
		virtual bool			GetNotesViewString( list<TEE::RichText>& records );
		virtual bool			GetNotesCodeString( list<TEE::RichText>& records );
		virtual bool			GetOptionCode(wstring& valueType);
		virtual bool			GetCPPCode(wstring& cppCode) const;

		virtual ErrorMask		CheckError();
		ErrorMask				GetErrorMask() const;
		void					AddErrorFlag(ErrorMask val);
		void					ClearErrorFlag( ErrorMask val );

		//For TriggerEditor
		virtual wstring			GetReadableString() const;
		//////////////////////////////////////////////////////////////////////////
		//Operation
		virtual bool			NewEventable();
		virtual bool			NewConditionable();
		virtual bool			NewFunctionable();
		virtual bool			NewOptionable();
		virtual bool			NewSpaceable();
		virtual bool			Deleteable();
		virtual bool			Propertyable();
		virtual bool			Editable();
		virtual bool			Dragable();
		virtual bool			Includeable();
		virtual bool			Viewable(){ return true; }

		//////////////////////////////////////////////////////////////////////////
		NODE_TYPE				ClassType() const { return m_classType; }
		void					ClassType(NODE_TYPE val) { m_classType = val; }
		bool					IsA(NODE_TYPE val) const { return this != nullptr && m_classType == val; }
		std::string				TypeName();
		wstring					DisplayName( bool bWithTag = false ) const;
		void					DisplayName( const wstring& val ) { m_DisplayName = val; }
		const wstring&			CodeName() const { return m_CodeName; }
		void					CodeName( const wstring& val ) { m_CodeName = val; }
		NodeBase*				Ancestor( NODE_TYPE eType ) const;
		int						ChildCount( NODE_TYPE eType ) const;
		bool					IsVoidFunction() const;

	public:
		wstring					m_DisplayName;
		wstring					m_CodeName;
		wstring					m_tag;
	public:
		NODE_TYPE				m_classType;
		ErrorMask				m_errorMask;
	};


	class RootNode: public NodeBase
	{
		DECLARE_BLOCK_ALLOCATE
	public:
		RootNode();
		virtual TiXmlElement*	ToXMLElement();
		virtual	bool			FromXMLElement( const TiXmlElement* pXMLElement );
		bool					Save();

		virtual bool			NewEventable();
		virtual bool			NewConditionable();
		virtual bool			NewFunctionable();
		virtual bool			NewOptionable();
		virtual bool			NewSpaceable();
		virtual bool			Propertyable();
		virtual bool			Editable(){ return false; }
		virtual bool			Viewable(){ return false; }

	public:
		wstring m_fileName;
	};

	inline bool	GetXMLElementAttribute( const TiXmlElement* pXMLElement, const string& attrName, wstring &result )
	{
		const char* pAttri = pXMLElement->Attribute( attrName.c_str() );
		result = pAttri != NULL ? (TCHAR*)FUTF8ToTCHAR( pAttri ) : _T("");
		return pAttri != NULL;
	}

	template<typename ResultType>
	inline	bool	GetXMLAttributeInfo( const TiXmlElement* pXMLElement, const string& attrName, ResultType &result )
	{
		const std::string* pAttri = pXMLElement->Attribute( attrName, &result );
		return pAttri != NULL;
	}

} 