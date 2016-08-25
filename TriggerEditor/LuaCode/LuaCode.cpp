#include "LuaCode.h"

#include "SpaceManager.h"
#include "Process_LuaProtocol.h"
#include "LuaClient.h"
#include "NodeVerify.h"
#include <wx/msgdlg.h>
#include <strsafe.h>
#include "TgrFrame.h"
#include "TgrApp.h"
#include "SVariable.h"

using namespace Space;

GenCode*	GGenCode = GenCode::GetInstance();

GenCode::GenCode()
{
}
bool	GenCode::Gen()
{
	bool bSuccess = true;
	bSuccess &= GenLuaCode();

	SNode* pRoot = GSpaceMgr->ChiefRoot();
	if( pRoot == NULL )
		return false;

	if( bSuccess )
	{
		if (wxGetApp().GetTgrFrame()->IsConnected())
		{
			LuaProto_Set::NetProcess::TS_ReloadSpace_Req( (const char*)FTCHARToANSI(pRoot->Name().c_str()), (const char*)FTCHARToANSI(pRoot->Name().c_str()) );
		}
		else
		{
			Log(_T("重启空间失败，请确认是否已和服务器连接！"));
		}
	}

	return bSuccess;
}
void	GenCode::SetGenerateLuaPath( const TCHAR* path )
{
	m_GenLuaPath = FTCHARToANSI(path);
}
bool	GenCode::GenLuaCode( )
{
	SNode* pRoot = GSpaceMgr->ChiefRoot();
	if( pRoot == NULL )
		return false;

	// base import
	bool bSuccess = false;
	FILE* baseInclude = NULL;
	errno_t err = fopen_s( &baseInclude, string("BaseInclude.lua").c_str(), "r" );
	if ( baseInclude == NULL || err != 0 )
		return bSuccess;

	string name = ( string(FTCHARToANSI(pRoot->Name().c_str())) + ".lua");

	FILE* file = NULL;
	err = fopen_s( &file, string(m_GenLuaPath+name).c_str(), "w" );
	if ( file == NULL || err != 0 )
		return bSuccess;

	char temp[1024+1];
	ZeroMemory( temp, sizeof(temp) ); 
	while( fread_s( temp, 1024*sizeof(char), sizeof(char), 1024, baseInclude ) > 0 )
	{
		fprintf( file, temp );
		ZeroMemory( temp, sizeof(temp) ); 
	}
	fprintf( file, "\n" );
	fclose( baseInclude );

	//main
	{
		fprintf(file, "\n");
		fprintf(file, "function main()\n");
		fprintf( file, "\tInitGlobals();\n" );
		fprintf( file, "end\n" );
	}

	//InitGlobals
	{
		fprintf(file, "\n");
		fprintf(file, "function InitGlobals()\n");

		vector<SNode*> VarItems;
		SNode* pVarSet = GSpaceMgr->FindRoot( SNT_VARSET, NULL );
		if (pVarSet) pVarSet->FindChildren(SVerifyAlwaysOk(), VarItems);

		vector<SNode*>::iterator iter(VarItems.begin());
		for( int iIndex = 0; iter != VarItems.end(); ++iter, ++iIndex )
		{
			SNode* pVarItem = (*iter);
			wstring luaCode;
			pVarItem->GetLuaCode(luaCode, 0);
			luaCode += TEXT("\n");
			fprintf(file, FTCHARToANSI(luaCode.c_str()));
		}

		fprintf( file, "end\n" );
	}

	//Triggers
	{
		SNode* pRoot = GSpaceMgr->FindRoot( SNT_ROOT, NULL );
		if( pRoot == NULL )
			goto END;

		set<DWORD> triggerIds;
		set<wstring> functionNames;
		string eventCode, triggerCode, functionCode;
		GenLuaCodeRecursively(pRoot, eventCode, triggerCode, functionCode, triggerIds, functionNames);
		eventCode =
			"function InitTriggers()\n" +
			eventCode +
			"end\n\n";

		fprintf(file, "\n");
		fprintf(file, functionCode.c_str());
		fprintf(file, eventCode.c_str());
		fprintf(file, triggerCode.c_str());
	}
	bSuccess = true;
END:
	fclose( file );
	return bSuccess;
}

void GenCode::GenLuaCodeRecursively(SNode* pRoot, string& eventCode, string& triggerCode, string& functionCode, set<DWORD>& triggerIds, set<wstring>& functionNames)
{
	bool bValidTriggerRoot = pRoot && (pRoot->ClassType() == SNT_ROOT || pRoot->ClassType() == SNT_GROUP);
	if (!bValidTriggerRoot)
	{
		return;
	}

	char temp[1024+1];
	for (SNode* pChild = pRoot->FirstChild(); pChild != NULL; pChild = pChild->Next())
	{
		if (!pChild->Working())
		{
			continue;
		}
		if (pChild->ClassType() == SNT_TRIGGER)
		{
			SNode* pTrigger = pChild;
			DWORD TriggerId = reinterpret_cast<DWORD>(pTrigger->UserData( "id" ));
			set<DWORD>::_Pairib pair_ = triggerIds.insert( TriggerId );
			if( !pair_.second )
			{
				wxMessageDialog dialog( NULL, _T("Trigger编号重复，请找程序员"), _T("错误"), wxOK|wxICON_ERROR);
				dialog.ShowModal();
			}

			//Event
			SNode* pEventRoot = pTrigger->FindFirstChild(SVerifyNodeType(SNT_EVENTROOT));
			for ( SNode* pEvent = pEventRoot->FirstChild(); pEvent != NULL; pEvent = pEvent->Next() )
			{
				if( !pEvent->Working() )
					continue;
				char* pName = FTCHARToANSI(pEvent->Name().c_str());
				StringCbPrintfA( temp, 1024, "\tTriggerRegister%sEvent( \"Trigger_%d\"", pName, TriggerId );
				eventCode += temp;
				for ( SNode* pRealParam = pEvent->FirstChild(); pRealParam != NULL; pRealParam = pRealParam->Next() )
				{
					wstring realParam;
					if( pRealParam->GetRealParam( realParam ) )
					{
						char* pRealParamString = FTCHARToANSI(realParam.c_str());
						StringCbPrintfA( temp, 1024, ", %s", pRealParamString );
						eventCode += temp;
					}
				}
				eventCode += " );\n";
			}

			//Condition
			SNode* pConditionRoot = pTrigger->FindFirstChild(SVerifyNodeType(SNT_CONDITIONROOT));
			if (pConditionRoot && pConditionRoot->Working())
			{
				StringCbPrintfA( temp, 1024, "function Trigger_%d_Conditions()\n", TriggerId );
				triggerCode += temp;
				for ( SNode* pCondition = pConditionRoot->FirstChild(); pCondition != NULL; pCondition = pCondition->Next() )
				{
					wstring luaCode;
					if( pCondition->GetLuaCode( luaCode, 0 ) )
					{
						luaCode = _T("\tif not(") + luaCode + _T(") then\n");
						luaCode += _T("\t\treturn false\n");
						luaCode += _T("\tend\n");
						triggerCode += FTCHARToANSI(luaCode.c_str());
					}
				}
				triggerCode += "\treturn true;\n";
				triggerCode += "end\n\n";
			}

			//Context parameters
			string contextParamsCode = "";
			SNode* pFirstEvent = pEventRoot->FirstChild();
			if (pFirstEvent && pFirstEvent->TEETmp())
			{
				vector<TEE::NodeBase*> contextParams;
				pFirstEvent->TEETmp()->FindChildren(TEE::VerifyNodeType(TEE::NT_CONTEXTPARAM), contextParams, true);
				if (contextParams.size() > 0)
				{
					for (unsigned int ui = 0; ui < contextParams.size(); ++ui)
					{
						TEE::NodeBase* pContextParam = contextParams[ui];
						contextParamsCode += FTCHARToANSI((PARAM_VAR_PREFIX + pContextParam->CodeName()).c_str());
						if (pContextParam->Next() != NULL)
						{
							contextParamsCode += ", ";
						}
					}
				}
			}

			//Action
			StringCbPrintfA( temp, 1024, "function Trigger_%d_Actions(%s)\n", TriggerId, contextParamsCode.c_str() );
			triggerCode += temp;
			SNode* pActionRoot = pTrigger->FindFirstChild(SVerifyNodeType(SNT_ACTIONROOT));
			for ( SNode* pAction = pActionRoot->FirstChild(); pAction != NULL; pAction = pAction->Next() )
			{
				if( !pAction->Working() )
					continue;

				wstring luaCode;
				if( pAction->GetLuaCode( luaCode, 1 ) )
				{
					triggerCode += FTCHARToUTF8(luaCode.c_str());
					triggerCode += "\n";
				}
			}
			triggerCode += "end\n\n";

		}
		else if (pChild->ClassType() == SNT_FUNCTION)
		{
			SNode* pFunction = pChild;
			set<wstring>::_Pairib pair_ = functionNames.insert(pFunction->Name());
			if (!pair_.second)
			{
				wxMessageDialog dialog(NULL, _T("函数名字重复，请找程序员"), _T("错误"), wxOK | wxICON_ERROR);
				dialog.ShowModal();
			}

			//Parameters
			string paramsCode = "";
			SNode* pParamsRoot = pFunction->FindFirstChild(SVerifyNodeType(SNT_PARAMROOT));
			for (SNode* pParamNode = pParamsRoot->FirstChild(); pParamNode; pParamNode = pParamNode->Next())
			{
				if (!pParamNode->IsA(SNT_VARIABLE)) continue;
				paramsCode += FTCHARToANSI((PARAM_VAR_PREFIX + pParamNode->Name()).c_str());
				if (pParamNode->Next() != NULL)
				{
					paramsCode += ", ";
				}
			}

			//Action
			StringCbPrintfA(temp, 1024, "function %s(%s)\n", (char*)FTCHARToANSI(pFunction->Name().c_str()), paramsCode.c_str());
			functionCode += temp;
			SNode* pActionRoot = pFunction->FindFirstChild(SVerifyNodeType(SNT_ACTIONROOT));
			for (SNode* pAction = pActionRoot->FirstChild(); pAction != NULL; pAction = pAction->Next())
			{
				if (!pAction->Working())
					continue;

				wstring luaCode;
				if (pAction->GetLuaCode(luaCode, 1))
				{
					functionCode += FTCHARToUTF8(luaCode.c_str());
					functionCode += "\n";
				}
			}
			functionCode += "end\n\n";

		}
		else if (pChild->ClassType() == SNT_GROUP)
		{
			GenLuaCodeRecursively(pChild, eventCode, triggerCode, functionCode, triggerIds, functionNames);
		}
	}
}

extern bool LuaCMain(vector<string> inputfiles, string outfile, bool bstrip_debug_info);

bool GenCode::CompileLua()
{
	SNode* pRoot = GSpaceMgr->ChiefRoot();
	if( pRoot == NULL )
		return false;

	string baseName = string(FTCHARToANSI(pRoot->Name().c_str()));
	string inputFile = m_GenLuaPath + baseName + ".lua";
	string outputFile = m_GenLuaPath + baseName + ".luac";

	vector<string> inputfiles;
	inputfiles.push_back(inputFile);
	return LuaCMain(inputfiles, outputFile, true);
}