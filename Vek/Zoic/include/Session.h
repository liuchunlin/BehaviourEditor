/*! @file
	@ingroup network
	@brief �����������ӻ���
*/

#ifndef __ZOIC_SESSION_H
#define __ZOIC_SESSION_H

#include "Types.h"
#include <string>
#include <list>
#ifdef HAVE_MINGW32
#include <winsock2.h>
#ifdef _MSC_VER
#pragma comment (lib, "ws2_32.lib")
#endif
#else
#include <netinet/in.h>
#endif
#include "NoCopy.h"
#include "Selectable.h"

namespace Zoic
{
	class Message;
	class SysPingTime;
	/*!
		@brief �������ӻ���
		@ingroup network
	*/
	class Session:
		public NoCopy,
		public Selectable

	{
		friend class SessionBank;
	public:
		Session();
		virtual ~Session();

		//!�õ�Զ��IP��ַ
		const char * getIP() const;

		//!�õ�Զ�˶˿ں�
		WORD getPort() const;

		//!�õ�����IP��ַ
		const char * getLocalIP() const;

		//!�õ����ض˿ں�
		WORD getLocalPort() const;

		//!���ô����
		void setError(
			int error	/*!< �����Ϊ0�����ӶϿ�<br>
								<0 ϵͳ��������<br>
								>0 �Զ�������
							*/
			);
	
		//!�õ������
		int	getError(void);

		//!������
		virtual bool open(
			SOCKET sock,				//!< �׽���
			const sockaddr_in &addr		//!< Զ�̵�ַ
			);

		/*! @brief ���շ�����Ϣ����
		*/
		virtual bool routine();

		//!�ر�����
		virtual void close();

		//!������Ϣ
		virtual bool parseMessage(
			Message &msg		//!< ������Ϣ
			);

		//!������Ϣ
		virtual bool sendMessage(
			const Message &msg	//!< Ҫ���͵���Ϣ
			);

		//!�õ������
		int	getSystemError(void);
	protected:
		//!
		DWORD getSendBufferSize() const;
		//!
		DWORD getRecvBufferSize() const;
		//!�������ͻ���
		void adjustSendBuffer(
			DWORD handled_size				//!< �Ѵ�����
			);

		//!�õ���������
		const char * getSendData();

		//!�õ��������ݴ�С
		DWORD getSendDataSize();

		//!�����ʼ��
		void netInitialize();

		//!����SYSPINGʱ��
		bool setSysPingTime(
			const SysPingTime* sysPingTime	//!< SYSPINGʱ��
			);

		DWORD		m_init_send_buffer_size;
		DWORD		m_init_recv_buffer_size;
		bool		m_sendbuf_dynamic;				//!< ���ͻ��治��ʱ�Ƿ�̬����
		bool		m_release_dynamic_sendbuf;		//!< ���ͻ����Ƿ�̬�ͷ�
		DWORD		m_max_dynamic_sendbuf_size;

		TIME_MSEL	m_lastReceivedTick;						//!< �����յ����ݵ�ʱ���
		Message * 	m_receivedMessage;						//!< ��ǰ���յ�������Ϣ
	private:
		//!
		void clearSendBuffer();
		//!
		void clearRecvBuffer();
		//!���ͻ������������
		bool pushData(const BYTE *data,WORD size);

		//!������������ջ���
		void initBuffer();
		//!��ԭbuffer
		void revertBuffer();

		//!��������
		static bool createBuffer(
			DWORD & size,			//!< ���泤��,�������
			char * & raw,			//!< ԭʼ����,���
			char * & data			//!< ʵ������,���
			);

		//!�������ջ���
		void adjustRecvBuffer(
			DWORD handled_size				//!< �Ѵ�����
			);

		//!���ӷ��ͻ����С
		bool increaseSendBuffer();

		//! ��������
		void recvData();

		//! ��������
		void sendData();

		/*! @brief �������

			�ӽ��ջ����н���߼�����������parseMessage����
		*/
		void uppack();

		BYTE	m_syspingCount;				//!< SYSPING̽�����

		const SysPingTime* m_syspingTimes;	//!< ϵͳPINGʱ���������

		std::string	m_ip;					//!< IP��ַ
		WORD		m_port;					//!< �˿�
		SOCKET		m_socket;				//!< �׽���
		std::string m_local_ip;				//!< ����IP��ַ
		WORD		m_local_port;			//!< ���ض˿�

		DWORD 		m_send_buffer_size;		//!< ���ͻ����С
		DWORD 		m_recv_buffer_size;		//!< ���ջ����С

		char * _m_send_buffer;		//!< ���ͻ���,Ԥ��Messageͷ
		char * m_send_buffer;		//!< ���ͻ���,ʵ�����ݿ�ʼλ��
		char * _m_recv_buffer;		//!< ���ջ���,Ԥ��Messageͷ
		char * m_recv_buffer;		//!< ���ջ���,ʵ�����ݿ�ʼλ��
		
		DWORD	m_send_size;		//!< ���ͻ��������ݳ���
		DWORD	m_recv_size;		//!< ���ջ��������ݳ���
		WORD	m_packet_len;		//!< �߼�������

		int	m_error;								//!< ����Ŷ���
		int m_systemError;							//!< ��ǰϵͳ�����
	};
	
	inline const char * Session::getIP() const
	{
		return m_ip.c_str();
	}
	
	inline WORD Session::getPort() const
	{
		return m_port;
	}

	inline const char * Session::getLocalIP() const
	{
		return m_local_ip.c_str();
	}

	inline WORD Session::getLocalPort() const
	{
		return m_local_port;
	}
	
	inline void Session::setError(int error)
	{
		if(!m_error)
		{
			m_error=error;
		}
	}
	
	inline int Session::getError(void)
	{
		return m_error;
	}

	inline int Session::getSystemError(void)
	{
		return m_systemError;
	}

	inline const char * Session::getSendData()
	{
		return m_send_buffer;
	}

	inline DWORD Session::getSendDataSize()
	{
		return m_send_size;
	}

	inline DWORD Session::getSendBufferSize() const
	{
		return m_send_buffer_size;
	}

	inline DWORD Session::getRecvBufferSize() const
	{
		return m_recv_buffer_size;
	}
}

#ifndef ON_MESSAGE
#define ON_MESSAGE template <const WORD cmd> static bool onMessage(Zoic::Session *pSession,Zoic::Message &msg);
#endif

#endif
