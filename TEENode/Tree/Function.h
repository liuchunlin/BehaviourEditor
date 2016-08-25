#pragma once 
#include "NodeBase.h"
#include "wx/string.h"

namespace TEE
{
	class FunctionRoot: public NodeBase
	{
		DECLARE_BLOCK_ALLOCATE
	public:
		FunctionRoot();
		virtual TiXmlElement*	ToXMLElement();
		virtual	bool			FromXMLElement( const TiXmlElement* pXMLElement );

		//Operation
		virtual bool			NewFunctionable();
		virtual bool			Editable(){ return false; }
		virtual bool			Viewable(){ return false; }
	};

	class Function: public NodeBase
	{
		DECLARE_BLOCK_ALLOCATE
	public:
		Function();
		virtual bool			FromString( const wstring& content = _T("") );
		virtual TiXmlElement*	ToXMLElement();
		virtual	bool			FromXMLElement( const TiXmlElement* pXMLElement );
		virtual NodeBase*		DoClone() const;
		virtual bool			GetNotesCodeString(list<TEE::RichText>& records);
		bool					GetNotesViewString( list<TEE::RichText>& records );
		virtual ErrorMask		CheckError();
		virtual bool			GetOptionCode(wstring& valueType);
		virtual bool			GetCPPCode( wstring& cppCode ) const;

		//Operation
		virtual bool			NewFunctionable();
		virtual bool			Deleteable();
		virtual bool			Propertyable();
		virtual bool			Dragable(){ return true; }
	protected:
		wxString				GetReturnTypeCode() const;
	public:
		wstring					m_ReturnCode;
		RETURN_DETAIL			m_eReturnDetail;
		bool					m_canBeAction;
	};
}