#include <LuaBind/RanaTimerBank.h>
#include <Zoic/include/Message.h>
#include "include/RanaService.h"
#include "include/RanaClient.h"
#include <Zoic/include/Log.h>
#include <Zoic/include/Clock.h>

RanaService::RanaService()
:m_clientEnable(true)
{
	m_strategy = new RanaServicesStrategy;
	RanaSpaceBank::getInstance()->setLogFunc( &printRanaLog );
}

RanaService::~RanaService()
{
	delete m_strategy;
}

Zoic::SessionS * RanaService::createSession()
{
	return new RanaClient();
}

namespace RanaSrv
{
	extern BYTE link_Cmd_Rana;
}

int RanaService::Init()
{
	if( strcmp(getListenIP(),"")==0 )
	{
		log_f("Can't Find LestenIP,Rana-Server Will not Listen For Rana-Client!");
		m_clientEnable = false;
	}
	int ret = 0;
	if( m_clientEnable )
	{
		ret=Zoic::SessionBank::start();
	}
	if(ret==0)
	{
		log_f("RanaServer start");
		RanaTIME curtime = g_Clock->getMilliSeconds();
		RanaTimerBank::getInstance()->initCurrentTime(curtime);
	}
	RanaSrv::link_Cmd_Rana = 0;
	return ret;
}

int RanaService::Tick( float DeltaTime )
{
	//更新当前的时间并检查时间触发事件
	RanaTIME curtime = g_Clock->getMilliSeconds();
	RanaTimerBank::getInstance()->routine(curtime);
	//
	RanaSpaceBank::getInstance()->routine();
	if(m_clientEnable)
	{
		return Zoic::SessionBank::run();
	}
	return 0;
}

int RanaService::Flush()
{
	log_f("RanaServer stop");
	RanaSpaceBank::release();
	RanaTimerBank::release();
	if(m_clientEnable)
	{
		return Zoic::SessionBank::stop();
	}
	return 0;
}

RanaClient * RanaService::getClient(Zoic::Message &msg)
{
	msg >> m_clientHandle >> m_clientSerial;
	return static_cast<RanaClient *>(g_RanaService->findSession(m_clientHandle,m_clientSerial));
}

void RanaService::printRanaLog(const char* pChar)
{
	log_f("%s",pChar);
};
