#include "RanaMsgImpl.h"
#include "RanaStrategy.h"
#include <LuaServer/include/AccessMask.h>

namespace ENS_RanaEvent
{
	extern RanaEvent* createEvent(WORD type,const std::vector<RanaVariant>& args);
}

RanaStrategy::RanaStrategy()
{
	m_basePath = "";
}

//检查等录用的函数，检查用户名密码是否合法，并通过access输出相应用户的权限
int RanaStrategy::certifcate(const char *user,const char *pass,DWORD *access)
{
	if(!strcmp(user,"")&&!strcmp(pass,"") )
	{
		*access = RSAM_SPACE_ALL;
		return 0;
	}
	return 1;
}

//保存Rana空间的功能，将当前Rana空间保存到对应的文件中
int RanaStrategy::saveSpace(RanaSpace *sapce)
{
	return 0;
}

//新建RanaEvent功能，根据所给定的事件类型，创建相应的RanaEvent实例
RanaEvent* RanaStrategy::CreateRanaEvent( WORD eventType , const std::vector<RanaVariant> & args)
{
	RanaEvent* pRanaEvent = ENS_RanaEvent::createEvent(eventType,args);
	return pRanaEvent;
}

//得到基础路径，从客户端发送的文件请求都将定位到这个目录下
const char* RanaStrategy::GetBaseScriptPath()
{
	return m_basePath.c_str();
}

std::string MakeValidPathName(const char* pPathName)
{
	int nStrlen = (int)strlen(pPathName);
	char* pStrBuf = new char[nStrlen+2];
	strcpy(pStrBuf,pPathName);
	pStrBuf[nStrlen+1] = '\0';
	while(nStrlen>0)
	{
		if(pStrBuf[nStrlen-1]=='\t'||pStrBuf[nStrlen-1]==' ')
		{
			pStrBuf[nStrlen-1] = '\0';
			nStrlen--;
		}
		else if(pStrBuf[nStrlen-1]!='/'&&pStrBuf[nStrlen-1]!='\\')
		{
			pStrBuf[nStrlen] = '/';
			break;
		}
		else
		{
			break;
		}
	}
	std::string ResultPath = pStrBuf;
	delete [] pStrBuf;
	return ResultPath;

}

//设置基础路径，并非RanaServicesStrategy中所提供的虚函数，用在GetBaseScriptPath之前
void RanaStrategy::setBaseBaseScriptPath(const char* pBasePath)
{
	if(!pBasePath)
	{
		return;
	}

	m_basePath = MakeValidPathName(pBasePath);
}

const char* RanaStrategy::GetEventCodeFromType( RanaEventType eventType )
{
	const char* typeCode = ENS_RanaEvent::getRanaEventNameFromEnum( static_cast<ENS_RanaEvent::TYPE>(eventType) );
	if(!typeCode)
	{
		return "";
	}
	return typeCode;
}

bool RanaStrategy::GetEventTypeFromCode(const char* eventTypeCode, RanaEventType& eventTypeOut)
{
	ENS_RanaEvent::TYPE eventType;
	if( !ENS_RanaEvent::getRanaEventEnumFromName(eventTypeCode,eventType) )
	{
		return false;
	}
	eventTypeOut = eventType;
	return true;
}
