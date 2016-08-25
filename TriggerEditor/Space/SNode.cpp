#include "SNode.h"
#include "SpaceManager.h"
#include "Commands.h"
#include "SFuncCall.h"

using namespace Space;

const char SNode::EventParamName[] = "TEEParam";

//vector<INode::PFNodeDeleting>		INode::ms_DeleteCallback;
SNode::SNode()
:m_classType(SNT_NULL)
,m_pTEETmp(NULL)
,m_errorMask(VS_OK)
,m_bWorking(true)
{
}

SNode::~SNode()
{	
	Definition( NULL );

	int refNum = (int)m_references.size();
	while (refNum-- > 0 && m_references.size() > 0)
	{
		SNode* pReference = *m_references.begin();
		pReference->Definition(NULL);
	}
}

TiXmlElement*	SNode::CreateNodeXml( SNode* pSrc, TiXmlElement* pXmlParent )
{
	if( pSrc == NULL )
		return NULL;

	TiXmlElement* pXmlNode = pSrc->ToXMLElement( );
	if ( pXmlParent )
	{
		pXmlParent->LinkEndChild( pXmlNode );
	}

	CreateNodeXml( pSrc->FirstChild(), pXmlNode );

	CreateNodeXml( pSrc->Next(), pXmlParent );

	return pXmlNode;
}

SNode*	SNode::Clone( const SNode* pSrc, SNode* pParent /*= NULL*/, bool bCloneNext /* = false */ )
{
	if( pSrc == NULL )
		return NULL;

	SNode* pDst = SpaceManager::CreateNode(pSrc->ClassType());
	pDst->CopyFrom(pSrc);
	if (pParent) pParent->AddChild(pDst);

	//child
	SNode* pChild = Clone( pSrc->FirstChild(), pDst, true );

	//next
	if( bCloneNext )
	{
		SNode* pChild = Clone( pSrc->Next(), pParent, true );
	}

	return pDst;
}

ErrorMask	SNode::CheckErrorTree( SNode* pSrc, bool bRecursively /*= true*/ )
{
	if( pSrc == NULL )
		return VS_EVERYTHING;

	ErrorMask oldMask = pSrc->GetErrorMask();

	DoCheckErrorTree( pSrc, bRecursively );
	for(SNode* pParent = pSrc->Parent(); pParent != NULL; pParent = pParent->Parent() )
	{
		DoCheckErrorTree( pParent, false );
	}

	if (pSrc->GetErrorMask() != VS_OK || oldMask != pSrc->GetErrorMask())
		UpdateTreeWidgets(pSrc, true);
	return pSrc->GetErrorMask();
}
inline ErrorMask	SNode::DoCheckErrorTree( SNode* pSrc, bool bRecursively /*= true*/ )
{
	ErrorMask nValid = VS_OK;
	if( pSrc == NULL )
		return nValid;

	pSrc->ClearErrorFlag( VS_EVERYTHING );

	SNode* pChild = pSrc->FirstChild();
	for ( ; pChild != NULL; pChild = pChild->Next() )
	{
		if( bRecursively )
		{
			nValid |= DoCheckErrorTree( pChild, bRecursively );
		}
		else
		{
			nValid |= pChild->GetErrorMask();
		}
	}
	nValid |= pSrc->CheckError();
	pSrc->m_errorMask = ( nValid );

	return nValid;
}
void	SNode::PostLoadedSpace( SNode* pSrc )
{
	if( pSrc == NULL )
		return;

	SNode* pChild = pSrc->FirstChild();
	for ( ; pChild != NULL; pChild = pChild->Next() )
	{
		PostLoadedSpace( pChild );
	}
	pSrc->PostXMLLoaded();
}

//INode static end////////////////////////////////////////////////////////////////////////

TiXmlElement*	SNode::ToXMLElement()
{
	TiXmlElement *pXmlElement = new TiXmlElement( "" );
	if( !m_Name.empty() )
		pXmlElement->SetAttribute( "Name", FTCHARToUTF8(m_Name.c_str()) );

	pXmlElement->SetValue( TypeName() );

	return pXmlElement;
}

FORCEINLINE bool SNode::DoFromXMLElement( const TiXmlElement* pXMLElement )
{
	if( pXMLElement == NULL )
		return false;

	wstring name;
	XmlUtility::GetXMLElementAttribute( pXMLElement, "Name", name );
	Name(name);

	if( pXMLElement->FirstChild() )
	{
		const TiXmlElement	*pXMLChild = pXMLElement->FirstChild()->ToElement();
		for( ; pXMLChild; pXMLChild = pXMLChild->NextSiblingElement() )
		{
			wstring clsName = FUTF8ToTCHAR(pXMLChild->Value());
			SNode* pNode = SpaceManager::CreateNode( clsName );
			assert( pNode );
			if( pNode != NULL )
			{
				AddChild(pNode);
				pNode->FromXMLElement( pXMLChild );
			}
		}
	}
	return true;
}
bool SNode::FromXMLElement( const TiXmlElement* pXMLElement )
{
	return DoFromXMLElement(pXMLElement);
}
bool	SNode::PostXMLLoaded()
{
	return true;
}

void SNode::FixTEETmp()
{
	for (SNode* child = FirstChild(); child; child = child->Next())
	{
		child->FixTEETmp();
	}
}

void SNode::CopyFrom(const SNode* src)
{
	assert(m_classType == src->m_classType);
	m_pTEETmp = src->m_pTEETmp;
	m_Name = src->m_Name;
	m_errorMask = src->m_errorMask;
	m_UserData = src->m_UserData;
	m_bWorking = src->m_bWorking;

	if (SNode* pImpl = Definition())
		pImpl->AddReference(this);

	//clear widget data
	if (!IsA(SNT_GROUP) && !IsA(SNT_TRIGGER))
	{
		UserData("tree", NULL);
		UserData("itemid", NULL);
	}
}

bool	SNode::GetContextView( ContextView& cnxt )
{
	cnxt.text = GetReadText();
	return true;
}
void	SNode::GetChildViews( vector<ContextView>& cnxts )
{
}

void	SNode::GetContextViews( vector<ContextView>& cnxts )
{
	if( TEETmp() == NULL )
		return;

	SNode* pChild = FirstChild();
	TEE::NodeBase *pTmpChild = TEETmp()->FirstChild();
	for ( ; pTmpChild != NULL; pTmpChild = pTmpChild->Next() )
	{
		ContextView cnxt;
		bool bSuccess = true;
		switch(pTmpChild->ClassType())
		{
		case TEE::NT_LABEL:
			cnxt.bPreferences = false;
			cnxt.bValid = true;
			cnxt.pNode = NULL;
			cnxt.text = pTmpChild->GetReadableString();
			cnxts.push_back( cnxt );
			break;
		default:
			//assert( pChild != NULL );
			if( pChild != NULL )
			{
				bSuccess &= pChild->GetContextView( cnxt );
				if ( bSuccess )
				{
					cnxts.push_back( cnxt );
				}
				pChild->GetChildViews( cnxts );

				pChild = pChild->Next();
			}
			break;
		}
	}
}
wstring	SNode::GetReadText() const
{
	wstring text;

	if( TEETmp() == NULL )
		return Name();

	SNode* pChild = FirstChild();
	TEE::NodeBase *pTmpChild = TEETmp()->FirstChild();
	for ( ; pTmpChild != NULL; pTmpChild = pTmpChild->Next() )
	{
		switch(pTmpChild->ClassType())
		{
		case TEE::NT_LABEL:
			text += pTmpChild->GetReadableString();
			break;
		case TEE::NT_PARAMETER:
			if( pChild != NULL )
			{
				wstring readtext = pChild->GetReadText();
				text += readtext;
				pChild = pChild->Next();
			}
			break;
		case TEE::NT_CONTEXTROOT:
			text += pTmpChild->GetReadableString();
			break;
		default:
			break;
		}
		if( !text.empty() && pTmpChild->Next() )
		{
			text += _T(" ");
		}
	}
	for ( ; pChild != NULL; pChild = pChild->Next() )
	{
		text += _T(" ");
		wstring readtext = pChild->GetReadText();
		text += readtext;
	}
	if( text.empty() )
		text = Name();
	return text;
}

ErrorMask	SNode::CheckError()
{
	return GetErrorMask();
}
bool	SNode::GetRealParam( wstring& realParam ) const
{
	assert( false );
	return false;
}
bool	SNode::GetLuaCode( wstring& luaCode, int depth )
{
	assert( false );
	return false;
}
void*	SNode::UserData( const string& key ) const
{
	if( m_UserData.find(key) == m_UserData.end() )
		return NULL;

	map<string, void*>::const_iterator cIt = (m_UserData).find(key);
	return (*cIt).second;
}
void	SNode::UserData( const string& key, void* pValue )
{
	m_UserData[key] = pValue;
	//m_UserData.insert( make_pair( key, pValue ) );
} 
SNode*	SNode::Ancestor( SPACE_NODE_TYPE val, bool bCheckThis )	const
{
	SNode* pAncestor = bCheckThis ? (SNode*)this : Parent();
	while ( pAncestor != NULL && pAncestor->ClassType() != val )
	{
		pAncestor = pAncestor->Parent();
	}
	return pAncestor;
}
int	SNode::ChildCount( SPACE_NODE_TYPE eType ) const
{
	int iCount = 0;
	for( SNode* pChild = FirstChild(); pChild != NULL; pChild = pChild->Next() )
	{
		if( SNT_NULL == eType || pChild->ClassType() == eType )
		{
			++iCount;
		}
	}
	return iCount;
}

void	SNode::Definition( SNode* pImpl )
{
	if( SNode* pOldImpl = Definition() )
		pOldImpl->RemoveReference(this);

	UserData( "definition", reinterpret_cast<void*>(pImpl) );

	if( pImpl != NULL )
	{
		pImpl->AddReference(this);
	}
}

bool SNode::IsStatementFunctionCall() const
{
	return (IsA(SNT_TFUNCCALL) || IsA(SNT_SFUNCCALL)) && (Parent() == nullptr || Parent()->IsStatementRoot());
}

bool SNode::IsVoidFunctionCall()
{
	return IsA(SNT_TFUNCCALL) && (TEETmp() == nullptr || TEETmp()->IsVoidFunction()) ||
		IsA(SNT_SFUNCCALL) && (Definition() == nullptr || Definition()->IsVoidFunction());
}

bool SNode::IsVoidFunction()
{
	if (IsA(SNT_FUNCTION))
	{
		TEE::NodeBase* pTEE = TEETmp();
		return pTEE == nullptr || pTEE->CodeName() == VOID_TEE;
	}
	return false;
}

bool SNode::NodeTreesToString(const vector<SNode*>& Nodes, string& OutStr)
{
	TiXmlDocument TiDoc;
	for (UINT Index = 0; Index < Nodes.size(); ++Index)
	{
		TiXmlElement* pRoot = SNode::CreateNodeXml( Nodes[Index], NULL );
		if (pRoot)
		{
			TiDoc.LinkEndChild(pRoot);
		}
	}

	FILE* fp = NULL;
	errno_t err = fopen_s( &fp, "~~~CopyTmp.###tmp", "w+bTD" );
	if ( fp && err == 0 )
	{
		if (TiDoc.SaveFile(fp))
		{
			fseek( fp, 0, SEEK_END );
			unsigned int FileLength = ftell(fp);
			OutStr.resize( FileLength / sizeof(char) + 1, 0 );
			fseek(fp, 0, SEEK_SET);
			fread_s(&OutStr[0], FileLength, FileLength, 1, fp);
			fclose( fp );
			return true;
		}
		else
		{
			fclose( fp );
		}
	}
	return false;
}

bool SNode::StringToNodeTrees(const string& InStr, vector<SNode*>& OutNodes)
{
	if (InStr.length() == 0)
	{
		return false;
	}
	FILE* fp = NULL;
	errno_t err = fopen_s( &fp, "~~~CopyTmp.###tmp", "w+bTD" );
	if (fp == NULL || err != 0)
	{
		return false;
	}
	fwrite(&InStr[0], InStr.size() * sizeof(char), 1, fp);
	fseek(fp, 0, SEEK_SET);

	TiXmlBase::SetCondenseWhiteSpace(false);
	TiXmlDocument TiDoc;
	if (!TiDoc.LoadFile(fp))
	{
		fclose(fp);
		return false;
	}
	fclose(fp);
	for (TiXmlElement* pXMLElement = TiDoc.FirstChildElement(); pXMLElement != NULL; pXMLElement = pXMLElement->NextSiblingElement())
	{
		wstring clsName = FUTF8ToTCHAR(pXMLElement->Value());
		SNode* pNode = SpaceManager::CreateNode( clsName );
		if (pNode)
		{
			pNode->FromXMLElement( pXMLElement );
			SNode::PostLoadedSpace( pNode );
			SNode::CheckErrorTree( pNode );
			OutNodes.push_back(pNode);
		}
	}
	return true;
}

std::string	SNode::TypeName()
{
	return (ANSICHAR*)FTCHARToANSI(SpaceManager::NodeTypeToName(ClassType()).c_str());
}