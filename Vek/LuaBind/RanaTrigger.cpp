
#include "RanaTrigger.h"
#include "RanaSpaceBank.h"

RanaTrigger::RanaTrigger()
	:m_pRanaSpace(NULL)
{
	m_isOn = false;
	m_triggerStat = new RanaTriggerStat;
}

RanaTrigger::~RanaTrigger()
{
	if(m_triggerStat)
	{
		delete m_triggerStat;
		m_triggerStat = NULL;
	}
}

RanaSpace* RanaTrigger::getSelfSpace()
{
	return m_pRanaSpace;
}

void RanaTrigger::setName(const RanaTriggerName &_name)
{
	m_name = _name;
}

const RanaTriggerName &RanaTrigger::getName()
{
	return m_name;
}

void RanaTrigger::setOnOff(bool _isOn)
{
	m_isOn = _isOn;
}

bool RanaTrigger::getOnOff()
{
	return m_isOn;
}

static const int APPEND_MAX_LEN = 32;
bool RanaTrigger::checkCondtion()
{
	char funcName[sizeof(m_name)+APPEND_MAX_LEN];
	m_name.data(funcName);
	strcat(funcName,"_Conditions");
	bool res;
	if(m_pRanaSpace == NULL)
	{
		debug_ranaLog("RanaTrigger::No Self Space Init, Can not run Condtion");
		return false;
	}

	int ret = m_pRanaSpace->runFunction(funcName,&res) ;
	if(ret == RanaSpace::RESULT_OK)
	{
		return res;
	}
	if(ret == RanaSpace::ERROR_NO_SUCH_FUNCTION)
	{
		return true;
	}
	return false;
}

void RanaTrigger::doAction(RanaMessage* msg)
{
	char funcName[sizeof(m_name)+APPEND_MAX_LEN];
	m_name.data(funcName);
	strcat(funcName,"_Actions");

	if(m_pRanaSpace == NULL)
	{
		debug_ranaLog("RanaTrigger::No Self Space Init, Can not run Action");
		return;
	}

	m_pRanaSpace->runFunctionInNewThread(funcName, msg);
}
