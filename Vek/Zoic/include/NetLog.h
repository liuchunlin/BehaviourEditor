/*! @file
	@ingroup buildin
	@brief ����������־��
*/

#ifndef __ZOIC_NETLOG_H
#define __ZOIC_NETLOG_H

#include "Types.h"
#include <Vek/Base/Module.h>

namespace Zoic
{
	/*! @brief ��־��
		@ingroup buildin


		�������շ����ݶ���¼������
	*/
	class NetLog:
		public Vek::Module<NetLog>
	{
		friend class Vek::Singleton<NetLog>;
	public:
		/*! @brief ����������־
			
			
			����������־����Ŀ¼��,��Ҫ��־ǰ׺�ͺ�׺(���Ϊ����д��Ҫ��־)<br>
			ֻ���������˸ú������Ժ���д������־ʱ����Ч������д
		*/
		void config(
			const char *basedir,		//!< ����Ŀ¼
			const char *prefix=NULL,	//!< ��Ҫ��־ǰ׺
			const char *suffix=NULL		//!< ��Ҫ��־��׺
			);
		
		/*! @brief д������־
			
			
			��ʹ��ǰӦ�ȵ���config,������ǿպ�������
		*/
		void netlog(
			const char *ip,			//IP��ַ
			DWORD port,				//�˿ں�
			const char *local_ip,	//����IP��ַ
			DWORD local_port,		//���ض˿ں�
			const void *buffer,		//���ݻ���
			int len,				//���ݳ���
			bool recv				//���ձ�־(true:����;false:����)
			);

		/*! @brief д������־


		ͨ��ÿ����Config��������ʱд������־
		*/
		void netlogWithConfig(
			const char *basedir,			//!< ����Ŀ¼(��β������б��)
			const char *ip,					//!< IP��ַ
			DWORD port,						//!< �˿ں�
			const char *binary_pathname,	//!< �����ļ���(�����basedir)
			const void *buffer,				//!< ���ݻ���
			int len,						//!< ���ݳ���
			bool recv,						//!< ���ձ�־(true:����;false:����)
			const char *brief_pathname		//!< ��Ҫ��־·��
			);
	private:
		NetLog();
		//!������־
		void local_flog(
			const char *pathname,	//!< ·��
			char *msg,	//!< ���Ƹ�ʽ
			...);

		//!����ʱ��
		void updateTime();
		const char *m_basedir;		//!< ��־����Ŀ¼
		const char *m_prefix;		//!< ǰ׺
		const char *m_suffix;		//!< ��׺
		TIME_SEC	m_lasttime;		//!< ��һ�ε���log��ʱ��
		char 		m_timebuf[20];	//!< ʱ���ַ���ֵ
		char		m_datebuf[20];	//!< �����ַ���ֵ
	};
}

#define netlog Zoic::NetLog::GetInstance()->netlog
#define g_Netlog Zoic::NetLog::getInstance()

#endif
