#include "Service.h"
#include <algorithm>

using namespace Vek;

Service::Service()
{
}
Service::~Service()
{
}
int	Service::PrevInit()
{
	for each (ServicBase* var in m_ModuleInfos)
	{
		var->PrevInit();
	}
	return 0;
}
int	Service::Init()
{
	for each (ServicBase* var in m_ModuleInfos)
	{
		var->Init();
	}
	return 0;
}
int	Service::Tick( float AppTime )
{
	for each (ServicBase* var in m_ModuleInfos)
	{
		var->Tick(AppTime);
	}
	return 0;
}
void	Service::Flush()
{
	for each (ServicBase* var in m_ModuleInfos)
	{
		var->Flush();
	}

	for each (ServicBase* var in m_ModuleInfos)
	{
		delete var;
	}
	m_ModuleInfos.clear();
}

void	Service::RegistService( void* pModule, Vek::ServicBase* pFtr )
{
	if( pModule == NULL || pFtr == NULL )	
		return;
	//�ڵ��������У����ܽ��������������m_ModuleInfos����������ʧЧ
	m_ModuleInfos.push_back( pFtr );
}
