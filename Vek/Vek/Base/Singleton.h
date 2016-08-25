#pragma once

#include <vector>
#include <new>

namespace Vek
{
	// 单例模板
	template <class T>
	class Singleton
	{
		friend class Rleaser;
	public:
		//!获得实例
		static T* GetInstance(void);
	protected:
		Singleton();
	private:
		//! 删除实例
		static void Destroy();

		static T* m_Instance;	//!< 单例指针
	};

	template <class T> T* Singleton<T>::m_Instance = 0;

	class SingletonRleaser
	{
		template <class T> friend class Singleton;
	public:
		~SingletonRleaser();
	private:
		typedef void (*DESTROY_FUNC)();
		std::vector<DESTROY_FUNC> m_funcs;
	};

	inline SingletonRleaser::~SingletonRleaser()
	{
		for(std::vector<DESTROY_FUNC>::const_iterator it=m_funcs.begin();it!=m_funcs.end();++it)
		{
			(**it)();
		}
		m_funcs.clear();
	}

	inline SingletonRleaser & getSingletonRleaser()
	{
		static SingletonRleaser releaser;
		return releaser;
	}

	template <class T> 
	T* Singleton<T>::GetInstance(void)
	{
		if(m_Instance){	return m_Instance;}
		else
		{
			m_Instance=new T;
			getSingletonRleaser().m_funcs.push_back(&Destroy);
			return m_Instance;
		}
	}

	template <class T> 
	Singleton<T>::Singleton()
	{
	}

	template <class T> 
	void Singleton<T>::Destroy()
	{
		if(m_Instance)
		{
			delete m_Instance;
			m_Instance = 0;
		}
	}
}

