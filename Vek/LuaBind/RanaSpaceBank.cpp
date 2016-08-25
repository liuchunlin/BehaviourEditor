#include "RanaSpaceBank.h"
#include "RanaEventBank.h"
#include "RanaTimerBank.h"
#include "RanaContext.h"

#include "RanaWait.h"

RanaSpaceBank* RanaSpaceBank::INSTANCE = NULL;

RanaSpaceBank::RanaSpaceBank()
:m_RanaSpaces(NULL)
,m_logFunc(NULL)
,m_errorFunc(NULL)
,m_currentSerial(0)
{
}

RanaSpaceBank::~RanaSpaceBank()
{
	initMaxSpaces(0);
	if (m_RanaSpaces)
	{
		delete [] m_RanaSpaces;
	}
}

void RanaSpaceBank::initMaxSpaces( RanaWORD maxSpaces )
{
	if(m_activeSpaces.size())
	{
		for(list<RanaSpace*>::iterator it = m_activeSpaces.begin();it!=m_activeSpaces.end();it = m_activeSpaces.begin())
		{
			RanaSpace* pRanaSpace = *it;
			m_activeSpaces.erase(it);
			const RanaSpaceHandle& spaceHandle = pRanaSpace->getHandle();
			delSpace(spaceHandle);
		}
		RANA_ASSERT(m_activeSpaces.empty());
	}
	if(m_unusedSpaces.size())
	{
		for(list<RanaSpace*>::iterator it = m_unusedSpaces.begin();it!=m_unusedSpaces.end();it = m_unusedSpaces.begin())
		{
			RanaSpace* pRanaSpace = *it;
			m_unusedSpaces.erase(it);
			delete pRanaSpace;
		}
		RANA_ASSERT(m_unusedSpaces.empty());
	}
	m_SpaceMap.clear();
	if(m_RanaSpaces)
	{
		delete [] m_RanaSpaces;
		m_RanaSpaces = NULL;
	}
	m_maxSpaces = maxSpaces;
	m_RanaSpaces = new RanaSpace*[m_maxSpaces];
	for(RanaWORD i=0;i<m_maxSpaces;i++)
	{
		RanaSpace* pRanaSpace = new RanaSpace;
		pRanaSpace->m_spaceHandle.handle = i;
		m_unusedSpaces.push_back(pRanaSpace);
		m_RanaSpaces[i] = NULL;
	}
}

RanaSpaceBank* RanaSpaceBank::getInstance()
{
	if (!INSTANCE)
	{
		INSTANCE = new RanaSpaceBank;
	}
	return INSTANCE;
}

void RanaSpaceBank::release()
{
	if (INSTANCE)
	{
		delete INSTANCE;
		INSTANCE = NULL;
	}
}

RanaSpace* RanaSpaceBank::forceFindSpaceByIndex(unsigned short index)
{
	if(index>=m_maxSpaces)
	{
		return NULL;
	}
	return m_RanaSpaces[index];
}

RanaSpace* RanaSpaceBank::forceFindSpaceByName(RanaSpaceName spaceName)
{
	for (RanaWORD i = 0; i < m_maxSpaces; ++i)
	{
		if (m_RanaSpaces[i] && m_RanaSpaces[i]->getSpaceName() == spaceName)
		{
			return m_RanaSpaces[i];
		}
	}
	return NULL;
}

RanaSpace*	RanaSpaceBank::GetLuaSpace(RanaBYTE type)
{
	list<RanaSpace*>::iterator iter( m_activeSpaces.begin() );
	for( ; iter != m_activeSpaces.end(); ++iter )
	{
		RanaSpace* pSpace = (*iter);
		if( pSpace->getSpaceType() == type )
		{
			return pSpace;
		}
	}
	return NULL;
}

// 获得一个RanaSpace
RanaSpace* RanaSpaceBank::getLuaSpace(const RanaSpaceHandle& spaceHandle)
{
	if( !forceFindSpaceByIndex(spaceHandle.handle) )
	{
		return NULL;
	}
	if( m_RanaSpaces[spaceHandle.handle]->m_spaceHandle.serial != spaceHandle.serial )
	{
		return NULL;
	}
	return m_RanaSpaces[spaceHandle.handle];
}

// 创建一个RanaSpace
RanaSpace* RanaSpaceBank::createSpace(RanaSpaceName spaceName, const char* fileName, RanaBYTE type, RanaContext * context,bool forceCreate)
{
	if(fileName == NULL)
	{
		debug_ranaLog("RanaSpaceBank::createSpace : filname error\n");
		return NULL;
	}
	if( m_unusedSpaces.empty() )
	{
		debug_ranaLog("RanaSpaceBank::createSpace : spaces full\n");
		return NULL;
	}
	RANA_TYPEMAP::iterator itmap = m_SpaceMap.find(type);
	if(itmap != m_SpaceMap.end())
	{
		RANA_NAMEMAP* pNameMap = &(itmap->second);
		RANA_NAMEMAP::iterator itname = pNameMap->find(spaceName);
		if(itname != pNameMap->end())
		{
			//@todo: 名字已存在，错误处理
			char namestr[sizeof(RanaSpaceName)+1];
			strncpy(namestr, (char *)spaceName.name, sizeof(RanaSpaceName));
			namestr[sizeof(RanaSpaceName)] =  0;
			debug_ranaLog("RanaSpaceBank::createSpace : luaspace[%s] exist\n", namestr);
			return NULL;
		}
	}
	RanaSpace *pRanaSpace=m_unusedSpaces.front();
	pRanaSpace->Status( RanaSpace::SS_INIT );
#ifdef _DEBUG
	vector<RanaTrigger*> triggers;
	pRanaSpace->getTriggerBank()->listTriggers(triggers);
	RANA_ASSERT( triggers.empty() );
#endif
	pRanaSpace->m_spaceHandle.serial = m_currentSerial++;
	const RanaSpaceHandle& spaceHandle = pRanaSpace->getHandle();
	m_unusedSpaces.pop_front();
	m_activeSpaces.push_back(pRanaSpace);
	m_SpaceMap[type][spaceName] = pRanaSpace;
	m_RanaSpaces[spaceHandle.handle] = pRanaSpace;
	int retCode = pRanaSpace->initialEmptySpace(spaceName,type,context);
	if( retCode != RanaSpace::RESULT_OK )
	{
		if( delSpace(spaceHandle) != RANA_OK )
		{
			RANA_ASSERT(0);
		}
		return NULL;
	}
	retCode = pRanaSpace->initialFromFileForEmptySpace(fileName);
	if (retCode !=RanaSpace::RESULT_OK)
	{
		if(!forceCreate)
		{
			pRanaSpace->clear();
			if( delSpace(spaceHandle) != RANA_OK )
			{
				RANA_ASSERT(0);
			}
			return NULL;
		}
		if(fileName)
		{
			pRanaSpace->m_FileName = fileName;
			ranaLog("Warning: init Space [%s] Failed For Load File \"%s\"(code %d)!Force Load Empty.",static_cast<const char*>(spaceName),fileName,retCode );
		}
	}
	ranaLog("init Space [%s] of Handle[%d] OK!!!\n",static_cast<const char*>(spaceName),spaceHandle.handle );
	pRanaSpace->Status( RanaSpace::SS_RUNING );
	return pRanaSpace;
}

// 删除一个RanaSpace
int RanaSpaceBank::delSpace(const RanaSpaceHandle& spaceHandle)
{
	RanaSpace* pRanaSpace = getLuaSpace(spaceHandle);
	if(!pRanaSpace)
	{
		if( m_RanaSpaces[spaceHandle.handle] )
		{
			debug_ranaLog("RanaSpaceBank::delSpace : handle[%d] serial error!\n", spaceHandle.handle);
		}
		else
		{
			debug_ranaLog("RanaSpaceBank::delSpace : handle[%d] not exist\n", spaceHandle.handle);
		}
		return -1;
	}
	RANA_ASSERT(pRanaSpace->getHandle().handle==spaceHandle.handle);
	if(pRanaSpace->m_errorCode!=RanaSpace::EC_SUCCESS)
	{
		pRanaSpace->m_errorCode = RanaSpace::EC_SUCCESS;
		m_exitSpaces.erase(pRanaSpace->m_itErrorList);
	}
	RanaSpaceName spaceName = pRanaSpace->getSpaceName();
	RanaBYTE type = pRanaSpace->getSpaceType();
	pRanaSpace->Status( RanaSpace::SS_PENDING );
	//RanaWait
	GLuaWaitGridge->DeleteSpace( type );
	pRanaSpace->getEventBank()->delAllEvent();
	pRanaSpace->getTriggerBank()->deleteTrigger();
	// 删除相应RanaTimer
	RanaTimerBank::getInstance()->delTimer(spaceHandle);
	// 释放RanaContext空间
	pRanaSpace->releaseValueTable();
	//关闭lua空间
	pRanaSpace->clear();

	ranaLog("close Space of Handle[%d].\n",spaceHandle.handle );
	m_activeSpaces.remove(pRanaSpace);
	m_unusedSpaces.push_back(pRanaSpace);
	m_RanaSpaces[spaceHandle.handle] = NULL;
	RANA_TYPEMAP::iterator itmap = m_SpaceMap.find(type);
	if(itmap!=m_SpaceMap.end())
	{
		RANA_NAMEMAP* pNameMap = &(itmap->second);
		size_t count = pNameMap->erase(spaceName);
		if(!count)
		{
			debug_ranaError("RanaSpaceError:can not find deleting space of name \"%s\".",static_cast<const char*>(spaceName) );
			RANA_ASSERT(0);
		}
	}
	return RANA_OK;
}

// 重载一个RanaSpace
int RanaSpaceBank::reloadSpace(const RanaSpaceHandle& spaceHandle, const char* fileName)
{
	RanaSpace *pRanaSpace = getLuaSpace(spaceHandle);
	if(!pRanaSpace)
	{
		debug_ranaLog("RanaSpaceBank::reloadSpace : luaspace handle[%d] not exist\n", spaceHandle.handle);
		return -1;
	}
	RanaSpaceName spaceName = pRanaSpace->getSpaceName();
	unsigned char type = pRanaSpace->getSpaceType();
	pRanaSpace->Status( RanaSpace::SS_PENDING );

	//RanaWait
	GLuaWaitGridge->DeleteSpace( type );
	RanaContext * context = pRanaSpace->getContext();

	pRanaSpace->getEventBank()->delAllEvent();
	pRanaSpace->getTriggerBank()->deleteTrigger();
	// 删除相应RanaTimer
	RanaTimerBank::getInstance()->delTimer(spaceHandle);
	// 释放RanaContext空间
	pRanaSpace->releaseValueTable();
	//关闭lua空间
	pRanaSpace->clear();
	if (pRanaSpace->initial(fileName,spaceName,type,context)==RanaSpace::RESULT_OK)
	{
		debug_ranaLog("reload Space [%s] OK!!!\n",static_cast<const char*>(spaceName) );
	}
	else
	{
		debug_ranaLog("RanaSpaceBank::reloadSpace : cannot create luaspace [handle%d], filename[%s]\n",spaceHandle.handle, fileName);
		delSpace(spaceHandle);
		return -1;
	}
	return 0;
}

int RanaSpaceBank::getSpaceCount(RanaWORD type)
{
	RANA_TYPEMAP::iterator itmap = m_SpaceMap.find((RanaBYTE)type);
	if( itmap != m_SpaceMap.end() )
	{
		return (int)itmap->second.size();
	}
	return 0;
}

const map<RanaSpaceName,RanaSpace*>& RanaSpaceBank::listSpaces( RanaWORD type )
{
	RANA_TYPEMAP::iterator itmap = m_SpaceMap.find((RanaBYTE)type);
	if(itmap != m_SpaceMap.end() )
	{
		return itmap->second;
	}
	return m_emptyList;
}

void RanaSpaceBank::routine()
{
	if(m_exitSpaces.size())
	{
		list<RanaSpace*>::iterator it = m_exitSpaces.begin();
		for(;it!=m_exitSpaces.end();it=m_exitSpaces.begin())
		{
			RanaSpace* pRanaSpace = *it;
			const RanaSpaceHandle& spaceHandle = pRanaSpace->getHandle();
			RanaSpace::EXCEPTION_CODE errorCode = pRanaSpace->m_errorCode;
			pRanaSpace->m_errorCode = RanaSpace::EC_SUCCESS;
			m_exitSpaces.erase(it);
			if( pRanaSpace->getContext()->onRanaException(errorCode) == RanaSpace::EM_DELSPACE )
			{
				delSpace(spaceHandle);
			}
		}
	}
//#ifdef _DEBUG
//	list<RanaSpace*>::const_iterator itSpace = m_activeSpaces.begin();
//	for(; itSpace != m_activeSpaces.end(); ++itSpace )
//	{
//		RanaSpace* pSpace = *itSpace;
//		const int currentStack = lua_gettop(pSpace->m_state);
//		for( int i = currentStack;i>0;--i)
//		{
//			ranaError( "RanaSpaceBank:lua stack leak:%d(%s)\n", i,lua_typename(pSpace->m_state,lua_type(pSpace->m_state,i)) );
//		}
//		RANA_ASSERT( currentStack == 0 );
//	}
//#endif
}

void RanaSpaceBank::setSpaceExit(const RanaSpaceHandle& spaceHandle, RanaSpace::EXCEPTION_CODE exitCode)
{
	RanaSpace* pRanaSpace = getLuaSpace(spaceHandle);
	if(pRanaSpace)
	{
		if(exitCode!=RanaSpace::EC_SUCCESS)
		{
			if(pRanaSpace->m_errorCode == RanaSpace::EC_SUCCESS && pRanaSpace->m_context)
			{
				pRanaSpace->m_errorCode = exitCode;
				pRanaSpace->m_itErrorList = m_exitSpaces.insert(m_exitSpaces.end(),pRanaSpace);
			}
		}
		else
		{
			if(pRanaSpace->m_errorCode != RanaSpace::EC_SUCCESS)
			{
				pRanaSpace->m_errorCode = exitCode;
				m_exitSpaces.erase(pRanaSpace->m_itErrorList);
			}
		}
	}
}

void RanaSpaceBank::setLogFunc( LOGFUNC logFunc )
{
	m_logFunc = logFunc;
}

void RanaSpaceBank::setErrorFunc( LOGFUNC errorFunc )
{
	m_errorFunc = errorFunc;
}

void RanaSpaceBank::printLog(const char* pChar)
{
	if(!m_logFunc)
	{
		printf("%s",pChar);
		return;
	}
	(*m_logFunc)(pChar);
}

void RanaSpaceBank::printError(const char* pChar)
{
	if(m_errorFunc)
	{
		(*m_errorFunc)(pChar);
		return;
	}
	if(m_logFunc)
	{
		(*m_logFunc)(pChar);
	}
	perror(pChar);
}
