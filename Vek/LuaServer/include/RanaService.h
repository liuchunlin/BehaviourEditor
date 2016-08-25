#ifndef __RANASRV_RANASERVICE_H
#define __RANASRV_RANASERVICE_H
#include <string>
#include <Vek/Base/Module.h>
#include <Zoic/include/SessionBank.h>
#include "RanaServicesStrategy.h"

class RanaClient;
class RanaService:	public Zoic::SessionBank, public Vek::Module<RanaService>
{
	friend class Vek::Singleton<RanaService>;
public:
	Zoic::SessionS *createSession();
	int Init();
	int Tick( float DeltaTime );
	int Flush();

	//����Rana�ͻ���
	RanaClient * getClient(Zoic::Message &msg);
	const HandleType & getClientHandle();
	const SerialType & getClientSerial();
	//�õ�����ָ��
	RanaServicesStrategy * getStrategy(void);
	//���ò���ָ��
	void setStrategy(RanaServicesStrategy *strategy);
	//�õ��ű�Ŀ¼
	const char * getScriptPath(void);
	//���ýű�Ŀ¼
	void setScriptPath(const char *path);

	//������ʱϵͳ
	void updateCurrentTime();
private:
	RanaService();
	~RanaService();
private:
	static void printRanaLog(const char* pChar);
	bool m_clientEnable;				//Rana��������SessionBank�Ƿ���
	RanaServicesStrategy * m_strategy;	//����ָ��
	HandleType	m_clientHandle;		//�ͻ�������
	SerialType	m_clientSerial;		//�ͻ������к�	
	std::string	m_scriptPath;		//�ű�Ŀ¼

};

inline const HandleType & RanaService::getClientHandle()
{
	return m_clientHandle;
}

inline const SerialType & RanaService::getClientSerial()
{
	return m_clientSerial;
}

inline RanaServicesStrategy * RanaService::getStrategy(void)
{
	return m_strategy;
}

inline void RanaService::setStrategy(RanaServicesStrategy *strategy)
{
	if(m_strategy)
	{
		delete m_strategy;
	}
	m_strategy=strategy;
}

inline const char * RanaService::getScriptPath(void)
{
	return m_scriptPath.c_str();
}

inline void RanaService::setScriptPath(const char *path)
{
	m_scriptPath=path;
}

#define g_RanaService RanaService::GetInstance()

#endif
