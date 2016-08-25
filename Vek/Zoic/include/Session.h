/*! @file
	@ingroup network
	@brief 定义网络连接基类
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
		@brief 网络连接基类
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

		//!得到远端IP地址
		const char * getIP() const;

		//!得到远端端口号
		WORD getPort() const;

		//!得到本地IP地址
		const char * getLocalIP() const;

		//!得到本地端口号
		WORD getLocalPort() const;

		//!设置错误号
		void setError(
			int error	/*!< 如果不为0则连接断开<br>
								<0 系统定义错误号<br>
								>0 自定义错误号
							*/
			);
	
		//!得到错误号
		int	getError(void);

		//!打开连接
		virtual bool open(
			SOCKET sock,				//!< 套接字
			const sockaddr_in &addr		//!< 远程地址
			);

		/*! @brief 接收发送消息例程
		*/
		virtual bool routine();

		//!关闭连接
		virtual void close();

		//!解析消息
		virtual bool parseMessage(
			Message &msg		//!< 网络消息
			);

		//!发送消息
		virtual bool sendMessage(
			const Message &msg	//!< 要发送的消息
			);

		//!得到错误号
		int	getSystemError(void);
	protected:
		//!
		DWORD getSendBufferSize() const;
		//!
		DWORD getRecvBufferSize() const;
		//!调整发送缓存
		void adjustSendBuffer(
			DWORD handled_size				//!< 已处理长度
			);

		//!得到发送数据
		const char * getSendData();

		//!得到发送数据大小
		DWORD getSendDataSize();

		//!网络初始化
		void netInitialize();

		//!设置SYSPING时间
		bool setSysPingTime(
			const SysPingTime* sysPingTime	//!< SYSPING时间
			);

		DWORD		m_init_send_buffer_size;
		DWORD		m_init_recv_buffer_size;
		bool		m_sendbuf_dynamic;				//!< 发送缓存不足时是否动态分配
		bool		m_release_dynamic_sendbuf;		//!< 发送缓存是否动态释放
		DWORD		m_max_dynamic_sendbuf_size;

		TIME_MSEL	m_lastReceivedTick;						//!< 最后接收到数据的时间戳
		Message * 	m_receivedMessage;						//!< 当前接收到网络消息
	private:
		//!
		void clearSendBuffer();
		//!
		void clearRecvBuffer();
		//!向发送缓冲区添加数据
		bool pushData(const BYTE *data,WORD size);

		//!创建发送与接收缓存
		void initBuffer();
		//!还原buffer
		void revertBuffer();

		//!创建缓存
		static bool createBuffer(
			DWORD & size,			//!< 缓存长度,输入输出
			char * & raw,			//!< 原始数据,输出
			char * & data			//!< 实际数据,输出
			);

		//!调整接收缓存
		void adjustRecvBuffer(
			DWORD handled_size				//!< 已处理长度
			);

		//!增加发送缓存大小
		bool increaseSendBuffer();

		//! 接收数据
		void recvData();

		//! 发送数据
		void sendData();

		/*! @brief 解包处理

			从接收缓存中解出逻缉包，并调用parseMessage函数
		*/
		void uppack();

		BYTE	m_syspingCount;				//!< SYSPING探测计数

		const SysPingTime* m_syspingTimes;	//!< 系统PING时间和最大次数

		std::string	m_ip;					//!< IP地址
		WORD		m_port;					//!< 端口
		SOCKET		m_socket;				//!< 套接字
		std::string m_local_ip;				//!< 本地IP地址
		WORD		m_local_port;			//!< 本地端口

		DWORD 		m_send_buffer_size;		//!< 发送缓存大小
		DWORD 		m_recv_buffer_size;		//!< 接收缓存大小

		char * _m_send_buffer;		//!< 发送缓存,预留Message头
		char * m_send_buffer;		//!< 发送缓存,实际数据开始位置
		char * _m_recv_buffer;		//!< 接收缓存,预留Message头
		char * m_recv_buffer;		//!< 接收缓存,实际数据开始位置
		
		DWORD	m_send_size;		//!< 发送缓存中数据长度
		DWORD	m_recv_size;		//!< 接收缓存中数据长度
		WORD	m_packet_len;		//!< 逻辑包长度

		int	m_error;								//!< 错误号定义
		int m_systemError;							//!< 当前系统错误号
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
