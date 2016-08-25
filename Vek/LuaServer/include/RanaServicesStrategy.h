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
	
	//��֤����Ա
	/*
		����ֵ
		<0�����ڴ�����
		=0���ɹ�
		>0��ʧ��
	*/
	virtual int certifcate(const char *user,const char *pass,DWORD *access);
	
	//����Rana�ռ�
	/*
		����ֵ
		<0�����ڴ�����
		=0���ɹ�
		>0��ʧ��
	*/
	virtual int saveSpace(RanaSpace *sapce);

	virtual RanaEvent* CreateRanaEvent( WORD eventType,const std::vector<RanaVariant> & args);

	virtual const char* GetBaseScriptPath();

	virtual const char* GetEventCodeFromType( RanaEventType eventType );

	virtual bool GetEventTypeFromCode(const char* eventTypeCode, RanaEventType& eventTypeOut);

	virtual ~RanaServicesStrategy();
};

#endif
