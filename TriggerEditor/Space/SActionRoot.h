#pragma once

#include "SNode.h"

namespace Space
{
	class SActionRoot: public SNode
	{
		DECLARE_BLOCK_ALLOCATE
	public:
		SActionRoot();
		virtual TiXmlElement*	ToXMLElement();
		virtual	bool			FromXMLElement( const TiXmlElement* pXMLElement );
		virtual bool			IsStatementRoot(){ return true; }

		//Menu
		virtual	bool			Pasteable( const SNode* pSrc );
		virtual bool			Deleteable();
		virtual bool			Permitable();
		virtual bool			Disableable(){ return false; }
		virtual bool			NewActionable() { return true; }
	};
}