#pragma once
#include "Types.h"

namespace Vek
{
	template <class T>
	class ObjLink
	{
	public:
		ObjLink(T* pObj)
			: m_pPrev(NULL)
			, m_pNext(NULL)
			, m_pObj(pObj)
		{
			Link();
		}
		virtual ~ObjLink()
		{
			Unlink();
		}

		T* Obj() { return m_pObj; }
		ObjLink* Prev() const { return m_pPrev; }
		ObjLink* Next() const { return m_pNext; }

		template <class Fn>
		static void VisitAll(const Fn& fn);
		static ObjLink* Head() { return m_pHead; }
	private:
		void Link();
		void Unlink();

	private:
		T*				m_pObj;
		ObjLink*		m_pPrev;
		ObjLink*		m_pNext;

		static ObjLink* m_pHead;
	};

	template <class T>
	ObjLink<T>* ObjLink<T>::m_pHead = nullptr;

	template <class T>
	template <class Fn>
	void ObjLink<T>::VisitAll(const Fn& fn)
	{
		ObjLink* pCurrent = m_pHead;
		while (pCurrent)
		{
			ObjLink* pNext = pCurrent->m_pNext;
			fn(pCurrent->m_pObj);
			pCurrent = pNext;
		}
	}

	template <class T>
	void ObjLink<T>::Link()
	{
		if (m_pHead == nullptr)
		{
			m_pHead = this;
		}
		else
		{
			m_pNext = m_pHead;
			m_pHead->m_pPrev = this;
			m_pHead = this;
		}
	}

	template <class T>
	void ObjLink<T>::Unlink()
	{
		if (m_pPrev)
			m_pPrev->m_pNext = m_pNext;
		if (m_pNext)
			m_pNext->m_pPrev = m_pPrev;
		if (m_pHead == this)
			m_pHead = m_pNext;
		m_pPrev = m_pNext = nullptr;
	}
}