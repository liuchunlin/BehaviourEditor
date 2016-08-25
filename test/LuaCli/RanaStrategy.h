#ifndef __RANASTRATEGY_H
#define __RANASTRATEGY_H

//策略类，实现RanaServicesStrategy中所提供的虚函数，使之具有针对性的的功能

#include <LuaServer/include/RanaServicesStrategy.h>
#include <string>

class RanaStrategy:
	public RanaServicesStrategy
{
public:
	RanaStrategy();
	int certifcate(const char *user,const char *pass,DWORD *access);
	int saveSpace(RanaSpace *sapce);
	RanaEvent* CreateRanaEvent( WORD eventType,const std::vector<RanaVariant> & args);
	const char* GetBaseScriptPath();
	void setBaseBaseScriptPath(const char* pBasePath);
	const char* GetEventCodeFromType( RanaEventType eventType );
	bool GetEventTypeFromCode(const char* eventTypeCode, RanaEventType& eventTypeOut);
private:
	std::string m_basePath;
};

#endif
