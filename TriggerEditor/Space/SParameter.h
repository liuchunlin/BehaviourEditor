#pragma once

#include "SNode.h"
#include "NodeBase.h"

namespace Space
{
	class SParameter: public SNode
	{
		DECLARE_BLOCK_ALLOCATE
	public:
		SParameter();
		virtual TiXmlElement*	ToXMLElement();
		virtual	bool			FromXMLElement( const TiXmlElement* pXMLElement );
		virtual	bool			GetContextView( ContextView& cnxt );
		virtual	void			GetContextViews( vector<ContextView>& cnxts );
		virtual wstring			GetReadText() const;
		virtual ErrorMask	CheckError();
	private:
	};
}