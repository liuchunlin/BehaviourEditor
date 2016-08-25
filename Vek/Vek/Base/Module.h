#pragma once

#include "Singleton.h"
#include "Service.h"
#include <vector>

namespace Vek
{
	template <typename T>
	class Module: public Vek::Singleton<T>
	{
	public:
	protected:
		Module();
		virtual int		PrevInit(){ return 0; }
		virtual int		Init(){ return 0; }
		virtual int		Tick( float DeltaTime ){ return 0; }
		virtual int		Flush(){ return 0; }
	private:
	};
	template <typename T>
	Module<T>::Module()
	{
		GVekService->RegistService( this ,new Serviceable<Module<T> >( this,&Module<T>::PrevInit,  &Module<T>::Init, &Module<T>::Tick, &Module<T>::Flush ) );
	}
}

