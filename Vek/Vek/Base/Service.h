#pragma once

#include "Singleton.h"
#include "Types.h"

namespace Vek
{
	class ServicBase
	{
	public :
		virtual int		PrevInit() = 0;
		virtual int		Init() = 0;
		virtual int		Tick( float AppTime ) = 0;
		virtual int		Flush() = 0;
	};
	template <typename ServicType>
	class Serviceable: public ServicBase
	{
	public :
		typedef int ( ServicType::*FunctorPrevInit )();
		typedef int ( ServicType::*FunctorInit )();
		typedef int ( ServicType::*FunctorTick )( float AppTime );
		typedef int ( ServicType::*FunctorFlush )();

		Serviceable( ServicType* pServic, FunctorPrevInit pfnPrevInit, FunctorInit pfnInit = NULL, FunctorTick pfnTIck = NULL, FunctorFlush pfnFlush = NULL )
			: m_pfnPrevInit( pfnPrevInit )
			, m_pfnInit( pfnInit )
			, m_pfnTick( pfnTIck )
			, m_pfnFlush( pfnFlush )
			, m_pServic( pServic )
		{}
		virtual int		PrevInit()
		{
			if( m_pServic != NULL && m_pfnPrevInit != NULL )
				return (m_pServic->*m_pfnPrevInit)( );
			return 0;
		}
		virtual int		Init()
		{
			if( m_pServic != NULL && m_pfnInit != NULL )
				return (m_pServic->*m_pfnInit)( );
			return 0;
		}
		virtual int		Tick( float AppTime )
		{
			if( m_pServic != NULL && m_pfnTick != NULL )
				return (m_pServic->*m_pfnTick)( AppTime );
			return 0;
		}
		virtual int		Flush()
		{
			if( m_pServic != NULL && m_pfnFlush != NULL )
				return (m_pServic->*m_pfnFlush)( );
			return 0;
		}
	private:
		FunctorPrevInit		m_pfnPrevInit;
		FunctorInit			m_pfnInit;
		FunctorTick			m_pfnTick;
		FunctorFlush		m_pfnFlush;
		ServicType*			m_pServic;
	};

	class Service: public Singleton<Service>
	{
		friend class Singleton<Service>;
	public:
		int		PrevInit();
		int		Init();
		int		Tick( float DeltaTime );
		void	Flush();

		void	RegistService( void* pModule, Vek::ServicBase* pFtr );
	private:
		Service();
		~Service();
	private:
		list<ServicBase*>					m_ModuleInfos;		//迭代器不能失效
	};
#define GVekService Vek::Service::GetInstance()
}

