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

//����¼�õĺ���������û��������Ƿ�Ϸ�����ͨ��access�����Ӧ�û���Ȩ��
int RanaStrategy::certifcate(const char *user,const char *pass,DWORD *access)
{
	if(!strcmp(user,"")&&!strcmp(pass,"") )
	{
		*access = RSAM_SPACE_ALL;
		return 0;
	}
	return 1;
}

//����Rana�ռ�Ĺ��ܣ�����ǰRana�ռ䱣�浽��Ӧ���ļ���
int RanaStrategy::saveSpace(RanaSpace *sapce)
{
	return 0;
}

//�½�RanaEvent���ܣ��������������¼����ͣ�������Ӧ��RanaEventʵ��
RanaEvent* RanaStrategy::CreateRanaEvent( WORD eventType , const std::vector<RanaVariant> & args)
{
	RanaEvent* pRanaEvent = ENS_RanaEvent::createEvent(eventType,args);
	return pRanaEvent;
}

//�õ�����·�����ӿͻ��˷��͵��ļ����󶼽���λ�����Ŀ¼��
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

//���û���·��������RanaServicesStrategy�����ṩ���麯��������GetBaseScriptPath֮ǰ
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
