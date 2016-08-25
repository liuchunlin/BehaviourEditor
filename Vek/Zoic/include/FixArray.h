/*! @file
	@ingroup utils
	@brief ��������ģ��
*/

#ifndef __ZOIC_FIXARRAY_H
#define __ZOIC_FIXARRAY_H
#include "Types.h"

namespace Zoic
{
	/*!
		@brief ����ģ��
		@ingroup utils
		@param T Ԫ������
		@param LEN ������󳤶�


		��Ҫ���������С�Ϊ��ʵ�ִ�Ŀ��,���������ڵ�����ֻ����ƽ����������
	*/
	template <class T,unsigned int LEN>
	class FixArray
	{
	public:
		typedef T &			reference;
		typedef const T	&	const_reference;
		typedef unsigned int	size_type;
		
		//@}
		reference operator[](size_type pos);
		const_reference operator[](size_type pos) const;
		//!�õ������ʼԪ��ָ��
		T * getData();
		const T * getData() const;

		//!��ȱȽϲ�����
		bool operator == (const FixArray & o) const;

		//!���ȱȽϲ�����
		bool operator != (const FixArray & o) const;

		static size_type getSizeLimit();
	private:
		T		m_data[LEN];	//!< ��������
	};
	
	template <class T,unsigned int LEN>
	inline typename FixArray<T,LEN>::size_type FixArray<T,LEN>::getSizeLimit( )
	{
		return LEN;
	}

	template <class T,unsigned int LEN>
	inline typename FixArray<T,LEN>::reference FixArray<T,LEN>::operator[](size_type pos)
	{
		return m_data[pos];
	}

	template <class T,unsigned int LEN>
	inline typename FixArray<T,LEN>::const_reference FixArray<T,LEN>::operator[](size_type pos) const
	{
		return m_data[pos];
	}
	
	template <class T,unsigned int LEN>
	inline T * FixArray<T,LEN>::getData()
	{
		return m_data;
	}	

	template <class T,unsigned int LEN>
	inline const T * FixArray<T,LEN>::getData() const
	{
		return m_data;
	}	

	template <class T,unsigned int LEN>
	bool FixArray<T,LEN>::operator == (const FixArray & o) const
	{
		for(size_type i=0;i<LEN;i++)
		{
			if( m_data[i]!=o.m_data[i])
			{
				return false;
			}
		}
		return true;
	}

	template <class T,unsigned int LEN>
	bool FixArray<T,LEN>::operator != (const FixArray & o) const
	{
		return !(*this==o);
	}
};

#endif
