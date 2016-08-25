#ifndef __RANASRV_RANASERVICESSTRATEGY_H
#define __RANASRV_RANASERVICESSTRATEGY_H
#include <vector>
#include <Zoic/include/Types.h>
#include <LuaBind/RanaSpaceBank.h>

class RanaSpace;
class RanaServicesStrategy
{
public:
	RanaServicesStrategy();
	
	//验证管理员
	/*
		返回值
		<0：正在处理中
		=0：成功
		>0：失败
	*/
	virtual int certifcate(const char *user,const char *pass,DWORD *access);
	
	//保存Rana空间
	/*
		返回值
		<0：正在处理中
		=0：成功
		>0：失败
	*/
	virtual int saveSpace(RanaSpace *sapce);

	virtual RanaEvent* CreateRanaEvent( WORD eventType,const std::vector<RanaVariant> & args);

	virtual const char* GetBaseScriptPath();

	virtual const char* GetEventCodeFromType( RanaEventType eventType );

	virtual bool GetEventTypeFromCode(const char* eventTypeCode, RanaEventType& eventTypeOut);

	virtual ~RanaServicesStrategy();
};

#endif
