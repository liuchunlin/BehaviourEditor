/*! @file
	@ingroup buildin
	@brief �����ź���
*/

#ifndef __ZOIC_SIGNAL_H
#define __ZOIC_SIGNAL_H
#ifndef ZOIC_NO_SERVICE

#include <Vek/Base/Module.h>


namespace Zoic
{
	/*! @brief �ź���
		@ingroup buildin
	*/
	class Signal: public Vek::Module<Signal>
	{
		friend class Vek::Singleton<Signal>;
	private :
		Signal();

		//!�������̣�����Ƿ����ź��ж�
		int run();

		//!�źŴ������
		static void sighandler(int sig);

		int	m_sig;	//!< ��һ���жϵ��ź�
	};
}
#endif
#endif
