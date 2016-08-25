#include "XmlUtility.h"

#include "tinyXML/tinyxml.h"

namespace Space
{
	bool	XmlUtility::SaveAuiPerspective( const wstring& fileName, const wstring& content )
	{
		TiXmlDocument TiDoc;
		TiXmlHandle TiHandle( &TiDoc );

		bool bSuccess = true;

		TiXmlElement* pRoot = new TiXmlElement( ("") );
		pRoot->SetValue( "Perspective" );
		TiXmlText *pText = new TiXmlText( (char*)FTCHARToUTF8(content.c_str()) );
		pText->SetCDATA( true );
		pRoot->LinkEndChild( pText );

		TiHandle.ToNode()->InsertEndChild( *pRoot );
		bSuccess &= TiDoc.SaveFile( (char*)FTCHARToUTF8(fileName.c_str()) );
		assert( bSuccess );
		if( !bSuccess )
			return false;

		SAFE_DELETE( pRoot );

		return true;
	}
	bool	XmlUtility::LoadAuiPerspective( const wstring& fileName, wstring& content )
	{
		bool bSuccess = true;
		TiXmlDocument TiDoc( (char*)FTCHARToUTF8(fileName.c_str()) );
		TiXmlHandle TiHandle( &TiDoc );
		TiXmlBase::SetCondenseWhiteSpace(false);
		bSuccess &= TiDoc.LoadFile( TIXML_ENCODING_UTF8 );
		if( !bSuccess )
			return false;

		TiXmlElement* pRoot = TiHandle.FirstChild( "Perspective" ).ToElement();
		assert( bSuccess &= pRoot != NULL );
		if( !bSuccess )
			return false;
		if( pRoot->FirstChild() == NULL )
			return false;

		TiXmlText *pPerspective = pRoot->FirstChild()->ToText();
		if( pPerspective == NULL )
			return false;

		content = (TCHAR*)FUTF8ToTCHAR(pPerspective->Value());

		return true;
	}
	bool	XmlUtility::GetXMLElementAttribute( const TiXmlElement* pXMLElement, const string& attrName, wstring &result )
	{
		const std::string* pAttri = pXMLElement->Attribute( attrName );
		result = pAttri != NULL ? (TCHAR*)FUTF8ToTCHAR( pAttri->c_str() ) : _T("");
		return pAttri != NULL;
	}
}
