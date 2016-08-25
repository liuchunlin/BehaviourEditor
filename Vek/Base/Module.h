#pragma once

#include "Singleton.h"
#include "Service.h"
#include <vector>

namespace Vek
{
	template <typename T>
	class Module: public Vek::Singleton<T>
	{
		friend class Vek::Singleton<T>;
	public:
	protected:
		Module();
		virtual int		Initialize(){ return 0; }
		virtual int		Tick( float DeltaTime ){ return 0; }
		virtual int		Destory(){ return 0; }
	private:
	};
	template <typename T>
	Module<T>::Module()
	{
		GVekService->RegistService( this ,new Serviceable<Module<T> >( this, &Module<T>::Initialize, &Module<T>::Tick, &Module<T>::Destory ) );
	}
}

