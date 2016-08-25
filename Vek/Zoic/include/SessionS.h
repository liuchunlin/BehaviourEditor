/*! @file
	@ingroup network
	@brief �����������ӷ�����
*/

#ifndef __ZOIC_SESSIONS_H
#define __ZOIC_SESSIONS_H

#include "Session.h"
#include <list>

namespace Zoic
{
	class CommandFactory;
	class SessionS;
	typedef std::list<SessionS *> LIST_SESSIONS;
	/*! @brief �������ӷ�����
		@ingroup network


		��������������ӵ�һ��
	*/
	class SessionS
		:public Session
	{
		friend class SessionBank;
	public:
		SessionS();
		//!������Ӿ��
		const HandleType & getHandle();
		//!����������к�
		const SerialType & getSerial();
		//!�Ƿ�׼����
		bool isReady();		
	protected:
		//!������Ϣ
		bool parseMessage(
			Message &msg	//!< ������Ϣ
			);
		
		bool					m_ready;			//!< �ͻ���װ����ϱ�־
	private:
		HandleType				m_handle;			//!< ���
		SerialType				m_serial;			//!< ���к�
		LIST_SESSIONS::iterator	m_itList;			//!< ��List�е�λ��
		CommandFactory *		m_commandFactory;	//!< �����������
	};
	
	inline const HandleType & SessionS::getHandle()
	{
		return m_handle;
	}
	
	inline const SerialType & SessionS::getSerial()
	{
		return m_serial;
	}
	
	inline bool SessionS::isReady()
	{
		return m_ready;
	}
}

#endif
