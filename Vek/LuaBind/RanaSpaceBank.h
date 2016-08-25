#ifndef __RANA_SPACEBANK_H__
#define __RANA_SPACEBANK_H__

#pragma warning (disable : 4786)

#include <set>
#include <map>
#include <list>
#include "RanaSpace.h"

class RanaSpaceBank
{
	friend class RanaSpace;
public:
	typedef map<RanaSpaceName,RanaSpace*> RANA_NAMEMAP;
	typedef map<RanaBYTE,RANA_NAMEMAP> RANA_TYPEMAP;	//空间类型的管理
	typedef void (*LOGFUNC)(const char* pChar);

	static RanaSpaceBank*	getInstance();
	static void				release();

	RanaSpace*				createSpace(RanaSpaceName spaceName, const char* fileName, RanaBYTE type = 0, RanaContext * context = NULL, bool forceCreate = false	);
	int						delSpace(const RanaSpaceHandle& spaceHandle);
	int						reloadSpace(const RanaSpaceHandle& spaceHandle, const char* fileName);
	RanaSpace*				getLuaSpace(const RanaSpaceHandle& spaceHandle);
	RanaSpace*				GetLuaSpace(RanaBYTE type);

	void					routine();
	void					initMaxSpaces( RanaWORD maxSpaces );
	int						getSpaceCount( RanaWORD type );
	const map<RanaSpaceName,RanaSpace*>& listSpaces( RanaWORD type );

	void					setLogFunc( LOGFUNC logFunc );
	void					setErrorFunc( LOGFUNC errorFunc );
	void					printLog(const char* pChar);
	void					printError(const char* pChar);

	RanaSpace*				forceFindSpaceByIndex(unsigned short index);
	RanaSpace*				forceFindSpaceByName(RanaSpaceName spaceName);
private:
	RanaSpaceBank();
	~RanaSpaceBank();
	void					setSpaceExit(const RanaSpaceHandle& spaceHandle, RanaSpace::EXCEPTION_CODE exitCode);
private:
	static RanaSpaceBank *INSTANCE;

	RanaWORD				m_maxSpaces;
	RanaSpace **			m_RanaSpaces;
	list<RanaSpace*>		m_unusedSpaces;
	list<RanaSpace*>		m_activeSpaces;
	RanaDWORD				m_currentSerial;
	RANA_TYPEMAP			m_SpaceMap;
	const RANA_NAMEMAP		m_emptyList;
	LOGFUNC					m_logFunc;
	LOGFUNC					m_errorFunc;

	list<RanaSpace*>		m_exitSpaces;
};

#endif //__RANA_SPACEMANAGER_H__
