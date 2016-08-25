#pragma once

#include "SNode.h"
#include "Vek/Base/ObjLink.h"

namespace Space
{
	class STrigger: public SNode
	{
		DECLARE_BLOCK_ALLOCATE
	public:
		STrigger();
		~STrigger();
		virtual TiXmlElement*	ToXMLElement();
		virtual	bool			FromXMLElement( const TiXmlElement* pXMLElement );
		virtual SPACE_NODE_TYPE CommonParentType() const { return SNT_GROUP; }

		virtual	bool			Cutable();
		virtual	bool			Copyable();
		virtual	bool			Pasteable( const SNode* pSrc );
		virtual	bool			Deleteable();
		virtual bool			Paramerterable(){ return true; }
		virtual bool			NewActionable() { return true; }

		virtual bool			UniqueName() const;
	private:
		Vek::ObjLink<STrigger> m_objlink;
	};
}