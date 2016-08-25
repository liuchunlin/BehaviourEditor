#include "GenerateCode.h"
#include "TEEManager.h"
#include "NodeVerify.h"
#include "Option.h"
#include "EventCode.h"
#include <strsafe.h>

extern bool g_isGenCsCode;

GenCode* GGenCode = GenCode::GetInstance();

GenCode::GenCode()
{
}

bool GenCode::Gen()
{
	bool bSuccess = true;
	if (g_isGenCsCode)
	{
		bSuccess &= RanaEventImplCS();
		bSuccess &= RanaEventTypeCS();
		bSuccess &= RanaMsgImplCS();
		bSuccess &= RanaRegisterEventCS();
		bSuccess &= RanaSpaceTypeCS();
		bSuccess &= RanaTaskCS();
	}
	else
	{
		bSuccess &= RanaEventImpl();
		bSuccess &= RanaEventType();
		bSuccess &= RanaMsgImpl();
		bSuccess &= RanaRegisterEvent();
		bSuccess &= RanaSpaceType();
		bSuccess &= RanaType();
		bSuccess &= RanaTask();
	}
	return bSuccess;
}

void GenCode::SetGenerateCPPPath( const TCHAR* path )
{
	m_GenCPPPath = FTCHARToANSI(path);
}

bool GenCode::RanaEventImpl()
{
	vector<TEE::NodeBase*> events;
	TEE::GTEEMgr->GetNodesByType(TEE::NT_EVENT, events);

	FILE* file = NULL;
	errno_t err = fopen_s( &file, string(m_GenCPPPath+"/RanaEventImpl.h").c_str(), "w" );
	if ( file == NULL || err != 0 )
		return false;

	char temp[1024];
	//title
	{
		static string s_Titles[] = 
		{
			"#pragma once",
			"#include <string>",
			"#include <Vek/Base/Types.h>",
			"#include <LuaBind/RanaEvent.h>",
			"#include \"RanaEventType.h\"",
			"#include \"RanaMsgImpl.h\"",
			"#include \"RanaType.h\"",
		};
		for ( int k=0; k < ARRAY_COUNT(s_Titles); ++k )
		{
			fprintf( file, s_Titles[k].c_str() );
			fprintf( file, "\n" );
		}
	}
	//content
	{
		vector<TEE::NodeBase*>::iterator iter(events.begin());
		for ( ; iter != events.end(); ++iter )
		{
			TEE::NodeBase* pEvent = (*iter);
			vector<TEE::NodeBase*> paramerts;
			TEE::NodeBase* pChild = pEvent->FirstChild();
			for( ; pChild != NULL; pChild = pChild->Next() )
			{
				if( pChild->ClassType() == TEE::NT_PARAMETER )
				{
					paramerts.push_back( pChild );
				}
			}

			string pName = FTCHARToANSI(pEvent->CodeName().c_str());
			fprintf( file, "class %sEvent: public RanaEvent\n", pName.c_str() );
			fprintf( file, "{\n" );
			fprintf( file, "public:\n" );

			//构造函数
			{
				string param("(");
				vector<TEE::NodeBase*>::iterator itParam(paramerts.begin());
				for ( ; itParam != paramerts.end(); ++itParam )
				{
					TEE::NodeBase* pParameter = (*itParam);
					string paramDisplayName = FTCHARToANSI(pParameter->DisplayName().c_str());
					string paramCodeName = FTCHARToANSI(pParameter->CodeName().c_str());

					StringCbPrintfA( temp, 1024, "RNT_%s::TYPE %s", paramDisplayName.c_str(), paramCodeName.c_str() );
					param += temp;
					param += ( pParameter->HasNextTypeNode(pParameter, pParameter->ClassType()) ? (",") : ("") ) ;
				}
				param += (")\n");

				fprintf( file, "\t%sEvent%s", pName.c_str(), param.c_str() );
				fprintf( file, "\t{\n" );

				{
					fprintf( file, "\t\tsetType(ENS_RanaEvent::E_%s_EVENT);\n", (pName.c_str()) );
					vector<TEE::NodeBase*>::iterator itParam(paramerts.begin());
					for ( ; itParam != paramerts.end(); ++itParam )
					{
						TEE::NodeBase* pParameter = (*itParam);
						string pCode = FTCHARToANSI(pParameter->CodeName().c_str());
						fprintf( file, "\t\tm_%s = %s;\n", pCode.c_str(), pCode.c_str() );
					}
				}
				fprintf( file, "\t\tgetArgument(m_args);\n" );

				fprintf( file, "\t}\n" );
			}

			// Custom msg event code
			TEE::MsgEventCode* pMsgEventCode = static_cast<TEE::MsgEventCode*>(pEvent->FindFirstChild(TEE::VerifyNodeType(TEE::NT_MSGEVENTCODE)));
			if (pMsgEventCode)
			{
				fprintf( file, FTCHARToANSI(pMsgEventCode->m_strCode.c_str()) );
				fprintf( file, "\n" );
			}

			// Custom event code
			TEE::EventCode* pEventCode = static_cast<TEE::EventCode*>(pEvent->FindFirstChild(TEE::VerifyNodeType(TEE::NT_EVENTCODE)));
			if (pEventCode)
			{
				fprintf( file, FTCHARToANSI(pEventCode->m_strCode.c_str()) );
				fprintf( file, "\n" );
			}

			//preValidate
			bool bDefinedPreValidate = pEventCode && pEventCode->m_strCode.find(TEXT("preValidate")) != wstring::npos;
			if (!bDefinedPreValidate)
			{
				fprintf( file, "\tvirtual bool preValidate(RanaMessage *msg)\n" );
				fprintf( file, "\t{\n" );
				fprintf( file, "\t\t%sMsg* pMsg = static_cast<%sMsg*>(msg);\n", pName.c_str(), pName.c_str() );
				string condition;
				vector<TEE::NodeBase*>::iterator itParam(paramerts.begin());
				for ( ; itParam != paramerts.end(); ++itParam )
				{
					TEE::NodeBase* pParameter = (*itParam);
					string pCode = FTCHARToANSI(pParameter->CodeName().c_str());
					char con[256];
					StringCbPrintfA( con, 256, "pMsg->get_%s() != m_%s%s", pCode.c_str(), pCode.c_str(), itParam+1 != paramerts.end() ? " || " : "" );

					condition += con;
				}
				if( !paramerts.empty() )
				{
					fprintf( file, "\t\tif( %s )\n", condition.c_str() );
					fprintf( file, "\t\t{\n" );
					fprintf( file, "\t\t\treturn false;\n" );
					fprintf( file, "\t\t}\n" );
				}
				fprintf( file, "\t\treturn true;\n" );

				fprintf( file, "\t}\n" );
			}
			//getArgument
			{
				fprintf( file, "\tvirtual void getArgument(std::vector<RanaVariant>& args)\n" );
				fprintf( file, "\t{\n" );
				if( !paramerts.empty() )
					fprintf( file, "\t\tRanaVariant arg;\n" );
				vector<TEE::NodeBase*>::iterator itParam(paramerts.begin());
				for ( ; itParam != paramerts.end(); ++itParam )
				{
					TEE::NodeBase* pParameter = (*itParam);
					string pCode = FTCHARToANSI(pParameter->CodeName().c_str());
					fprintf( file, "\t\targ = m_%s;\n", pCode.c_str() );
					fprintf( file, "\t\targs.push_back(arg);\n" );
				}
				fprintf( file, "\t}\n" );

			}

			//private member
			{
				fprintf( file, "private:\n" );
				vector<TEE::NodeBase*>::iterator itParam(paramerts.begin());
				for ( ; itParam != paramerts.end(); ++itParam )
				{
					TEE::NodeBase* pParameter = (*itParam);
					string pCode = FTCHARToANSI(pParameter->CodeName().c_str());
					string pDisplayName = FTCHARToANSI(pParameter->DisplayName().c_str());
					fprintf( file, "\tRNT_%s::TYPE m_%s;\n", pDisplayName.c_str(), pCode.c_str() );
				}
			}

			fprintf( file, "};\n" );
		}
	}

	//end
	{
		fprintf( file, "namespace ENS_RanaEvent\n" );
		fprintf( file, "{\n" );
		fprintf( file, "\textern RanaEvent* createEvent(WORD type,const std::vector<RanaVariant>& args);\n" );
		fprintf( file, "}\n" );
	}

	fclose( file );
	return true;
}

bool GenCode::RanaEventImplCS()
{
	vector<TEE::NodeBase*> events;
	TEE::GTEEMgr->GetNodesByType(TEE::NT_EVENT, events);

	FILE* file = NULL;
	errno_t err = fopen_s( &file, string(m_GenCPPPath+"/RanaEventImpl.cs").c_str(), "w" );
	if ( file == NULL || err != 0 )
		return false;

	char temp[1024];
	//include
	{
		fprintf( file, "using System.Collections.Generic;\n" );
	}
	//content
	{
		vector<TEE::NodeBase*>::iterator iter(events.begin());
		for ( ; iter != events.end(); ++iter )
		{
			TEE::NodeBase* pEvent = (*iter);
			vector<TEE::NodeBase*> paramerts;
			TEE::NodeBase* pChild = pEvent->FirstChild();
			for( ; pChild != NULL; pChild = pChild->Next() )
			{
				if( pChild->ClassType() == TEE::NT_PARAMETER )
				{
					paramerts.push_back( pChild );
				}
			}

			string pName = FTCHARToANSI(pEvent->CodeName().c_str());
			fprintf( file, "public class %sEvent : RanaEvent\n", pName.c_str() );
			fprintf( file, "{\n" );

			//构造函数
			{
				string param("(");
				vector<TEE::NodeBase*>::iterator itParam(paramerts.begin());
				for ( ; itParam != paramerts.end(); ++itParam )
				{
					TEE::NodeBase* pParameter = (*itParam);
					string paramTypeName = GetValueTypeCodeName(pParameter);
					string paramCodeName = FTCHARToANSI(pParameter->CodeName().c_str());

					StringCbPrintfA( temp, 1024, "%s %s", paramTypeName.c_str(), paramCodeName.c_str() );
					param += temp;
					param += ( pParameter->HasNextTypeNode(pParameter, pParameter->ClassType()) ? (",") : ("") ) ;
				}
				param += (")\n");

				fprintf( file, "\tpublic %sEvent%s", pName.c_str(), param.c_str() );
				fprintf( file, "\t{\n" );

				{
					fprintf( file, "\t\tm_type = RanaEventType.%s;\n", (pName.c_str()) );
					vector<TEE::NodeBase*>::iterator itParam(paramerts.begin());
					for ( ; itParam != paramerts.end(); ++itParam )
					{
						TEE::NodeBase* pParameter = (*itParam);
						string pCode = FTCHARToANSI(pParameter->CodeName().c_str());
						fprintf( file, "\t\tm_%s = %s;\n", pCode.c_str(), pCode.c_str() );
					}
				}

				fprintf( file, "\t}\n" );
			}

			// Custom msg event code
			TEE::MsgEventCode* pMsgEventCode = static_cast<TEE::MsgEventCode*>(pEvent->FindFirstChild(TEE::VerifyNodeType(TEE::NT_MSGEVENTCODE)));
			if (pMsgEventCode)
			{
				fprintf( file, FTCHARToANSI(pMsgEventCode->m_strCode.c_str()) );
				fprintf( file, "\n" );
			}

			// Custom event code
			TEE::EventCode* pEventCode = static_cast<TEE::EventCode*>(pEvent->FindFirstChild(TEE::VerifyNodeType(TEE::NT_EVENTCODE)));
			if (pEventCode)
			{
				fprintf( file, FTCHARToANSI(pEventCode->m_strCode.c_str()) );
				fprintf( file, "\n" );
			}

			//preValidate
			bool bDefinedPreValidate = pEventCode && pEventCode->m_strCode.find(TEXT("preValidate")) != wstring::npos;
			if (!bDefinedPreValidate)
			{
				fprintf( file, "\tpublic override bool preValidate(RanaMessage msg)\n" );
				fprintf( file, "\t{\n" );
				fprintf( file, "\t\t%sMsg pMsg = (%sMsg)(msg);\n", pName.c_str(), pName.c_str() );
				string condition;
				vector<TEE::NodeBase*>::iterator itParam(paramerts.begin());
				for ( ; itParam != paramerts.end(); ++itParam )
				{
					TEE::NodeBase* pParameter = (*itParam);
					string pCode = FTCHARToANSI(pParameter->CodeName().c_str());
					char con[256];
					StringCbPrintfA( con, 256, "pMsg.%s != m_%s%s", pCode.c_str(), pCode.c_str(), itParam+1 != paramerts.end() ? " || " : "" );

					condition += con;
				}
				if( !paramerts.empty() )
				{
					fprintf( file, "\t\tif( %s )\n", condition.c_str() );
					fprintf( file, "\t\t{\n" );
					fprintf( file, "\t\t\treturn false;\n" );
					fprintf( file, "\t\t}\n" );
				}
				fprintf( file, "\t\treturn true;\n" );

				fprintf( file, "\t}\n" );
			}

			//private member
			{
				fprintf( file, "\n" );
				vector<TEE::NodeBase*>::iterator itParam(paramerts.begin());
				for ( ; itParam != paramerts.end(); ++itParam )
				{
					TEE::NodeBase* pParameter = (*itParam);
					string pCode = FTCHARToANSI(pParameter->CodeName().c_str());
					string paramTypeName = GetValueTypeCodeName(pParameter);
					fprintf( file, "\tprivate %s m_%s;\n", paramTypeName.c_str(), pCode.c_str() );
				}
			}

			fprintf( file, "};\n" );
		}
	}

	fclose( file );
	return true;
}

bool GenCode::RanaEventType()
{
	vector<TEE::NodeBase*> events;
	TEE::GTEEMgr->GetNodesByType(TEE::NT_EVENT, events);

	FILE* file = NULL;
	errno_t err = fopen_s( &file, string(m_GenCPPPath+"/RanaEventType.h").c_str(), "w" );
	if ( file == NULL || err != 0 )
		return false;

	//title
	{
		static string s_Titles[] = 
		{
			"#pragma once",
		};
		for ( int k=0; k < ARRAY_COUNT(s_Titles); ++k )
		{
			fprintf( file, s_Titles[k].c_str() );
			fprintf( file, "\n" );
		}
	}
	//content
	{
		fprintf( file, "namespace ENS_RanaEvent\n" );
		fprintf( file, "{\n" );
		fprintf( file, "\tenum TYPE\n" );
		fprintf( file, "\t{\n" );
		vector<TEE::NodeBase*>::iterator iter(events.begin());
		for ( ; iter != events.end(); ++iter )
		{
			TEE::NodeBase* pEvent = (*iter);
			string pName = FTCHARToANSI(pEvent->CodeName().c_str());
			string pDisplayName = FTCHARToANSI(pEvent->DisplayName().c_str());
			fprintf( file, "\t\tE_%s_EVENT", (pName.c_str()) );
			if( iter == events.begin() )
			{
				fprintf( file, " = RANA_USER_EVENT,	//%s\n", pDisplayName.c_str() );
			}
			else
			{
				fprintf( file, ",	//%s\n", pDisplayName.c_str() );
			}
		}
		
		fprintf( file, "\t\tEVENT_COUNT\n" );
		fprintf( file, "\t};\n" );
		fprintf( file, "\tconst char* getRanaEventNameFromEnum( TYPE eventType );\n" );
		fprintf( file, "\tbool getRanaEventEnumFromName(const char* eventTypeName, TYPE& eventTypeOut);\n" );
		fprintf( file, "};\n" );
	}
	fclose( file );
	return true;
}

bool GenCode::RanaEventTypeCS()
{
	vector<TEE::NodeBase*> events;
	TEE::GTEEMgr->GetNodesByType(TEE::NT_EVENT, events);

	FILE* file = NULL;
	errno_t err = fopen_s( &file, string(m_GenCPPPath+"/RanaEventType.cs").c_str(), "w" );
	if ( file == NULL || err != 0 )
		return false;

	//content
	{
		fprintf( file, "public enum RanaEventType\n" );
		fprintf( file, "{\n" );
		vector<TEE::NodeBase*>::iterator iter(events.begin());
		for ( ; iter != events.end(); ++iter )
		{
			TEE::NodeBase* pEvent = (*iter);
			string pName = FTCHARToANSI(pEvent->CodeName().c_str());
			string pDisplayName = FTCHARToANSI(pEvent->DisplayName().c_str());
			fprintf( file, "\t%s,	//%s\n", pName.c_str(), pDisplayName.c_str() );
		}

		fprintf( file, "\tEVENT_COUNT\n" );
		fprintf( file, "};\n" );
	}
	fclose( file );
	return true;
}

bool GenCode::RanaMsgImpl( )
{
	vector<TEE::NodeBase*> events;
	TEE::GTEEMgr->GetNodesByType(TEE::NT_EVENT, events);

	FILE* file = NULL;
	errno_t err = fopen_s( &file, string(m_GenCPPPath+"/RanaMsgImpl.h").c_str(), "w" );
	if ( file == NULL || err != 0 )
		return false;

	char temp[1024];
	//title
	{
		static string s_Titles[] = 
		{
			"#pragma once",
			"#include <LuaBind/RanaMessage.h>",
			"#include \"RanaEventType.h\"",
			"#include <Vek/Base/Types.h>",
			"#include \"RanaType.h\"",
		};
		for ( int k=0; k < ARRAY_COUNT(s_Titles); ++k )
		{
			fprintf( file, s_Titles[k].c_str() );
			fprintf( file, "\n" );
		}
	}
	//content
	{
		vector<TEE::NodeBase*>::iterator iter(events.begin());
		for ( ; iter != events.end(); ++iter )
		{
			TEE::NodeBase* pEvent = (*iter);

			string pName = FTCHARToANSI(pEvent->CodeName().c_str());
			fprintf( file, "class %sMsg: public RanaMessage\n", pName.c_str() );
			fprintf( file, "{\n" );
			fprintf( file, "public:\n" );

			//构造函数
			vector<TEE::NodeBase*> paramerts;
			pEvent->FindChildren(TEE::VerifyNodeType(TEE::NT_PARAMETER), paramerts);
			{
				string paramCode("(");
				vector<TEE::NodeBase*>::iterator itParam(paramerts.begin());
				for ( ; itParam != paramerts.end(); ++itParam )
				{
					TEE::NodeBase* pParameter = (*itParam);
					string paramDisplayName = FTCHARToANSI(pParameter->DisplayName().c_str());
					string paramCodeName = FTCHARToANSI(pParameter->CodeName().c_str());

					StringCbPrintfA( temp, 1024, "RNT_%s::TYPE %s", paramDisplayName.c_str(), paramCodeName.c_str() );
					paramCode += temp;
					paramCode += ( pParameter->HasNextTypeNode(pParameter, pParameter->ClassType()) ? (",") : ("") ) ;
				}
				paramCode += (")\n");

				fprintf( file, "\t%sMsg%s", pName.c_str(), paramCode.c_str() );
				fprintf( file, "\t{\n" );

				{
					fprintf( file, "\t\tsetEventType(ENS_RanaEvent::E_%s_EVENT);\n", (pName.c_str()) );
					vector<TEE::NodeBase*>::iterator itParam(paramerts.begin());
					for ( ; itParam != paramerts.end(); ++itParam )
					{
						TEE::NodeBase* pParameter = (*itParam);
						string pCode = FTCHARToANSI(pParameter->CodeName().c_str());
						fprintf( file, "\t\tm_%s = %s;\n", pCode.c_str(), pCode.c_str() );
					}
				}

				fprintf( file, "\t}\n" );
			}

			// Custom msg event code
			TEE::MsgEventCode* pMsgEventCode = static_cast<TEE::MsgEventCode*>(pEvent->FindFirstChild(TEE::VerifyNodeType(TEE::NT_MSGEVENTCODE)));
			if (pMsgEventCode)
			{
				fprintf( file, FTCHARToANSI(pMsgEventCode->m_strCode.c_str()) );
				fprintf( file, "\n" );
			}

			//void SetContexts(...)
			vector<TEE::NodeBase*> contextParams;
			pEvent->FindChildren(TEE::VerifyNodeType(TEE::NT_CONTEXTPARAM), contextParams, true);
			if (contextParams.size() > 0)
			{
				string paramCode("(");
				vector<TEE::NodeBase*>::iterator itParam(contextParams.begin());
				for ( ; itParam != contextParams.end(); ++itParam )
				{
					TEE::NodeBase* pParameter = (*itParam);
					string paramDisplayName = FTCHARToANSI(pParameter->DisplayName().c_str());
					string paramCodeName = FTCHARToANSI(pParameter->CodeName().c_str());

					StringCbPrintfA( temp, 1024, "RNT_%s::TYPE %s", paramDisplayName.c_str(), paramCodeName.c_str() );
					paramCode += temp;
					paramCode += ( pParameter->HasNextTypeNode(pParameter, pParameter->ClassType()) ? (",") : ("") ) ;
				}
				paramCode += (")\n");

				fprintf( file, "\tvoid SetContexts%s", paramCode.c_str() );
				fprintf( file, "\t{\n" );

				{
					fprintf( file, "\t\tRanaVariant __rval__;\n" );
					vector<TEE::NodeBase*>::iterator itParam(contextParams.begin());
					for ( ; itParam != contextParams.end(); ++itParam )
					{
						TEE::NodeBase* pParameter = (*itParam);
						string paramCodeName = FTCHARToANSI(pParameter->CodeName().c_str());
						fprintf( file, "\t\t__rval__ = %s;\n", paramCodeName.c_str() );
						fprintf( file, "\t\tm_contexts.push_back(__rval__);\n");
					}
				}

				fprintf( file, "\t}\n" );
			}

			//get
			{
				vector<TEE::NodeBase*>::iterator itParam(paramerts.begin());
				for ( ; itParam != paramerts.end(); ++itParam )
				{
					TEE::NodeBase* pParameter = (*itParam);
					string pCode = FTCHARToANSI(pParameter->CodeName().c_str());
					string pDisplayName = FTCHARToANSI(pParameter->DisplayName().c_str());

					fprintf( file, "\tRNT_%s::TYPE get_%s() const\n", pDisplayName.c_str(), pCode.c_str() );
					fprintf( file, "\t{\n" );
					fprintf( file, "\t\treturn m_%s;\n", pCode.c_str() );
					fprintf( file, "\t}\n" );

				}
			}

			//private member
			{
				fprintf( file, "private:\n" );
				vector<TEE::NodeBase*>::iterator itParam(paramerts.begin());
				for ( ; itParam != paramerts.end(); ++itParam )
				{
					TEE::NodeBase* pParameter = (*itParam);
					string pCode = FTCHARToANSI(pParameter->CodeName().c_str());
					string pDisplayName = FTCHARToANSI(pParameter->DisplayName().c_str());
					fprintf( file, "\tRNT_%s::TYPE m_%s;\n", pDisplayName.c_str(), pCode.c_str() );
				}
			}

			fprintf( file, "};\n" );
		}
	}

	fclose( file );
	return true;
}

bool GenCode::RanaMsgImplCS( )
{
	vector<TEE::NodeBase*> events;
	TEE::GTEEMgr->GetNodesByType(TEE::NT_EVENT, events);

	FILE* file = NULL;
	errno_t err = fopen_s( &file, string(m_GenCPPPath+"/RanaMsgImpl.cs").c_str(), "w" );
	if ( file == NULL || err != 0 )
		return false;

	char temp[1024];

	//include
	{
		fprintf( file, "using System.Collections.Generic;\n" );
	}

	//content
	{
		vector<TEE::NodeBase*>::iterator iter(events.begin());
		for ( ; iter != events.end(); ++iter )
		{
			TEE::NodeBase* pEvent = (*iter);

			string pName = FTCHARToANSI(pEvent->CodeName().c_str());
			fprintf( file, "public class %sMsg: RanaMessage\n", pName.c_str() );
			fprintf( file, "{\n" );

			//构造函数
			vector<TEE::NodeBase*> paramerts;
			pEvent->FindChildren(TEE::VerifyNodeType(TEE::NT_PARAMETER), paramerts);
			{
				string paramCode("(");
				vector<TEE::NodeBase*>::iterator itParam(paramerts.begin());
				for ( ; itParam != paramerts.end(); ++itParam )
				{
					TEE::NodeBase* pParameter = (*itParam);
					string paramTypeName = GetValueTypeCodeName(pParameter);
					string paramCodeName = FTCHARToANSI(pParameter->CodeName().c_str());

					StringCbPrintfA( temp, 1024, "%s %s", paramTypeName.c_str(), paramCodeName.c_str() );
					paramCode += temp;
					paramCode += ( pParameter->HasNextTypeNode(pParameter, pParameter->ClassType()) ? (",") : ("") ) ;
				}
				paramCode += (")\n");

				fprintf( file, "\tpublic %sMsg%s", pName.c_str(), paramCode.c_str() );
				fprintf( file, "\t{\n" );

				{
					fprintf( file, "\t\tm_eventType = RanaEventType.%s;\n", (pName.c_str()) );
					vector<TEE::NodeBase*>::iterator itParam(paramerts.begin());
					for ( ; itParam != paramerts.end(); ++itParam )
					{
						TEE::NodeBase* pParameter = (*itParam);
						string pCode = FTCHARToANSI(pParameter->CodeName().c_str());
						fprintf( file, "\t\tm_%s = %s;\n", pCode.c_str(), pCode.c_str() );
					}
				}

				fprintf( file, "\t}\n" );
			}

			// Custom msg event code
			TEE::MsgEventCode* pMsgEventCode = static_cast<TEE::MsgEventCode*>(pEvent->FindFirstChild(TEE::VerifyNodeType(TEE::NT_MSGEVENTCODE)));
			if (pMsgEventCode)
			{
				fprintf( file, FTCHARToANSI(pMsgEventCode->m_strCode.c_str()) );
				fprintf( file, "\n" );
			}

			//void SetContexts(...)
			vector<TEE::NodeBase*> contextParams;
			pEvent->FindChildren(TEE::VerifyNodeType(TEE::NT_CONTEXTPARAM), contextParams, true);
			if (contextParams.size() > 0)
			{
				string paramCode("(");
				vector<TEE::NodeBase*>::iterator itParam(contextParams.begin());
				for ( ; itParam != contextParams.end(); ++itParam )
				{
					TEE::NodeBase* pParameter = (*itParam);
					string paramTypeName = GetValueTypeCodeName(pParameter);
					string paramCodeName = FTCHARToANSI(pParameter->CodeName().c_str());

					StringCbPrintfA( temp, 1024, "%s %s", paramTypeName.c_str(), paramCodeName.c_str() );
					paramCode += temp;
					paramCode += ( pParameter->HasNextTypeNode(pParameter, pParameter->ClassType()) ? (",") : ("") ) ;
				}
				paramCode += (")\n");

				fprintf( file, "\tpublic void SetContexts%s", paramCode.c_str() );
				fprintf( file, "\t{\n" );

				{
					fprintf( file, "\t\tm_contexts = new List<object>();\n" );
					vector<TEE::NodeBase*>::iterator itParam(contextParams.begin());
					for ( ; itParam != contextParams.end(); ++itParam )
					{
						TEE::NodeBase* pParameter = (*itParam);
						string paramCodeName = FTCHARToANSI(pParameter->CodeName().c_str());
						fprintf( file, "\t\tm_contexts.Add(%s);\n", paramCodeName.c_str() );
					}
				}

				fprintf( file, "\t}\n" );
			}

			//getter/setter
			{
				vector<TEE::NodeBase*>::iterator itParam(paramerts.begin());
				for ( ; itParam != paramerts.end(); ++itParam )
				{
					TEE::NodeBase* pParameter = (*itParam);
					string pCode = FTCHARToANSI(pParameter->CodeName().c_str());
					string pTypeName = GetValueTypeCodeName(pParameter);

					fprintf( file, "\tpublic %s %s { get { return m_%s; } set { m_%s = value; } } \n", pTypeName.c_str(), pCode.c_str(), pCode.c_str(), pCode.c_str() );
				}
			}

			//private member
			{
				vector<TEE::NodeBase*>::iterator itParam(paramerts.begin());
				for ( ; itParam != paramerts.end(); ++itParam )
				{
					TEE::NodeBase* pParameter = (*itParam);
					string pCode = FTCHARToANSI(pParameter->CodeName().c_str());
					string typeName = GetValueTypeCodeName(pParameter);
					fprintf( file, "\tprivate %s m_%s;\n", typeName.c_str(), pCode.c_str() );
				}
			}

			fprintf( file, "};\n" );
		}
	}

	fclose( file );
	return true;
}

bool GenCode::RanaRegisterEvent( )
{
	vector<TEE::NodeBase*> events;
	TEE::GTEEMgr->GetNodesByType(TEE::NT_EVENT, events);

	FILE* file = NULL;
	errno_t err = fopen_s( &file, string(m_GenCPPPath+"/RanaRegisterEvent.cpp").c_str(), "w" );
	if ( file == NULL || err != 0 )
		return false;

	//title
	{
		static string s_Titles[] = 
		{
			"#include \"RanaRegisterEvent.h\"\n",
			"\n",
		};


		for ( int k=0; k < ARRAY_COUNT(s_Titles); ++k )
		{
			fprintf( file, s_Titles[k].c_str() );
			fprintf( file, "\n" );
		}
		//Register
		vector<TEE::NodeBase*>	spaces;
		TEE::GTEEMgr->SpaceRoot()->FindChildren(TEE::VerifyNodeType(TEE::NT_SPACEROOT), spaces);
		vector<TEE::NodeBase*>::iterator	iterSpace(spaces.begin());
		for( ; iterSpace != spaces.end(); ++iterSpace )
		{
			string SpaceName = FTCHARToANSI((*iterSpace)->CodeName().c_str()); 
			fprintf( file, "REG_RANA_EVENT(RANA_SPACETYPE_%s)\n", SpaceName.c_str() );
			fprintf( file, "REG_RANA_TRIGGER(RANA_SPACETYPE_%s)\n", SpaceName.c_str() );
			fprintf( file, "REG_RANA_TIMER(RANA_SPACETYPE_%s)\n", SpaceName.c_str() );
			fprintf( file, "REG_RANA_CONTEXT(RANA_SPACETYPE_%s)\n\n", SpaceName.c_str() );

		}

		vector<TEE::NodeBase*>::iterator iter(events.begin());
		for ( ; iter != events.end(); ++iter )
		{
			TEE::NodeBase* pEvent = (*iter);
			if( TEE::IsInternalEvent( pEvent->CodeName() ) )
				continue;
			string pName = FTCHARToANSI(pEvent->CodeName().c_str());
			//Register
			vector<wstring>	spaces = TEE::GTEEMgr->GetPertainSpaces( pEvent );
			vector<wstring>::iterator	iterSpace(spaces.begin());
			for( ; iterSpace != spaces.end(); ++iterSpace )
			{
				string SpaceName = FTCHARToANSI((*iterSpace).c_str()); 
				fprintf( file, "REG_RANA_FUNC(RANA_SPACETYPE_%s,TriggerRegister%sEvent)\n", SpaceName.c_str(), pName.c_str() );
			}
		}
	}
	//content
	{
		vector<TEE::NodeBase*>::iterator iter(events.begin());
		for ( ; iter != events.end(); ++iter )
		{
			TEE::NodeBase* pEvent = (*iter);
			if( TEE::IsInternalEvent( pEvent->CodeName() ) )
				continue;
			vector<TEE::NodeBase*> paramerts;
			TEE::NodeBase* pChild = pEvent->FirstChild();
			for( ; pChild != NULL; pChild = pChild->Next() )
			{
				if( pChild->ClassType() == TEE::NT_PARAMETER )
				{
					paramerts.push_back( pChild );
				}
			}

			string pName = FTCHARToANSI(pEvent->CodeName().c_str());

			fprintf( file, "int l_TriggerRegister%sEvent(RanaState L)\n", pName.c_str() );
			fprintf( file, "{\n" );
			string params;
			vector<TEE::NodeBase*>::iterator itParam(paramerts.begin());
			for ( int iIndex = 2; itParam != paramerts.end(); ++itParam, ++iIndex )
			{
				TEE::NodeBase* pParameter = (*itParam);
				string pCode = FTCHARToANSI(pParameter->CodeName().c_str());
				string pDisplayName = FTCHARToANSI(pParameter->DisplayName().c_str());
			
				params += pCode;
				if ( itParam+1 != paramerts.end() )
				{
					params += ",";
				}

				TEE::Option* pOption = (TEE::Option*)TEE::GTEEMgr->FindOptionFromCode(pParameter->DisplayName());
				string luaType("lua_tonumber");
				switch(pOption->m_eValueType)
				{
				case TEE::VT_INT:
				case TEE::VT_REAL:
					luaType = "lua_tonumber";
					fprintf( file, "\tRNT_%s::TYPE %s = static_cast<RNT_%s::TYPE>(%s(L,%d));\n", pDisplayName.c_str(), pCode.c_str(), pDisplayName.c_str(), luaType.c_str(), iIndex );
					break;
				case TEE::VT_STRING:
					luaType = "lua_tostring";
					fprintf( file, "\tRNT_%s::TYPE %s = static_cast<RNT_%s::TYPE>(%s(L,%d));\n", pDisplayName.c_str(), pCode.c_str(), pDisplayName.c_str(), luaType.c_str(), iIndex );
					break;
				case TEE::VT_BOOLEAN:
					luaType = "lua_toboolean";
					fprintf( file, "\tRNT_%s::TYPE %s = static_cast<RNT_%s::TYPE>(%s(L,%d)!=0);\n", pDisplayName.c_str(), pCode.c_str(), pDisplayName.c_str(), luaType.c_str(), iIndex );
					break;
				default:
					assert( false );
					break;
				}
			}
			fprintf( file, "\t%sEvent *event = new %sEvent(%s);\n", pName.c_str(), pName.c_str(), params.c_str() );
			fprintf( file, "\treturn CreatTrigger(L,event);\n" );
			fprintf( file, "}\n" );
		}

		{
			fprintf( file, "namespace ENS_RanaEvent\n" );
			fprintf( file, "{\n" );
			vector<TEE::NodeBase*>::iterator iter(events.begin());
			for ( ; iter != events.end(); ++iter )
			{
				TEE::NodeBase* pEvent = (*iter);
				if( TEE::IsInternalEvent( pEvent->CodeName() ) )
					continue;
				vector<TEE::NodeBase*> paramerts;
				TEE::NodeBase* pChild = pEvent->FirstChild();
				for( ; pChild != NULL; pChild = pChild->Next() )
				{
					if( pChild->ClassType() == TEE::NT_PARAMETER )
					{
						paramerts.push_back( pChild );
					}
				}

				string pName = FTCHARToANSI(pEvent->CodeName().c_str());

				string params;
				vector<TEE::NodeBase*>::iterator itParam(paramerts.begin());
				for ( int iIndex = 0; itParam != paramerts.end(); ++itParam, ++iIndex )
				{
					char con[256];
					StringCbPrintfA( con, 256, "args[%d]", iIndex );
					params += con;
					if( itParam+1 != paramerts.end() )
						params += ",";
				}

				fprintf( file, "\tstatic RanaEvent* Create%sEvent(const std::vector<RanaVariant>& args)\n", pName.c_str() );
				fprintf( file, "\t{\n" );
				fprintf( file, "\t\tif(args.size()!= %d){ return NULL; }\n", paramerts.size() );
				fprintf( file, "\t\ttry{ return new %sEvent(%s);}\n", pName.c_str(), params.c_str() );
				fprintf( file, "\t\tcatch(RanaVariant::RanaBadVariant *){return NULL;}\n" );

				fprintf( file, "\t}\n" );
				fprintf( file, "\tREGISTER_EVENT( E_%s_EVENT, \"%s\", Create%sEvent );\n", pName.c_str(), pName.c_str(), pName.c_str() );
			}
		}
		fprintf( file, "}\n" );

	}

	fclose( file );
	return true;
}

bool GenCode::RanaRegisterEventCS( )
{
	vector<TEE::NodeBase*> events;
	TEE::GTEEMgr->GetNodesByType(TEE::NT_EVENT, events);

	FILE* file = NULL;
	errno_t err = fopen_s( &file, string(m_GenCPPPath+"/RanaRegisterEvent.cs").c_str(), "w" );
	if ( file == NULL || err != 0 )
		return false;

	//include
	{
		fprintf( file, "using System;\n" );
		fprintf( file, "using System.Collections.Generic;\n" );
		fprintf( file, "using LuaInterface;\n" );
	}

	//content
	fprintf( file, "\n#pragma warning disable 0414 //disable warning CS0414: The private field XXX is assigned but its value is never used\n" );
	{
		fprintf( file, "public partial class LuaAPISet\n" );
		fprintf( file, "{\n" );

		vector<TEE::NodeBase*>::iterator iter(events.begin());
		for ( ; iter != events.end(); ++iter )
		{
			TEE::NodeBase* pEvent = (*iter);
			if( TEE::IsInternalEvent( pEvent->CodeName() ) )
				continue;

			//Register
			string eventName = FTCHARToANSI(pEvent->CodeName().c_str());
			char funcName[1024];
			StringCbPrintfA( funcName, sizeof(funcName), "TriggerRegister%sEvent", eventName.c_str() );
			vector<wstring>	spaces = TEE::GTEEMgr->GetPertainSpaces( pEvent );
			vector<wstring>::iterator	iterSpace(spaces.begin());
			for( ; iterSpace != spaces.end(); ++iterSpace )
			{
				string SpaceName = FTCHARToANSI((*iterSpace).c_str()); 
				fprintf( file, "\tprivate int %s_%s = RanaSpace.AddLuaAPI((int)ERanaSpaceType.%s, \"%s\", l_%s);\n", funcName, SpaceName.c_str(), SpaceName.c_str(), funcName, funcName );
			}

			vector<TEE::NodeBase*> paramerts;
			TEE::NodeBase* pChild = pEvent->FirstChild();
			for( ; pChild != NULL; pChild = pChild->Next() )
			{
				if( pChild->ClassType() == TEE::NT_PARAMETER )
				{
					paramerts.push_back( pChild );
				}
			}

			fprintf( file, "\tpublic static int l_%s(IntPtr L)\n", funcName );
			fprintf( file, "\t{\n" );
			string params;
			vector<TEE::NodeBase*>::iterator itParam(paramerts.begin());
			for ( int iIndex = 2; itParam != paramerts.end(); ++itParam, ++iIndex )
			{
				TEE::NodeBase* pParameter = (*itParam);
				string pCode = FTCHARToANSI(pParameter->CodeName().c_str());
				string typeName = GetValueTypeCodeName(pParameter);

				params += pCode;
				if ( itParam+1 != paramerts.end() )
				{
					params += ",";
				}

				TEE::Option* pOption = (TEE::Option*)TEE::GTEEMgr->FindOptionFromCode(pParameter->DisplayName());
				string luaType("LuaDLL.lua_tonumber");
				switch(pOption->m_eValueType)
				{
				case TEE::VT_INT:
				case TEE::VT_REAL:
					luaType = "LuaDLL.lua_tonumber";
					fprintf( file, "\t\t%s %s = (%s)(%s(L,%d));\n", typeName.c_str(), pCode.c_str(), typeName.c_str(), luaType.c_str(), iIndex );
					break;
				case TEE::VT_STRING:
					luaType = "LuaDLL.lua_tostring";
					fprintf( file, "\t\t%s %s = (%s)(%s(L,%d));\n", typeName.c_str(), pCode.c_str(), typeName.c_str(), luaType.c_str(), iIndex );
					break;
				case TEE::VT_BOOLEAN:
					luaType = "LuaDLL.lua_toboolean";
					fprintf( file, "\t\t%s %s = (%s)(%s(L,%d)!=0);\n", typeName.c_str(), pCode.c_str(), typeName.c_str(), luaType.c_str(), iIndex );
					break;
				default:
					assert( false );
					break;
				}
			}
			fprintf( file, "\t\t%sEvent pEvent = new %sEvent(%s);\n", eventName.c_str(), eventName.c_str(), params.c_str() );
			fprintf( file, "\t\treturn RanaSpace.CreateTrigger(L,pEvent);\n" );
			fprintf( file, "\t}\n" );
		}
		fprintf( file, "}\n" );
	}
	fprintf( file, "#pragma warning restore 0414\n" );

	fclose( file );
	return true;
}

bool GenCode::RanaType()
{
	vector<TEE::NodeBase*> options;
	TEE::GTEEMgr->GetNodesByType(TEE::NT_OPTION, options);

	FILE* file = NULL;
	errno_t err = fopen_s( &file, string(m_GenCPPPath+"/RanaType.h").c_str(), "w" );
	if ( file == NULL || err != 0 )
		return false;

	//title
	{
		static string s_Titles[] = 
		{
			"#pragma once\n",
			"#include \"RanaBaseTypes.h\"",
			"\n",
		};
		for ( int k=0; k < ARRAY_COUNT(s_Titles); ++k )
		{
			fprintf( file, s_Titles[k].c_str() );
			fprintf( file, "\n" );
		}
	}
	//content
	{
		vector<TEE::NodeBase*>::iterator iter(options.begin());
		for ( ; iter != options.end(); ++iter )
		{
			const TEE::Option* pOption = static_cast<const TEE::Option*>(*iter);
			string typeValue = GetValueTypeCodeName(pOption);
			string pName = FTCHARToANSI(pOption->CodeName().c_str());
			fprintf( file, "struct  RNT_%s\n", pName.c_str() );
			fprintf( file, "{\n" );
			fprintf( file, "\ttypedef %s TYPE;\n", typeValue.c_str() );
			fprintf( file, "};\n" );
		}
	}

	fclose( file );
	return true;
}
bool GenCode::RanaTask()
{
	FILE* file = NULL;
	errno_t err = fopen_s( &file, string(m_GenCPPPath+"/r_LuaApi.cpp").c_str(), "w" );	//TODO:"f:/GW/test/LuaCli/Decl/r_Battle.cpp"
	if ( file == NULL || err != 0 )
		return false;
	//title
	{
		static string s_Titles[] = 
		{
			"#include <LuaBind/RanaSpace.h>\n",
			"#include <LuaBind/RanaPush.h>",
			"#include <LuaBind/RanaInputStack.h>",
			"#include <Vek/Base/Exception.h>",
			"#include \"RanaSpaceType.h\"",
			"#include \"RanaType.h\"",
			"\n",
		};
		for ( int k=0; k < ARRAY_COUNT(s_Titles); ++k )
		{
			fprintf( file, s_Titles[k].c_str() );
			fprintf( file, "\n" );
		}
	}

	vector<TEE::NodeBase*> functions;
	TEE::GTEEMgr->GetNodesByType(TEE::NT_FUNCTION, functions);

	//content
	{
		vector<TEE::NodeBase*>::iterator iter(functions.begin());
		for ( ; iter != functions.end(); ++iter )
		{
			const TEE::NodeBase* pFunction = (*iter);
			if( TEE::IsInternalType( pFunction->CodeName() ) )
				continue;
			string pCodeName = FTCHARToANSI(pFunction->CodeName().c_str());
			string pDisplayName = FTCHARToANSI(pFunction->DisplayName().c_str());
			fprintf( file, "//%s\n", pDisplayName.c_str() );
			wstring cppCode;
			if( pFunction->GetCPPCode( cppCode ) )
			{
				string strCode = FTCHARToANSI(cppCode.c_str());
				fprintf( file, "%s;\n", strCode.c_str() );
				//Register
				vector<wstring>	spaces = TEE::GTEEMgr->GetPertainSpaces( pFunction );
				vector<wstring>::iterator	iterSpace(spaces.begin());
				for( ; iterSpace != spaces.end(); ++iterSpace )
				{
					string SpaceName = FTCHARToANSI((*iterSpace).c_str()); 
					fprintf( file, "REG_RANA_FUNC(RANA_SPACETYPE_%s, %s);\n", SpaceName.c_str(), pCodeName.c_str() );
				}
				if (TEE::NoGenerateCode(pFunction->CodeName()))
					continue;

				fprintf( file, "int l_%s(RanaState L)\n", pCodeName.c_str() );
				fprintf( file, "{\n" );

				TEE::NodeBase* pChile = pFunction->FirstChild();
				for( ; pChile != NULL; pChile = pChile->Next() )
				{
					if( pChile->ClassType() == TEE::NT_PARAMETER )
					{
						string pChildCodeName = FTCHARToANSI(pChile->CodeName().c_str());
						string pChildDisplayName = FTCHARToANSI(pChile->DisplayName().c_str());
						fprintf( file, ("\tRNT_%s::TYPE %s;\n"), pChildDisplayName.c_str(), pChildCodeName.c_str() );
					}
				}
				fprintf( file, "\tRANA_INPUT_BEGIN\n");
				fprintf( file, "\ts" );
				pChile = pFunction->FirstChild();
				for( ; pChile != NULL; pChile = pChile->Next() )
				{
					if( pChile->ClassType() == TEE::NT_PARAMETER )
					{
						string pChildCodeName = FTCHARToANSI(pChile->CodeName().c_str());
						fprintf( file, ">>%s", pChildCodeName.c_str() );
					}
				}
				fprintf( file, ";\n" );
				fprintf( file, "\tRANA_INPUT_END(%s)\n", pCodeName.c_str() );

				if (pFunction->IsVoidFunction())
				{
					fprintf(file, "\tr_%s(L", pCodeName.c_str());
					pChile = pFunction->FirstChild();
					for (; pChile != NULL; pChile = pChile->Next())
					{
						if (pChile->ClassType() == TEE::NT_PARAMETER)
						{
							string pChildCodeName = FTCHARToANSI(pChile->CodeName().c_str());
							fprintf(file, ",%s", pChildCodeName.c_str());
						}
					}
					fprintf(file, ");\n");
					fprintf(file, "\treturn 0;\n");
				}
				else
				{
					fprintf(file, "\trana_push( L, r_%s(L", pCodeName.c_str());
					pChile = pFunction->FirstChild();
					for (; pChile != NULL; pChile = pChile->Next())
					{
						if (pChile->ClassType() == TEE::NT_PARAMETER)
						{
							string pChildCodeName = FTCHARToANSI(pChile->CodeName().c_str());
							fprintf(file, ",%s", pChildCodeName.c_str());
						}
					}
					fprintf(file, ") );\n");
					fprintf(file, "\treturn 1;\n");
				}

				fprintf( file, "}\n" );
			}
		}
	}
	fclose( file );
	return true;
}

bool GenCode::RanaTaskCS()
{
	FILE* file = NULL;
	errno_t err = fopen_s( &file, string(m_GenCPPPath+"/r_LuaApi.cs").c_str(), "w" );	//TODO:"f:/GW/test/LuaCli/Decl/r_Battle.cpp"
	if ( file == NULL || err != 0 )
		return false;
	//include
	{
		fprintf( file, "using System;\n" );
		fprintf( file, "using System.Collections.Generic;\n" );
	}

	fprintf( file, "\n#pragma warning disable 0414 //disable warning CS0414: The private field XXX is assigned but its value is never used\n" );
	fprintf( file, "public partial class LuaAPISet\n" );
	fprintf( file, "{\n" );

	//functions
	{
		vector<TEE::NodeBase*> functions;
		TEE::GTEEMgr->GetNodesByType(TEE::NT_FUNCTION, functions);

		vector<TEE::NodeBase*>::iterator iter(functions.begin());
		for ( ; iter != functions.end(); ++iter )
		{
			const TEE::NodeBase* pFunction = (*iter);
			if( TEE::IsInternalType( pFunction->CodeName() ) )
				continue;
			string funcName = FTCHARToANSI(pFunction->CodeName().c_str());
			string pDisplayName = FTCHARToANSI(pFunction->DisplayName().c_str());
			fprintf( file, "//%s\n", pDisplayName.c_str() );
			wstring cppCode;
			if( pFunction->GetCPPCode( cppCode ) )
			{
				string strCode = FTCHARToANSI(cppCode.c_str());
				fprintf( file, "//%s;\n", strCode.c_str() );
				//Register
				vector<wstring>	spaces = TEE::GTEEMgr->GetPertainSpaces( pFunction );
				vector<wstring>::iterator	iterSpace(spaces.begin());
				for( ; iterSpace != spaces.end(); ++iterSpace )
				{
					string SpaceName = FTCHARToANSI((*iterSpace).c_str()); 
					fprintf( file, "private int %s_%s = RanaSpace.AddLuaAPI((int)ERanaSpaceType.%s, \"%s\", l_%s);\n", 
						funcName.c_str(), SpaceName.c_str(), SpaceName.c_str(), funcName.c_str(), funcName.c_str() );
				}
				if (TEE::NoGenerateCode(pFunction->CodeName()))
					continue;
				fprintf( file, "public static int l_%s(IntPtr L)\n", funcName.c_str() );
				fprintf( file, "{\n" );

				TEE::NodeBase* pChild = pFunction->FirstChild();
				for( ; pChild != NULL; pChild = pChild->Next() )
				{
					if( pChild->ClassType() == TEE::NT_PARAMETER )
					{
						string pChildCodeName = FTCHARToANSI(pChild->CodeName().c_str());
						string typeName = GetValueTypeCodeName(pChild);
						fprintf( file, ("\t%s %s;\n"), typeName.c_str(), pChildCodeName.c_str() );
					}
				}
				if (pFunction->ChildCount(TEE::NT_PARAMETER) > 0)
				{
					fprintf( file, "\tRanaInputStack _s = new RanaInputStack(L);\n");
					pChild = pFunction->FirstChild();
					for( ; pChild != NULL; pChild = pChild->Next() )
					{
						if( pChild->ClassType() == TEE::NT_PARAMETER )
						{
							string pChildCodeName = FTCHARToANSI(pChild->CodeName().c_str());
							fprintf( file, "\t_s.Read(out %s);\n", pChildCodeName.c_str() );
						}
					}
				}

				if (pFunction->IsVoidFunction())
				{
					fprintf(file, "\tr_%s(L", funcName.c_str());
					pChild = pFunction->FirstChild();
					for (; pChild != NULL; pChild = pChild->Next())
					{
						if (pChild->ClassType() == TEE::NT_PARAMETER)
						{
							string pChildCodeName = FTCHARToANSI(pChild->CodeName().c_str());
							fprintf(file, ",%s", pChildCodeName.c_str());
						}
					}
					fprintf(file, ");\n");
					fprintf(file, "\treturn 0;\n");
				}
				else
				{
					fprintf(file, "\tRana.Push( L, r_%s(L", funcName.c_str());
					pChild = pFunction->FirstChild();
					for (; pChild != NULL; pChild = pChild->Next())
					{
						if (pChild->ClassType() == TEE::NT_PARAMETER)
						{
							string pChildCodeName = FTCHARToANSI(pChild->CodeName().c_str());
							fprintf(file, ",%s", pChildCodeName.c_str());
						}
					}
					fprintf(file, ") );\n");
					fprintf(file, "\treturn 1;\n");
				}

				fprintf( file, "}\n" );
			}
		}
	}

	fprintf( file, "}\n" );
	fprintf( file, "#pragma warning restore 0414\n" );

	fclose( file );
	return true;
}

bool GenCode::RanaSpaceType()
{
	FILE* file = NULL;
	errno_t err = fopen_s( &file, string(m_GenCPPPath+"/RanaSpaceType.h").c_str(), "w" );
	if ( file == NULL || err != 0 )
		return false;

	//title
	{
		static string s_Titles[] = 
		{
			"#pragma once\n",
		};
		for ( int k=0; k < ARRAY_COUNT(s_Titles); ++k )
		{
			fprintf( file, s_Titles[k].c_str() );
			fprintf( file, "\n" );
		}
	}
	//content
	{
		fprintf( file, "enum RANA_SPACETYPE\n" );
		fprintf( file, "{\n" );

		//Register
		vector<TEE::NodeBase*>	spaces;
		TEE::GTEEMgr->SpaceRoot()->FindChildren(TEE::VerifyNodeType(TEE::NT_SPACEROOT), spaces);
		vector<TEE::NodeBase*>::iterator iterSpace(spaces.begin());
		for( ; iterSpace != spaces.end(); ++iterSpace )
		{
			string SpaceName = FTCHARToANSI((*iterSpace)->CodeName().c_str()); 
			string Commont = FTCHARToANSI((*iterSpace)->DisplayName().c_str()); 
			fprintf( file, "\tRANA_SPACETYPE_%s,//%s\n", SpaceName.c_str(), Commont.c_str() );
		}
		fprintf( file, "};\n" );
	}

	fclose( file );
	return true;
}

bool GenCode::RanaSpaceTypeCS()
{
	FILE* file = NULL;
	errno_t err = fopen_s( &file, string(m_GenCPPPath+"/RanaSpaceType.cs").c_str(), "w" );
	if ( file == NULL || err != 0 )
		return false;

	//content
	{
		fprintf( file, "public enum ERanaSpaceType\n" );
		fprintf( file, "{\n" );

		//Register
		vector<TEE::NodeBase*>	spaces;
		TEE::GTEEMgr->SpaceRoot()->FindChildren(TEE::VerifyNodeType(TEE::NT_SPACEROOT), spaces);
		vector<TEE::NodeBase*>::iterator iterSpace(spaces.begin());
		for( ; iterSpace != spaces.end(); ++iterSpace )
		{
			string SpaceName = FTCHARToANSI((*iterSpace)->CodeName().c_str()); 
			string Commont = FTCHARToANSI((*iterSpace)->DisplayName().c_str()); 
			fprintf( file, "\t%s,//%s\n", SpaceName.c_str(), Commont.c_str() );
		}
		fprintf( file, "\tSpace_Num,\n" );
		fprintf( file, "};\n" );
	}

	fclose( file );
	return true;
}