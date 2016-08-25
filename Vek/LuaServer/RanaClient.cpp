#ifndef ZOIC_NO_SERVICE
#include <Zoic/include/NetLog.h>
#endif
#include <Zoic/include/Log.h>
#include <LuaBind/RanaSpaceBank.h>
#include "include/RanaClient.h"
#include "include/AccessMask.h"
#include "include/RanaService.h"

using namespace Zoic;
using namespace RanaProto;

#include <sys/types.h>
#include <sys/stat.h>

#include <Zoic/include/SysPing.h>

RanaClient::RanaClient():
	m_ranaAccess(0),
	m_loginState(LS_NONE),
	m_pCurWriteScriptBuf(NULL),
	m_pCurWriteSaveFileBuf(NULL),
	m_pCurLoadScriptBuf(NULL),
	m_curReceiveActionBuf(NULL)
{
	{//把ping时间改改长
		const BYTE timeSize = 255;//SYSPING_KEEPCNT+1;
		DWORD timeData[timeSize];
		DWORD tm=0;

		const DWORD maxClientSysPingTime = 100000;
		const DWORD clientSysPingKeepIdle = maxClientSysPingTime/2;
		const DWORD clientSysPingKeepIntvl = clientSysPingKeepIdle/SYSPING_KEEPCNT;
		tm = clientSysPingKeepIdle;
		for(int i=0;i<timeSize;++i)
		{
			timeData[i]=tm;
			tm+=clientSysPingKeepIntvl;
		}

		static Zoic::SysPingTime ranaClientSysPingTime;
		ranaClientSysPingTime.init(timeSize,timeData);
		setSysPingTime(&ranaClientSysPingTime);
	}
	
	m_sendbuf_dynamic = true;
}

RanaClient::~RanaClient()
{
	if(m_pCurWriteScriptBuf)
	{
		delete [] m_pCurWriteScriptBuf;
	}
	if(m_pCurWriteSaveFileBuf)
	{
		delete [] m_pCurWriteSaveFileBuf;
	}
	if(m_pCurLoadScriptBuf)
	{
		delete [] m_pCurLoadScriptBuf;
	}
}

bool RanaClient::parseMessage(Zoic::Message &msg)
try
{
#ifndef ZOIC_NO_SERVICE
	netlog(m_netlogName.c_str(), getPort(),"",getLocalPort() ,msg.getData(),msg.getSize(),true);
#endif
	return SessionS::parseMessage(msg);
}
catch(std::exception * e)
{
	dprintf("RanaClient[%d,%d] exception:%s\n",getHandle(),getSerial(),e->what());
	delete e;
	return false;
}

bool RanaClient::sendMessage(const Zoic::Message &msg)
{
#ifndef ZOIC_NO_SERVICE
	netlog(m_netlogName.c_str(), getPort(),"",getLocalPort(),msg.getData(),msg.getSize(),false);
#endif
	return SessionS::sendMessage(msg);
}

bool RanaClient::open(SOCKET sock,const sockaddr_in &addr)
{
	bool ret= SessionS::open(sock,addr);
	if(ret)
	{
		m_netlogName = "RanaClient_";
		m_netlogName += getIP();
		dlog_f("RanaClient[%d,%d] open on %s:%d\n",getHandle(),getSerial(),getIP(),getPort());
	}
	return ret;
}

void RanaClient::close()
{
	dlog_f("RanaClient[%d,%d] close on %s:%d\n",getHandle(),getSerial(),getIP(),getPort());
	if(m_pCurWriteScriptBuf)
	{
		delete [] m_pCurWriteScriptBuf;
		m_pCurWriteScriptBuf = NULL;
	}
	if(m_pCurWriteSaveFileBuf)
	{
		delete [] m_pCurWriteSaveFileBuf;
		m_pCurWriteSaveFileBuf = NULL;
	}
	if(m_pCurLoadScriptBuf)
	{
		delete [] m_pCurLoadScriptBuf;
		m_pCurLoadScriptBuf = NULL;
	}
	SessionS::close();
}

void RanaClient::sendLoginAck(RanaProto::ErrorType code)
{
	P_ST_Login_Ack pktSend;
	pktSend.code = code;
	Message msgSend;
	msgSend.ID(ST_LOGIN_ACK);
	msgSend << pktSend;
	sendMessage(msgSend);
}

void RanaClient::sendSaveSpaceAck(RanaProto::ErrorType code)
{

}

#ifdef HAVE_MINGW32
#include <direct.h>
#else
#endif

static void switchPathFormat(const char* path)
{
	if( const char* pbuf = strstr(path,"\\") )
	{
		do 
		{
			const_cast<char&>(*pbuf) = '/';
		} while( pbuf = strstr(pbuf,"\\") );
	}
}

static FILE * fopen(const char *filename)
{
#ifdef HAVE_MINGW32
#else
	switchPathFormat(filename);
#endif
	FILE *fp=::fopen(filename,"wb");
	if(!fp)
	{
		char* pbuf = new char[strlen(filename)+1];
		strcpy(pbuf,filename);
		for(int i=1;pbuf[i];i++)
		{
			if(pbuf[i]=='/'
			#ifdef HAVE_MINGW32
				|| pbuf[i]=='\\'
			#else
			#endif
				)
			{
				pbuf[i]='\0';
			#ifdef HAVE_MINGW32
				_mkdir(pbuf);
			#else
				mkdir(pbuf,0777);
			#endif
				pbuf[i]='/';
			}
		}
		fp=::fopen(filename,"wb");
	}
	return fp;
}

bool RanaClient::validatePath(const char* path,const char* scriptName)
{
	if(!path || (m_pCurWriteScriptBuf && m_pCurWriteSaveFileBuf) )
	{
		return false;
	}
	if(strstr(path,"..")||strstr(path,":"))
	{
		return false;
	}
	if( strstr(path,"=") || strstr(scriptName,":") )
	{
		return false;
	}
	return true;
}

int RanaClient::scriptWriteBegin(const char* path, DWORD scriptLength, DWORD saveFileLength ,const char* scriptName, BYTE scriptType)
{
	if(!validatePath(path,scriptName))
	{
		return -1;
	}
	const RanaScriptManager::RanaScriptType* pScriptType = g_RanaScriptManager->findItem(path);
	if(pScriptType)
	{
		if(pScriptType->spaceType != scriptType)
		{
			return -1;
		}
	}
	switchPathFormat(path);
	m_curWriteScript.path = path;
	m_curWriteScript.scriptName = scriptName;
	m_curWriteScript.spaceType = scriptType;
	m_curWriteScriptPath = "";
	const char* pBasePath = g_RanaService->getStrategy()->GetBaseScriptPath();
	if(pBasePath)
	{
		m_curWriteScriptPath = pBasePath;
	}
	m_curWriteScriptPath += path;
	m_curWriteSaveFilePath = m_curWriteScriptPath;
	m_curWriteScriptPath += LUA_SCRIPT_EXT;
	m_curWriteSaveFilePath += LUA_SAVEFILE_EXT;
	m_curWriteScriptLength = scriptLength;
	if(m_pCurWriteScriptBuf)
	{
		delete [] m_pCurWriteScriptBuf;
	}
	m_pCurWriteScriptBuf = new BYTE[m_curWriteScriptLength+1];
	m_pCurWriteScriptBuf[m_curWriteScriptLength] = '\0';
	m_curWriteScriptPosition = 0;

	m_curWriteSaveFileLength = saveFileLength;
	if(m_pCurWriteSaveFileBuf)
	{
		delete [] m_pCurWriteSaveFileBuf;
	}
	m_pCurWriteSaveFileBuf = new BYTE[m_curWriteSaveFileLength+1];
	m_pCurWriteSaveFileBuf[m_curWriteSaveFileLength] = '\0';
	m_curWriteSaveFilePosition = 0;
	return 0;
}

void RanaClient::scriptWriteScriptAppend( const Rana_ScriptContent& content)
{
	if(!m_pCurWriteScriptBuf)
	{
		return;
	}
	const char* pContent = content;
	DWORD nLen = (DWORD)strlen(pContent);
	long nCurMaxLen = nLen + m_curWriteScriptPosition;
	if(static_cast<DWORD>(nCurMaxLen)>m_curWriteScriptLength)
	{
		delete [] m_pCurWriteScriptBuf;
		m_pCurWriteScriptBuf = NULL;
		if(m_pCurWriteSaveFileBuf)
		{
			delete [] m_pCurWriteSaveFileBuf;
			m_pCurWriteSaveFileBuf = NULL;
		}
		return;
	}
	memcpy( m_pCurWriteScriptBuf+m_curWriteScriptPosition, pContent, nLen*sizeof(BYTE) );
	m_curWriteScriptPosition+=nLen;
}

void RanaClient::scriptWriteSaveFileAppend( const Rana_ScriptContent& content)
{
	if(!m_pCurWriteSaveFileBuf)
	{
		return;
	}
	const char* pContent = content;
	DWORD nLen = (DWORD)strlen(pContent);
	long nCurMaxLen = nLen + m_curWriteSaveFilePosition;
	if(static_cast<DWORD>(nCurMaxLen)>m_curWriteSaveFileLength)
	{
		if(m_pCurWriteScriptBuf)
		{
			delete [] m_pCurWriteScriptBuf;
			m_pCurWriteScriptBuf = NULL;
		}
		delete [] m_pCurWriteSaveFileBuf;
		m_pCurWriteSaveFileBuf = NULL;
		return;
	}
	memcpy( m_pCurWriteSaveFileBuf+m_curWriteSaveFilePosition, pContent, nLen*sizeof(BYTE) );
	m_curWriteSaveFilePosition+=nLen;
}

static bool saveBuffToFile(const char* path, const void* pBuff, DWORD length)
{
	FILE* pf = fopen(path);
	if(!pf)
	{
		return false;
	}
	DWORD nWriteNum =  (DWORD)fwrite(pBuff,sizeof(BYTE),length,pf);
	fclose(pf);
	if( nWriteNum != length )
	{
		return false;
	}
	return true;
}

int RanaClient::scriptWriteSave()
{
	if(!m_pCurWriteScriptBuf || !m_pCurWriteSaveFileBuf)
	{
		return -1;
	}
	int res = -1;
	if(m_curWriteScriptLength ==m_curWriteScriptPosition && m_curWriteSaveFileLength ==m_curWriteSaveFilePosition)
	{
		if( saveBuffToFile( m_curWriteScriptPath.c_str(),m_pCurWriteScriptBuf,m_curWriteScriptLength ) &&
			saveBuffToFile( m_curWriteSaveFilePath.c_str(),m_pCurWriteSaveFileBuf,m_curWriteSaveFileLength ) )
		{
			if(g_RanaScriptManager->setScriptInfo(m_curWriteScript.path,m_curWriteScript.scriptName,m_curWriteScript.spaceType) )
			{
				res = 0;
			}
		}
	}
	delete [] m_pCurWriteScriptBuf;
	m_pCurWriteScriptBuf = NULL;
	delete [] m_pCurWriteSaveFileBuf;
	m_pCurWriteSaveFileBuf = NULL;
	return res;
}

int RanaClient::fileLoad( const char* filename )
{
	if(m_pCurLoadScriptBuf)
	{
		return -1;
	}
	if(strstr(filename,"..")||strstr(filename,":"))
	{
		return -1;
	}
	m_curLoadScriptPath = "";
	const char* pBasePath = g_RanaService->getStrategy()->GetBaseScriptPath();
	if(pBasePath)
	{
		m_curLoadScriptPath = pBasePath;
	}
	m_curLoadScriptPath += filename;
	int res = -1;
	FILE* pf = fopen(m_curLoadScriptPath.c_str(),"rb");
	if(pf)
	{
		int nFileLen=0;
#ifdef HAVE_MINGW32
		struct _stat stStat;
		stStat.st_size = 0;
		_stat(m_curLoadScriptPath.c_str(), &stStat);
#else
		struct stat stStat;
		stStat.st_size = 0;
		stat(m_curLoadScriptPath.c_str(), &stStat);
#endif
		nFileLen = stStat.st_size;
		if(nFileLen>0)
		{
			m_curLoadScriptLength = nFileLen;
			m_pCurLoadScriptBuf = new BYTE[m_curLoadScriptLength+1];
			DWORD nReadNum =  (DWORD)fread(m_pCurLoadScriptBuf,1,m_curLoadScriptLength,pf);
			if( nReadNum == m_curLoadScriptLength )
			{
				m_pCurLoadScriptBuf[m_curLoadScriptLength] = '\0';
				res = 0;
			}
			else
			{
				delete [] m_pCurLoadScriptBuf;
				m_pCurLoadScriptBuf = NULL;
			}
		}
		fclose(pf);
	}
	return 0;
}

DWORD RanaClient::fileLoadGetLength()
{
	if(!m_pCurLoadScriptBuf)
	{
		return 0;
	}
	return m_curLoadScriptLength;
}

void RanaClient::fileLoadSend(WORD protoCode)
{
	if(!m_pCurLoadScriptBuf)
	{
		return;
	}
	P_TS_DownloadScriptContent_Ntf pktSend;
	const char* pBuf = (const char*)m_pCurLoadScriptBuf;
	DWORD leftLength = m_curLoadScriptLength;
	Message msgSend;
	while(leftLength > 3000)
	{
		pktSend.content = pBuf;
		leftLength -= 3000;
		pBuf += 3000;
		msgSend.ID(protoCode)<<pktSend;
		sendMessage(msgSend);
	}
	if(leftLength)
	{
		pktSend.content = pBuf;
		msgSend.clear();
		msgSend.ID(protoCode)<<pktSend;
		sendMessage(msgSend);
	}
	delete [] m_pCurLoadScriptBuf;
	m_pCurLoadScriptBuf = NULL;
}

static const char* const pFuncHead = "function ";
static const char* const pConditionHead = "_Conditions";
static const char* const pActionHead = "_Actions";
static const char* const pFuncStart = "()\n";
static const char* const pFuncEnd = "\nend";
static const char* const pFuncClean = " = nil;\n";
static const WORD nConditionAddtion = (WORD)(strlen(pFuncHead) + strlen(pConditionHead) + strlen(pFuncStart) + strlen(pFuncEnd));
static const WORD nActionAddtion = (WORD)(strlen(pFuncHead) + strlen(pActionHead) + strlen(pFuncStart) + strlen(pFuncEnd));
static const WORD nMaxClearAddtion = (WORD)(strlen(pConditionHead) + (WORD)strlen(pFuncClean));
static const char* const pEmptyCondition = "return true;";
static const char* const pEmptyAction = "";

int RanaClient::createTrigger( WORD space, const char*  triggerName )//const P_TS_CreateTrigger_Req&  triggerContext )
{
	P_ST_CreateTrigger_Ack pktSend;
	pktSend.space = space;
	if( checkAccess(RSAM_TRIGGER_CREATE) )
	{
		RanaSpace* pRanaSpace = RanaSpaceBank::getInstance()->forceFindSpaceByIndex(space);
		if(pRanaSpace)
		{
			RanaTriggerBank* pRanaTriggerBank = pRanaSpace->getTriggerBank();
			if(pRanaTriggerBank)
			{
				if(!pRanaTriggerBank->getTrigger( (const RanaTriggerName)(triggerName) ) )
				{
					pktSend.code = ERROR_FAIL;
					WORD conditionLen = (WORD)strlen(pEmptyCondition);
					WORD actionLen = (WORD)strlen(pEmptyAction);
					WORD nameLen = (WORD)strlen(triggerName);
					DWORD maxConditionLen =  nConditionAddtion + nameLen + conditionLen;
					DWORD maxActionLen =  nActionAddtion + nameLen + actionLen;

					if( maxConditionLen<RANA_MAX_FILE_LENGTH && maxActionLen<RANA_MAX_FILE_LENGTH )
					{
						WORD maxScriptLen = (WORD)(maxConditionLen>maxActionLen ? maxConditionLen : maxActionLen);
						char* pBuf = new char[maxScriptLen+1];
						strcpy(pBuf,pFuncHead);
						strcat(pBuf,triggerName);
						strcat(pBuf,pConditionHead);
						strcat(pBuf,pFuncStart);
						strcat(pBuf,pEmptyCondition);
						strcat(pBuf,pFuncEnd);
						int res = pRanaSpace->runBuf(pBuf);
						if(res == 0)
						{
							strcpy(pBuf,pFuncHead);
							strcat(pBuf,triggerName);
							strcat(pBuf,pActionHead);
							strcat(pBuf,pFuncStart);
							strcat(pBuf,pEmptyAction);
							strcat(pBuf,pFuncEnd);
							res = pRanaSpace->runBuf(pBuf);
						}
						if(res == 0)
						{
							RanaTrigger* pRanaTrigger = new RanaTrigger;
							pRanaTrigger->setName((const RanaTriggerName)(triggerName));
							pRanaTrigger->setOnOff(true);
							if( pRanaTriggerBank->addTrigger(pRanaTrigger) == 0)
							{
								pktSend.handle = pRanaTrigger->getHandle();
								pktSend.code = ERROR_OK;
							}
						}
						else
						{
							strcpy(pBuf,triggerName);
							strcat(pBuf,pConditionHead);
							strcat(pBuf,pFuncClean);
							pRanaSpace->runBuf(pBuf);
							strcpy(pBuf,triggerName);
							strcat(pBuf,pActionHead);
							strcat(pBuf,pFuncClean);
							pRanaSpace->runBuf(pBuf);
						}
						delete [] pBuf;
					}
				}//end of "if(!pRanaTriggerBank->getTrigger(triggerContext.name))"
				else
				{
					pktSend.code = ERROR_RANA_TRIGGER_ALREADY_EXISTS;
				}
			}
			else
			{
				pktSend.code = ERROR_RANA_SPACE;
			}
		}//end of "if(if(pRanaSpace))"
		else
		{
			pktSend.code = ERROR_RANA_SPACE;
		}
	}
	else
	{
		pktSend.code = ERROR_RANA_ACCESS;
	}
	Message msgSend;
	msgSend.ID(ST_CREATETRIGGER_ACK);
	msgSend << pktSend;
	sendMessage(msgSend);

	return 0;
}

/*
int RanaClient::deleteTrigger( WORD space, WORD handle)
{
	P_ST_DeleteTrigger_Ack pktSend;
	if( checkAccess(RSAM_TRIGGER_DELETE) )
	{
		RanaSpace* pRanaSpace = RanaSpaceBank::getInstance()->forceFindSpaceByIndex(space);
		if(pRanaSpace)
		{
			RanaTriggerBank* pRanaTriggerBank = pRanaSpace->getTriggerBank();
			RanaEventBank* pRanaEventBank = pRanaSpace->getEventBank();
			if(pRanaTriggerBank && pRanaEventBank)
			{
				RanaTrigger* pTrigger = pRanaTriggerBank->getTrigger(handle);
				if(pTrigger)
				{
					const RanaTriggerName triggerName = pTrigger->getName();
					if( pRanaTriggerBank->deleteTrigger(handle) == 0 )
					{
						WORD nScriptLen = (WORD)(nMaxClearAddtion + strlen(triggerName));
						char* pScriptBuf = new char[nScriptLen+1];
						strcpy(pScriptBuf,triggerName);
						strcat(pScriptBuf,pConditionHead);
						strcat(pScriptBuf,pFuncClean);
						pRanaSpace->runBuf(pScriptBuf);
						strcpy(pScriptBuf,triggerName);
						strcat(pScriptBuf,pActionHead);
						strcat(pScriptBuf,pFuncClean);
						pRanaSpace->runBuf(pScriptBuf);
						delete [] pScriptBuf;
						pktSend.code = ERROR_OK;
					}
					else
					{
						pktSend.code = ERROR_RANA_TRIGGER;
					}
				}
				else
				{
					pktSend.code = ERROR_RANA_TRIGGER;
				}
			} // end of "if(pRanaTriggerBank && pRanaEventBank)"
			else
			{
				pktSend.code = ERROR_RANA_SPACE;
			}
		}
		else
		{
			pktSend.code = ERROR_RANA_SPACE;
		}
	}
	else
	{
		pktSend.code = ERROR_RANA_ACCESS;
	}
	Message msgSend;
	msgSend.ID(ST_DELETETRIGGER_ACK);
	msgSend << pktSend;
	sendMessage(msgSend);

	return 0;
}*/


int RanaClient::modifyTriggerCondition( WORD space, WORD handle, const char* condition)
{
	P_ST_ModifyTriggerCondition_Ack pktSend;
	if( checkAccess(RSAM_TRIGGER_DELETE) )
	{
		RanaSpace* pRanaSpace = RanaSpaceBank::getInstance()->forceFindSpaceByIndex(space);
		if(pRanaSpace)
		{
			RanaTriggerBank* pRanaTriggerBank = pRanaSpace->getTriggerBank();
			if(pRanaTriggerBank)
			{
				RanaTrigger* pTrigger = pRanaTriggerBank->getTrigger(handle);
				if(pTrigger)
				{
					pktSend.code = ERROR_FAIL;
					const RanaTriggerName &triggerName = pTrigger->getName();
					WORD conditionLen = (WORD)strlen(condition);
					WORD nameLen = (WORD)strlen(triggerName);
					DWORD maxConditionLen =  nConditionAddtion + nameLen + conditionLen;

					if( maxConditionLen<RANA_MAX_FILE_LENGTH )
					{
						DWORD maxScriptLen = maxConditionLen;
						char* pBuf = new char[maxScriptLen+1];
						strcpy(pBuf,pFuncHead);
						strcat(pBuf,triggerName);
						strcat(pBuf,pConditionHead);
						strcat(pBuf,pFuncStart);
						strcat(pBuf,condition);
						strcat(pBuf,pFuncEnd);
						int res = pRanaSpace->runBuf(pBuf);
						if(res == 0)
						{
							pktSend.code = ERROR_OK;
						}
						delete pBuf;
					}
				}
				else
				{
					pktSend.code = ERROR_RANA_TRIGGER;
				}
			}
			else
			{
				pktSend.code = ERROR_RANA_SPACE;
			}
		}
		else
		{
			pktSend.code = ERROR_RANA_SPACE;
		}
	}
	else
	{
		pktSend.code = ERROR_RANA_ACCESS;
	}
	Message msgSend;
	msgSend.ID(ST_MODIFYTRIGGERCONDITION_ACK);
	msgSend << pktSend;
	sendMessage(msgSend);

	return 0;
}

int RanaClient::triggerActionReceiveBegin(WORD space, WORD handle, DWORD actionLength)
{
	RanaSpace* pRanaSpace = RanaSpaceBank::getInstance()->forceFindSpaceByIndex(space);
	if(!pRanaSpace)
	{
		return -1;
	}
	RanaTriggerBank* pRanaTriggerBank = pRanaSpace->getTriggerBank();
	if(!pRanaTriggerBank)
	{
		return -1;
	}
	if(actionLength >= RANA_MAX_BUFFER_LENGTH )
	{
		return -1;
	}
	if(m_curReceiveActionBuf)
	{
		delete [] m_curReceiveActionBuf;
	}
	m_curReceiveActionLength = actionLength;
	m_curReceiveActionSpace = space;
	m_curReceiveActionHandle = handle;

	m_curReceiveActionBuf = new char[actionLength+1];
	m_curReceiveActionBuf[actionLength] = '\0';
	m_curReceiveActionPosition = 0;

	return 0;
}

void RanaClient::triggerActionReceiveAppend( const Rana_TriggerAction& action)
{
	if(!m_curReceiveActionBuf)
	{
		return;
	}
	const char* pAction = action;
	DWORD nLen = (DWORD)strlen(pAction);
	long nCurMaxLen = nLen + m_curReceiveActionPosition;
	if(static_cast<DWORD>(nCurMaxLen)>m_curReceiveActionLength)
	{
		delete [] m_curReceiveActionBuf;
		m_curReceiveActionBuf = NULL;
		return;
	}
	memcpy( m_curReceiveActionBuf+m_curReceiveActionPosition, pAction, nLen*sizeof(char) );
	m_curReceiveActionPosition+=nLen;
}

int RanaClient::triggerActionCommit()
{
	if(!m_curReceiveActionBuf)
	{
		return -1;
	}
	int res = -1;
	if(m_curReceiveActionLength == m_curReceiveActionPosition)
	{
		res = modifyTriggerAction(m_curReceiveActionSpace,m_curReceiveActionHandle,m_curReceiveActionBuf);
	}
	delete [] m_curReceiveActionBuf;
	m_curReceiveActionBuf = NULL;
	return res;
}

int RanaClient::modifyTriggerAction(  WORD space, WORD handle, const char* action )
{
	P_ST_ModifyTriggerAction_Ack pktSend;
	if( checkAccess(RSAM_TRIGGER_DELETE) )
	{
		RanaSpace* pRanaSpace = RanaSpaceBank::getInstance()->forceFindSpaceByIndex(space);
		if(pRanaSpace)
		{
			RanaTriggerBank* pRanaTriggerBank = pRanaSpace->getTriggerBank();
			if(pRanaTriggerBank)
			{
				RanaTrigger* pTrigger = pRanaTriggerBank->getTrigger(handle);
				if(pTrigger)
				{
					pktSend.code = ERROR_FAIL;
					const RanaTriggerName &triggerName = pTrigger->getName();
					size_t actionLen = strlen(action);
					size_t nameLen = strlen(triggerName);
					size_t maxActionLen =  nActionAddtion + nameLen + actionLen;

					if( maxActionLen<RANA_MAX_FILE_LENGTH )
					{
						size_t maxScriptLen = maxActionLen;
						char* pBuf = new char[maxScriptLen+1];
						strcpy(pBuf,pFuncHead);
						strcat(pBuf,triggerName);
						strcat(pBuf,pActionHead);
						strcat(pBuf,pFuncStart);
						strcat(pBuf,action);
						strcat(pBuf,pFuncEnd);
						int res = pRanaSpace->runBuf(pBuf);
						if(res == 0)
						{
							pktSend.code = ERROR_OK;
						}
						delete pBuf;
					}
				}
				else
				{
					pktSend.code = ERROR_RANA_TRIGGER;
				}
			}
			else
			{
				pktSend.code = ERROR_RANA_SPACE;
			}
		}
		else
		{
			pktSend.code = ERROR_RANA_SPACE;
		}
	}
	else
	{
		pktSend.code = ERROR_RANA_ACCESS;
	}
	Message msgSend;
	msgSend.ID(ST_MODIFYTRIGGERACTION_ACK);
	msgSend << pktSend;
	sendMessage(msgSend);

	return 0;
}

template <class SENDER,class PKT>
class SenderMessageHelper
{
public:
	SenderMessageHelper(SENDER & sender,WORD id,const PKT &pkt):
		m_sender(sender),
		m_id(id),
		m_pkt(pkt)
	{
	};
	~SenderMessageHelper()
	{
		Message msgSend;
		msgSend.ID(m_id);
		msgSend << m_pkt;
		m_sender.sendMessage(msgSend);
	}
private:
	WORD			m_id;
	const PKT &		m_pkt;
	SENDER &		m_sender;
};

//生成事件参数
static bool makeEventArgs(const std::vector<Rana_Event::Argument> & src_args,std::vector<RanaVariant> & dest_args)
{
	RanaVariant arg;
	std::vector<Rana_Event::Argument>::const_iterator it;
	for(it = src_args.begin();it!=src_args.end();++it)
	{
		const Rana_Event::Argument& variantArgument = *it;
		switch(it->type)
		{
			case Rana_Event::Argument::AT_BOOL:
				arg.type=RanaVariant::VT_BOOL;
				arg.body.bVal=variantArgument.body.bVal?true:false;
				break;
			case Rana_Event::Argument::AT_INTEGER:
				arg.type=RanaVariant::VT_INTEGER;
				arg.body.iVal=variantArgument.body.iVal;
				break;
			case Rana_Event::Argument::AT_STRING:
				arg.type=RanaVariant::VT_STRING;
				arg.body.sVal=variantArgument.body.sVal;
				break;
			default:
				return false;
		}
		dest_args.push_back(arg);
	}
	return true;
}

//创建Rana事件
static RanaEvent* createEvent(const Rana_EventType & type,const std::vector<RanaVariant> & args)
{
	if(type.useCode)
	{
		RanaEventType eventType;
		if( g_RanaService->getStrategy()->GetEventTypeFromCode(type.code,eventType) )
		{
			return g_RanaService->getStrategy()->CreateRanaEvent(eventType,args);
		}
	}
	else
	{
		return g_RanaService->getStrategy()->CreateRanaEvent(type.id,args);
	}
	return NULL;
}

void RanaClient::createEvent( const P_TS_CreateEvent_Req&  eventContext )
{
	P_ST_CreateEvent_Ack pktSend;
	pktSend.space = eventContext.space;

	SenderMessageHelper<RanaClient,P_ST_CreateEvent_Ack> sendHelper(*this,ST_CREATEEVENT_ACK,pktSend);

	if( !checkAccess(RSAM_EVENT_CREATE) )
	{
		pktSend.code = ERROR_RANA_ACCESS;
		return;
	}

	RanaSpace* pRanaSpace = RanaSpaceBank::getInstance()->forceFindSpaceByIndex(eventContext.space);
	if(!pRanaSpace)
	{
		pktSend.code = ERROR_RANA_SPACE;
		return;
	}
	
	RanaEventBank* pRanaEventBank = pRanaSpace->getEventBank();
	RanaTriggerBank* pRanaTriggerBank = pRanaSpace->getTriggerBank();
	if(!pRanaEventBank || ! pRanaTriggerBank)
	{
		pktSend.code = ERROR_RANA_SPACE;
		return;
	}
	
	RanaTrigger* pRanaTrigger = pRanaTriggerBank->getTrigger(eventContext.triggerHandle);
	if(!pRanaTrigger)
	{
		pktSend.code = ERROR_RANA_TRIGGER;
		return;
	}

	std::vector<RanaVariant> args;
	if(!makeEventArgs(eventContext.event.args,args))
	{
		pktSend.code = ERROR_RANA_EVENT;
		return;
	}
	
	RanaEvent* pRanaEvent = ::createEvent(eventContext.event.type,args);
	if(!pRanaEvent)
	{
		pktSend.code = ERROR_RANA_EVENT;
		return;
	}
	
	pRanaEvent->setTriggerHandle(eventContext.triggerHandle);
	pRanaEvent->setOnOff(true);
#if 0	//FORNAME
	pRanaEvent->setName(static_cast<const char*>(eventContext.eventname) );
#endif
	if( 0 == pRanaSpace->registerEvent(pRanaEvent) )
	{
		pktSend.handle = pRanaEvent->getHandle();
		pktSend.code = ERROR_OK;
	}
	else
	{
		delete pRanaEvent;
		pktSend.code = ERROR_FAIL;
	}
}

/*
int RanaClient::deleteEvent( WORD space, WORD handle)
{
	P_ST_DeleteEvent_Ack pktSend;
	if( checkAccess(RSAM_EVENT_DELETE) )
	{
		RanaSpace* pRanaSpace = RanaSpaceBank::getInstance()->forceFindSpaceByIndex(space);
		if(pRanaSpace)
		{
			RanaEventBank* pRanaEventBank = pRanaSpace->getEventBank();
			if(pRanaEventBank)
			{
				if( pRanaEventBank->delEvent(handle) == 0 )
				{
					pktSend.code = ERROR_OK;
				}
				else
				{
					pktSend.code = ERROR_RANA_EVENT;
				}
			} // end of "if(pRanaTriggerBank && pRanaEventBank)"
			else
			{
				pktSend.code = ERROR_RANA_SPACE;
			}
		}
		else
		{
			pktSend.code = ERROR_RANA_SPACE;
		}
	}
	else
	{
		pktSend.code = ERROR_RANA_ACCESS;
	}
	Message msgSend;
	msgSend.ID(ST_DELETEEVENT_ACK);
	msgSend << pktSend;
	sendMessage(msgSend);

	return 0;
}*/


int RanaClient::saveSpace( DWORD spaceHandle)
{
	P_ST_SaveSpace_Ack pktSend;
	if( checkAccess(RSAM_SPACE_SAVE) )
	{
		RanaSpace* pRanaSpace = RanaSpaceBank::getInstance()->forceFindSpaceByIndex(static_cast<RanaWORD>(spaceHandle));
		if(pRanaSpace)
		{
			g_RanaService->getStrategy()->saveSpace(pRanaSpace);
		}
		else
		{
			pktSend.code = ERROR_RANA_SPACE;
		}
	}
	else
	{
		pktSend.code = ERROR_RANA_ACCESS;
	}
	Message msgSend;
	msgSend.ID(ST_SAVESPACE_ACK);
	msgSend << pktSend;
	sendMessage(msgSend);
	return 0;
}

int RanaClient::executeScript( DWORD spaceHandle, const Rana_ScriptContent& content)
{
	P_ST_ExecuteScript_Ack pktSend;
	if( checkAccess(RSAM_SCRIPT_EXECUTE) )
	{
		RanaSpace* pRanaSpace = RanaSpaceBank::getInstance()->forceFindSpaceByIndex(static_cast<RanaWORD>(spaceHandle));
		if(pRanaSpace)
		{
			if( pRanaSpace->runBuf(content) == 0 )
			{
				pktSend.code = ERROR_OK;
			}
			else
			{
				pktSend.code = ERROR_FAIL;
			}
		}
		else
		{
			pktSend.code = ERROR_RANA_SPACE;
		}
	}
	else
	{
		pktSend.code = ERROR_RANA_ACCESS;
	}
	Message msgSend;
	msgSend.ID(ST_EXECUTESCRIPT_ACK);
	msgSend << pktSend;
	sendMessage(msgSend);
	return 0;
}

RanaProto::Rana_ScriptPath RanaClient::swithPath2ScriptPath(const char* pPath)
{
	static const size_t extlen = strlen(LUA_SCRIPT_EXT);
	size_t len = strlen(pPath);
	len -= extlen;
	char pChar[1024];
	strcpy(pChar,pPath);
	if( len>=0 && !strcmp(pPath+len,LUA_SCRIPT_EXT) )
	{
		*(pChar+len) = '\0';
	}
	const char* pBasePath = g_RanaService->getStrategy()->GetBaseScriptPath();
	if(pBasePath)
	{
		size_t len = strlen(pBasePath);
		if(!memcmp(pChar,pBasePath,len) )
		{
			if(len>0)
			{
				if(pChar[len-1] == '\\' || pChar[len-1] == '/')
				{
					return (pChar+len);
				}
			}
			if(pChar[len] == '\\' || pChar[len] == '/')
			{
				return (pChar+len+1);
			}
		}
		else
		{
			return "";
		}
	}
	return pChar;
}

