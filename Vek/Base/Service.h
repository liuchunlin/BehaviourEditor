#pragma once

#include "Singleton.h"
#include <vector>
#include <map>
using namespace std;
namespace Vek
{
	class ServicBase
	{
	public :
		virtual int		Initialize() = 0;
		virtual int		Tick( float DeltaTime ) = 0;
		virtual int		Destory() = 0;
	};
	template <typename ServicType>
	class Serviceable: public ServicBase
	{
	public :
		typedef int ( ServicType::*FunctorInit )();
		typedef int ( ServicType::*FunctorTick )( float DeltaTime );
		typedef int ( ServicType::*FunctorDestory )();

		Serviceable( ServicType* pServic, FunctorInit pfnInit = NULL, FunctorTick pfnTIck = NULL, FunctorDestory pfnDestory = NULL )
			: m_pfnInit( pfnInit )
			, m_pfnTick( pfnTIck )
			, m_pfnDestory( pfnDestory )
			, m_pServic( pServic )
		{}
		virtual int		Initialize()
		{
			if( m_pServic != NULL && m_pfnInit != NULL )
				return (m_pServic->*m_pfnInit)( );
			return 0;
		}
		virtual int		Tick( float DeltaTime )
		{
			if( m_pServic != NULL && m_pfnTick != NULL )
				return (m_pServic->*m_pfnTick)( DeltaTime );
			return 0;
		}
		virtual int		Destory()
		{
			if( m_pServic != NULL && m_pfnDestory != NULL )
				return (m_pServic->*m_pfnDestory)( );
			return 0;
		}
	private:
		FunctorInit			m_pfnInit;
		FunctorTick			m_pfnTick;
		FunctorDestory		m_pfnDestory;
		ServicType*			m_pServic;
	};

	class Service: public Singleton<Service>
	{
		friend class Singleton<Service>;
	public:
		int		Start();
		int		Tick( float DeltaTime );
		void	Destory();

		void	RegistService( void* pModule, Vek::ServicBase* pFtr );
	private:
		int		Initialize();

		Service();
		~Service();

		std::map<void*, ServicBase*>		m_ModuleInfos;
	};
	#define GVekService Vek::Service::GetInstance()
}

