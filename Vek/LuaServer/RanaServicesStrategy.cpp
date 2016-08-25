#include "include/RanaServicesStrategy.h"
#include "include/AccessMask.h"

RanaServicesStrategy::RanaServicesStrategy()
{
}

RanaServicesStrategy::~RanaServicesStrategy()
{
}

int RanaServicesStrategy::certifcate(const char *user,const char *pass,DWORD *access)
{
	*access = RSAM_SPACE_ALL;
	return 0;
}

int RanaServicesStrategy::saveSpace(RanaSpace *sapce)
{
	return 0;
}

RanaEvent* RanaServicesStrategy::CreateRanaEvent( WORD eventType,const std::vector<RanaVariant> & args)
{
	return NULL;
}

const char* RanaServicesStrategy::GetBaseScriptPath()
{
	return "";
}

const char* RanaServicesStrategy::GetEventCodeFromType( RanaEventType eventType )
{
	return "";
}

bool RanaServicesStrategy::GetEventTypeFromCode(const char* eventTypeCode, RanaEventType& eventTypeOut)
{
	return false;
}
