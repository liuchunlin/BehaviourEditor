/* Generated by Together */

#include "RanaTriggerBank.h"
#include "RanaEventBank.h"
#include "RanaSpace.h"

RanaTriggerBank::RanaTriggerBank(RanaSpace* pSpace)
:m_pRanaSpace(pSpace)
,m_BurstingTrigger(NULL)
{
}

RanaTriggerBank::~RanaTriggerBank()
{
	deleteTrigger();
}


// 添加一个触发器
int RanaTriggerBank::addTrigger(RanaTrigger* pTrigger)
{
	RanaTriggerName triggerName = pTrigger->getName();
	if(!strcmp(triggerName,""))
	{
		return -1;
	}
	std::map<RanaTriggerName,RanaTrigger*>::iterator itTriggerName = m_nameMap.find(triggerName);
	if( itTriggerName != m_nameMap.end() )
	{
		//该RanaSpace中已有相同名字的trigger
		pTrigger->m_handle = itTriggerName->second->getHandle();
		return -1;
	}
	/*//---------------------------------------------------------------S
	const char* const pFileBuf  = m_pRanaSpace->getLoadedFileBuff();
	if(!pFileBuf)
	{
		return -1;
	}
	*///---------------------------------------------------------------E
	RanaWORD handle = 0;
	if(!m_unusedHandle.size())
	{
		const RanaWORD maxHandle = ~0;
		if( m_triggersByHandle.size() >= maxHandle )
		{
			return -1;
		}
		handle = static_cast<RanaWORD>( m_triggersByHandle.size() );
		m_triggersByHandle.push_back(NULL);
	}
	else
	{
		handle = m_unusedHandle.front();
		m_unusedHandle.pop_front();
	}
	m_triggersByHandle[handle] = pTrigger;
	m_nameMap[triggerName] = pTrigger;
	pTrigger->m_handle = handle;
	pTrigger->m_pRanaSpace = m_pRanaSpace;

/*	//---------------------------------------------------------------S
	if(m_pRanaSpace->getLoadedFileLength()>0)
	{
		std::string strFuncName = (const char*)triggerName;
		strFuncName += "_Conditions";
		unsigned long nContentLength = 0;
		const char* pContent = NULL;
		if( m_pRanaSpace->getFunctionContentInFileBuff(strFuncName.c_str(), pContent, nContentLength) == 0 )
		{
			char* pbuf = new char[nContentLength+1];
			memcpy(pbuf,pContent,nContentLength);
			pbuf[nContentLength] = '\0';
			pTrigger->setCondition(pbuf);
			delete pbuf;
		}
		strFuncName = (const char*)triggerName;
		strFuncName += "_Actions";
		if( m_pRanaSpace->getFunctionContentInFileBuff(strFuncName.c_str(), pContent, nContentLength) == 0 )
		{
			char* pbuf = new char[nContentLength+1];
			memcpy(pbuf,pContent,nContentLength);
			pbuf[nContentLength] = '\0';
			pTrigger->setAction(pbuf);
			delete pbuf;
		}
	}
*/
	return 0;
}

/*
// 删除一个触发器
int RanaTriggerBank::deleteTrigger(RanaWORD triggerHandle)
{
	RanaTrigger* pTrigger = getTrigger(triggerHandle);
	if(!pTrigger)
	{
		return -1;
	}
	m_triggersByHandle[triggerHandle] = NULL;
	std::map<RanaTriggerName,RanaTrigger*>::iterator itMap = m_nameMap.find(pTrigger->getName());
	if(itMap!=m_nameMap.end())
	{
		m_nameMap.erase(itMap);
	}
	m_unusedHandle.push_back(triggerHandle);
	delete pTrigger;

	m_pRanaSpace->getEventBank()->delEventByTrigger( triggerHandle );

	return 0;
}
*/

void RanaTriggerBank::deleteTrigger()
{
	for(size_t i = 0; i< m_triggersByHandle.size(); ++i)
	{
		RanaTrigger* pTrigger = m_triggersByHandle[i];
		if(!pTrigger)
		{
			continue;
		}
		delete pTrigger;
		m_triggersByHandle[i] = NULL;
	}
	m_triggersByHandle.clear();
	m_unusedHandle.clear();
	m_nameMap.clear();
	if( m_pRanaSpace->getEventBank() )
	{
		m_pRanaSpace->getEventBank()->delAllEvent();
	}
}

// 触发一个触发器
int RanaTriggerBank::burstTrigger(RanaWORD triggerHandle, RanaMessage* msg)
{
	RanaTrigger* pTrigger = getTrigger(triggerHandle);
	if(!pTrigger)
	{
		//没有找到合适的trigger
		return -1;
	}
	if(!pTrigger->getOnOff())
	{
		return -1;
	}
	m_BurstingTrigger = pTrigger;
	RanaTriggerStat* pTriggerStat = pTrigger->getTriggerStat();
	pTriggerStat->ConditionStart();
	if(pTrigger->checkCondtion())
	{
		pTriggerStat->ConditionOver();
		pTriggerStat->countMeca();	//2006-09-19 通过了Condtion检测并执行了Action
//*2006-09-20 测试用，打印此次调用进行Condition检查的花费时间
//		debug_ranaLog(" ( No.%d: \"%s\" Successed in Condition, Cost:%d )\n",pTriggerStat->getMecaCount(), pTrigger->getName().name, pTriggerStat->getLastConditionTime());
//*/
		pTriggerStat->ActionStart();

		pTrigger->doAction(msg);

		pTriggerStat->ActionOver();
//*2006-09-20 测试用，打印此次调用进行Action执行的花费时间
		debug_ranaLog(" ( Handle:%d(%d): \"%s\" run Action, Cost:<Condition>%d,<Action>%d )\n",m_pRanaSpace->getHandle().handle,pTriggerStat->getMecaCount(), pTrigger->getName().name,pTriggerStat->getLastConditionTime(),pTriggerStat->getLastActionTime() );
//*/
	}
	else
	{
		pTriggerStat->ConditionOver();
		pTriggerStat->countMec();	//2006-09-19 未通过Condtion检测并执行了Action
//*2006-09-20 测试用，打印此次调用进行Condition检查的花费时间
		debug_ranaLog(" ( Handle:%d(%d): \"%s\" failed in Condition, Cost:%d )\n",m_pRanaSpace->getHandle().handle, pTriggerStat->getMecCount(), pTrigger->getName().name,pTriggerStat->getLastConditionTime());
//*/
	}
	m_BurstingTrigger = NULL;
	return 0;
}

// 触发一个触发器（不进行条件判断）
int RanaTriggerBank::burstTrigger(RanaWORD triggerHandle, RanaMessage* msg, bool bCheckCondition)
{
	if( bCheckCondition )
	{
		burstTrigger( triggerHandle, msg );
	}
	else
	{
		RanaTrigger* pTrigger = getTrigger(triggerHandle);
		if( pTrigger == NULL )
		{
			return -1;
		}
		m_BurstingTrigger = pTrigger;
		pTrigger->doAction(msg);
		m_BurstingTrigger = NULL;
	}

	return 0;
}

// 打开/关闭一个触发器
int RanaTriggerBank::setTriggerOnOff(RanaWORD triggerHandle, bool isOn)
{
	RanaTrigger* pTrigger = getTrigger(triggerHandle);
	if(!pTrigger)
	{
		//没有找到合适的trigger
		return -1;
	}
	pTrigger->setOnOff(isOn);
//	RanaEventManager::getInstance()->setEventOnOff(spaceName, triggerName, isOn);
	return 0;
}

// 获得触发器开关状态
bool RanaTriggerBank::getTriggerOnOff(RanaWORD triggerHandle)
{
	RanaTrigger* pTrigger = getTrigger(triggerHandle);
	if(!pTrigger)
	{
		//没有找到合适的trigger
		return false;
	}
	return pTrigger->getOnOff();
}

//2006-09-19 触发器状态统计：收到消息
bool RanaTriggerBank::tagMessageCount(RanaWORD triggerHandle)
{
	if(!this)
	{
		return false;
	}
	RanaTrigger* pTrigger = getTrigger(triggerHandle);
	if(!pTrigger)
	{
		//没有找到合适的trigger
		return false;
	}

	pTrigger->getTriggerStat()->countM();

	return true;
}

void RanaTriggerBank::setAllTriggerOnOff(bool isOn)
{
	for(size_t i = 0; i< m_triggersByHandle.size(); ++i)
	{
		RanaTrigger* pTrigger = m_triggersByHandle[i];
		if(!pTrigger)
		{
			continue;
		}
		pTrigger->setOnOff(isOn);
	}
}


void RanaTriggerBank::listTriggers(std::vector<RanaTrigger*>& output)
{
	for(size_t i = 0; i< m_triggersByHandle.size(); ++i)
	{
		RanaTrigger* pTrigger = m_triggersByHandle[i];
		if(!pTrigger)
		{
			continue;
		}
		output.push_back(pTrigger);
	}
}

RanaTrigger* RanaTriggerBank::getTrigger(RanaWORD triggerHandle)
{
	if( triggerHandle >= m_triggersByHandle.size() )
	{
		return NULL;
	}
	return m_triggersByHandle[triggerHandle];

/*	std::map<RanaWORD,RanaTrigger*>::iterator it = m_triggers.find(triggerHandle);
	if( it == m_triggers.end())
	{
		//没有找到合适的trigger
		return NULL;
	}
	return it->second;
*/
}

RanaTrigger* RanaTriggerBank::getTrigger(const RanaTriggerName& triggerName)
{
	std::map<RanaTriggerName,RanaTrigger*>::iterator it = m_nameMap.find(triggerName);
	if( it == m_nameMap.end())
	{
		//没有找到合适的trigger
		return NULL;
	}
	return it->second;
}
