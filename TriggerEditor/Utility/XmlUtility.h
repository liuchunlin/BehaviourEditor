#pragma once
#include <string>
using namespace std;

#include "tinyXML/tinyxml.h"
#include "UnStringConv.h"
#include "TgrIDs.h"

namespace Space
{
	class XmlUtility
	{
	public:
		XmlUtility(void);
		~XmlUtility(void);

		static bool			SaveAuiPerspective( const wstring& fileName, const wstring& content );
		static bool			LoadAuiPerspective( const wstring& fileName, wstring& content );

		static bool	GetXMLElementAttribute( const TiXmlElement* pXMLElement, const string& attrName, wstring &result );

		template<typename ResultType>
		static	bool	GetXMLAttributeInfo( const TiXmlElement* pXMLElement, const string& attrName, ResultType &result )
		{
			const std::string* pAttri = pXMLElement->Attribute( attrName, &result );
			return pAttri != NULL;
		}

	};

};
