#include "Service.h"

using namespace Vek;

Service::Service()
{
}

Service::~Service()
{
}

int		Service::Start()
{
	Initialize();
	return 1;
}
int		Service::Tick( float DeltaTime )
{
	std::map<void*, ServicBase*>::iterator iter( m_ModuleInfos.begin() );
	for( ; iter != m_ModuleInfos.end(); ++iter )
	{
		ServicBase* pFuntor = (*iter).second;
		pFuntor->Tick( DeltaTime );
	}
	return 1;
}
void	Service::Destory()
{
	std::map<void*, ServicBase*>::iterator iter( m_ModuleInfos.begin() );
	for( ; iter != m_ModuleInfos.end(); ++iter )
	{
		ServicBase* pFuntor = (*iter).second;
		pFuntor->Destory();
		delete pFuntor;
	}
	m_ModuleInfos.clear();
}

int		Service::Initialize()
{
	std::map<void*, ServicBase*>::iterator iter( m_ModuleInfos.begin() );
	for( ; iter != m_ModuleInfos.end(); ++iter )
	{
		ServicBase* pFuntor = (*iter).second;
		pFuntor->Initialize();
	}
	return 1;
}

void	Service::RegistService( void* pModule, Vek::ServicBase* pFtr )
{
	if( pModule != NULL && pFtr != NULL )
		m_ModuleInfos.insert( make_pair( pModule, pFtr ) );
}
