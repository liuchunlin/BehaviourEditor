#pragma once

#include "../Base/Types.h"
#include <string>
#include <vector>


namespace Vek
{
	class Stream;

	//用于保存数据流类的状态
	class StreamState
	{
		friend class Stream;
	private:
		BYTE * m_pRead;		//!< 读取位置
		BYTE * m_pWrite;	//!< 写入位置
		Stream * m_object;	//!< 网络消息对象指针
	};

	//数据流类
	class Stream
	{
	public:
		//用于将一段内存强转为Message对象
		struct Fake
		{
			WORD *	pSize;				//!< 指向长度指针
			WORD *	pID;				//!< 指向消息ID指针
			BYTE *	pRead;				//!< 读取位置
			BYTE *	pWrite;				//!< 写入位置
			BYTE *	buffer[1];			//!< 数据缓存
		};

		Stream();

		//!清空数据
		void clear();
		/*! @brief 设置消息ID
			@return 自身的引用
		*/
		Stream & ID(
			WORD wID	//!< 消息ID
			);

		// 获得剩余数据大小
		int getRemainingRData();

		//!获得ID
		WORD ID() const;

		//!获得消息里逻缉数据长度
		WORD getSize() const;

		//!获得消息逻缉数据指针
		const BYTE * getData() const;
		
		//!保存状态
		void	saveState(
			StreamState &state	//!< 消息状态
			);
		//!载入状态
		void	loadState(
			const StreamState &state	//!< 消息状态
			);

		/*! @brief 读字符串
			@exception Vek::OutOfBoundException *　如果找不到长度小于size的字符串
		*/
		void	readString(
			char *buffer,	//!< 字符串缓存指针
			int size		//!< 缓存大小
			);

		/*! @brief 读一定长度的数据
			@exception Vek::OutOfBoundException *　长度不足
		*/
		void	readData(
			void * pData,	//!< 数据缓存指针
			int n			//!< 读取的数据长度
			);

		/*! @brief 跳过一定长度的数据
			@exception Vek::OutOfBoundException *　长度不足
		*/
		void skipData(
			int n			//!< 跳过的长度
			);

		/*! @brief 写入一定长度数据
			@exception Vek::OutOfBoundException *　写入数据超过缓存长度
		*/
		void	writeData(
			const void * pData,
			int n );

		//!获得写入位置
		void * getWritePosition() const;

		//!获得读取位置
		void * getReadPosition() const;

		Stream& operator = (const Stream& o);
		
		//@{
		/*! @brief 输出操作,将数据写入消息中
			@exception Vek::OutOfBoundException *　写入数据超过缓存长度
			@return 对自身的引用
			@param val 输出参数			
		*/
		Stream & operator << (const char & val);
		Stream & operator << (const unsigned char & val);
		Stream & operator << (const short & val);
		Stream & operator << (const unsigned short & val);
		Stream & operator << (const int & val);
		Stream & operator << (const unsigned int & val);
		Stream & operator << (const long & val);
		Stream & operator << (const unsigned long & val);
		Stream & operator << (const long long & val);
		Stream & operator << (const unsigned long long & val);
		Stream & operator << (const float & val);
		Stream & operator << (const double & val);
		Stream & operator << (const PCSTR & val);
		Stream & operator << (const std::string & val);
		//@}
		
		//@{
		/*! @brief 输入操作,从消息中读取数据
			@exception Vek::OutOfBoundException *　读取数据超过缓存长度
			@return 对自身的引用
			@param val 输入参数			
		*/
		Stream & operator >> (char & val);
		Stream & operator >> (unsigned char & val);
		Stream & operator >> (short & val);
		Stream & operator >> (unsigned short & val);
		Stream & operator >> (int & val);
		Stream & operator >> (unsigned int & val);
		Stream & operator >> (long & val);
		Stream & operator >> (unsigned long & val);
		Stream & operator >> (long long & val);
		Stream & operator >> (unsigned long long & val);
		Stream & operator >> (float & val);
		Stream & operator >> (double & val);
		Stream & operator >> (std::string &val);
		Stream & operator >> (std::wstring &val);
		Stream & operator >> (PCSTR & val);
		//@}
	private:
		WORD * m_pSize;					//!< 指向长度指针
		WORD * m_pID;					//!< 指向消息ID指针
		BYTE * m_pRead;					//!< 读取位置
		BYTE * m_pWrite;				//!< 写入位置
		BYTE m_pBuf[NETWORK_BUF_SIZE];	//!< 数据缓存
	};
	
	inline void Stream::clear()
	{
		m_pSize = (WORD *)&m_pBuf[0];
		m_pID	= (WORD *)&m_pBuf[2];
		m_pRead = m_pWrite = &m_pBuf[NETWORK_MSG_HEADER];
	}
	
	inline Stream & Stream::ID (WORD wID)
	{
		* m_pID		= wID;
		* m_pSize	= NETWORK_MSG_HEADER;                  
		m_pRead		= m_pWrite = &m_pBuf[NETWORK_MSG_HEADER];
		return *this;
	}
	
	inline int Stream::getRemainingRData()
	{
		if ( m_pRead < m_pBuf + getSize() )
		{
			return static_cast<int>( m_pBuf + getSize() - m_pRead );
		}
		return 0;
	}
	
	inline WORD Stream::ID() const
	{
		return *m_pID;
	}
	
	inline WORD Stream::getSize() const
	{
		return *m_pSize;
	}

	inline const BYTE * Stream::getData() const
	{
		return m_pBuf;
	}
	
	inline void Stream::saveState(StreamState &state)
	{
		state.m_object=this;
		state.m_pRead=m_pRead;
		state.m_pWrite=m_pWrite;
	}

	inline void Stream::loadState(const StreamState &state)
	{
		if(this==state.m_object)
		{
			m_pWrite=state.m_pWrite;
			m_pRead=state.m_pRead;
		}
	}

	inline void * Stream::getWritePosition() const
	{
		return m_pWrite;
	}

	inline void * Stream::getReadPosition() const
	{
		return m_pRead;
	}

	inline Stream& Stream::operator = (const Stream& o)
	{
		clear();
		memmove(m_pBuf,o.getData(),o.getSize());
		return *this;
	}

	inline Stream & Stream::operator << (const char & val)
	{
		writeData(&val,sizeof(val));
		return *this;
	}
	
	inline Stream & Stream::operator << (const unsigned char & val)
	{
		writeData(&val,sizeof(val));
		return *this;
	}
	
	inline Stream & Stream::operator << (const short & val)
	{
		writeData(&val,sizeof(val));
		return *this;
	}
	
	inline Stream & Stream::operator << (const unsigned short & val)
	{
		writeData(&val,sizeof(val));
		return *this;
	}
	
	inline Stream & Stream::operator << (const int & val)
	{
		writeData(&val,sizeof(val));
		return *this;
	}
	
	inline Stream & Stream::operator << (const unsigned int & val)
	{
		writeData(&val,sizeof(val));
		return *this;
	}
	
	inline Stream & Stream::operator << (const long & val)
	{
		writeData(&val,sizeof(val));
		return *this;
	}
	
	inline Stream & Stream::operator << (const unsigned long & val)
	{
		writeData(&val,sizeof(val));
		return *this;
	}

	inline Stream & Stream::operator << (const long long & val)
	{
		writeData(&val,sizeof(val));
		return *this;
	}
	
	inline Stream & Stream::operator << (const unsigned long long & val)
	{
		writeData(&val,sizeof(val));
		return *this;
	}

	inline Stream & Stream::operator << (const float & val)
	{
		writeData(&val,sizeof(val));
		return *this;
	}
	
	inline Stream & Stream::operator << (const double & val)
	{
		writeData(&val,sizeof(val));
		return *this;
	}
		
	inline Stream & Stream::operator << (const PCSTR & val)
	{
		writeData(val, static_cast<int>(strlen(val) + 1));
		return * this;
	}
	
	inline Stream & Stream::operator << (const std::string & val)
	{
		writeData(val.c_str(), static_cast<int>(val.length() + 1));
		return * this;
	};
		
	inline Stream & Stream::operator >> (char & val)
	{
		readData(&val,sizeof(val));
		return *this;
	}
	
	inline Stream & Stream::operator >> (unsigned char & val)
	{
		readData(&val,sizeof(val));
		return *this;
	}
	
	inline Stream & Stream::operator >> (short & val)
	{
		readData(&val,sizeof(val));
		return *this;
	}
	
	inline Stream & Stream::operator >> (unsigned short & val)
	{
		readData(&val,sizeof(val));
		return *this;
	}
	
	inline Stream & Stream::operator >> (int & val)
	{
		readData(&val,sizeof(val));
		return *this;
	}
	
	inline Stream & Stream::operator >> (unsigned int & val)
	{
		readData(&val,sizeof(val));
		return *this;
	}
	
	inline Stream & Stream::operator >> (long & val)
	{
		readData(&val,sizeof(val));
		return *this;
	}
	
	inline Stream & Stream::operator >> (unsigned long & val)
	{
		readData(&val,sizeof(val));
		return *this;
	}

	inline Stream & Stream::operator >> (long long & val)
	{
		readData(&val,sizeof(val));
		return *this;
	}
	
	inline Stream & Stream::operator >> (unsigned long long & val)
	{
		readData(&val,sizeof(val));
		return *this;
	}

	inline Stream & Stream::operator >> (float & val)
	{
		readData(&val,sizeof(val));
		return *this;
	}
	
	inline Stream & Stream::operator >> (double & val)
	{
		readData(&val,sizeof(val));
		return *this;
	}
}

#include "Stream.inl"

