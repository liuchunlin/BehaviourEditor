#pragma once

#include "Types.h"

namespace Vek
{
	//出作用域自动清除的指针类
	template<class T>
	class Dumper_Ptr
	{
	public:
		explicit Dumper_Ptr(T *_Ptr = 0)
			: m_Ptr(_Ptr)
		{
		}

		Dumper_Ptr(Dumper_Ptr<T>& Right)
			: m_Ptr(Right.Release())
		{
		}

		template<class Other>
		Dumper_Ptr<T>& operator=(Dumper_Ptr<Other>& Right)
		{
			Reset(Right.Release());
			return (*this);
		}

		template<class Other>
		Dumper_Ptr(Dumper_Ptr<Other>& Right)
			: m_Ptr(Right.Release())
		{
		}

		Dumper_Ptr<T>& operator=(Dumper_Ptr<T>& Right)
		{
			Reset(Right.Release());
			return (*this);
		}

		~Dumper_Ptr()
		{
			delete m_Ptr;
		}

		T& operator*() const
		{
			return (*Pointer());
		}

		T *operator->() const
		{
			return (Pointer());
		}

		T *Pointer() const
		{
			return (m_Ptr);
		}

		operator T*()
		{
			return m_Ptr;
		}
		operator bool()
		{
			return m_Ptr != NULL;
		}

		T *Release()
		{
			T *_Tmp = m_Ptr;
			m_Ptr = 0;
			return (_Tmp);
		}

		void reset(T* _Ptr = 0)
		{
			if (_Ptr != m_Ptr)
				delete m_Ptr;
			m_Ptr = _Ptr;
		}

	private:
		T		*m_Ptr;
	};
}
