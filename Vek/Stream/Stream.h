#pragma once

#include "../Base/Types.h"
#include <string>
#include <vector>


namespace Vek
{
	class Stream;

	//���ڱ������������״̬
	class StreamState
	{
		friend class Stream;
	private:
		BYTE * m_pRead;		//!< ��ȡλ��
		BYTE * m_pWrite;	//!< д��λ��
		Stream * m_object;	//!< ������Ϣ����ָ��
	};

	//��������
	class Stream
	{
	public:
		//���ڽ�һ���ڴ�ǿתΪMessage����
		struct Fake
		{
			WORD *	pSize;				//!< ָ�򳤶�ָ��
			WORD *	pID;				//!< ָ����ϢIDָ��
			BYTE *	pRead;				//!< ��ȡλ��
			BYTE *	pWrite;				//!< д��λ��
			BYTE *	buffer[1];			//!< ���ݻ���
		};

		Stream();

		//!�������
		void clear();
		/*! @brief ������ϢID
			@return ���������
		*/
		Stream & ID(
			WORD wID	//!< ��ϢID
			);

		// ���ʣ�����ݴ�С
		int getRemainingRData();

		//!���ID
		WORD ID() const;

		//!�����Ϣ���߼����ݳ���
		WORD getSize() const;

		//!�����Ϣ�߼�����ָ��
		const BYTE * getData() const;
		
		//!����״̬
		void	saveState(
			StreamState &state	//!< ��Ϣ״̬
			);
		//!����״̬
		void	loadState(
			const StreamState &state	//!< ��Ϣ״̬
			);

		/*! @brief ���ַ���
			@exception Vek::OutOfBoundException *������Ҳ�������С��size���ַ���
		*/
		void	readString(
			char *buffer,	//!< �ַ�������ָ��
			int size		//!< �����С
			);

		/*! @brief ��һ�����ȵ�����
			@exception Vek::OutOfBoundException *�����Ȳ���
		*/
		void	readData(
			void * pData,	//!< ���ݻ���ָ��
			int n			//!< ��ȡ�����ݳ���
			);

		/*! @brief ����һ�����ȵ�����
			@exception Vek::OutOfBoundException *�����Ȳ���
		*/
		void skipData(
			int n			//!< �����ĳ���
			);

		/*! @brief д��һ����������
			@exception Vek::OutOfBoundException *��д�����ݳ������泤��
		*/
		void	writeData(
			const void * pData,
			int n );

		//!���д��λ��
		void * getWritePosition() const;

		//!��ö�ȡλ��
		void * getReadPosition() const;

		Stream& operator = (const Stream& o);
		
		//@{
		/*! @brief �������,������д����Ϣ��
			@exception Vek::OutOfBoundException *��д�����ݳ������泤��
			@return �����������
			@param val �������			
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
		/*! @brief �������,����Ϣ�ж�ȡ����
			@exception Vek::OutOfBoundException *����ȡ���ݳ������泤��
			@return �����������
			@param val �������			
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
		WORD * m_pSize;					//!< ָ�򳤶�ָ��
		WORD * m_pID;					//!< ָ����ϢIDָ��
		BYTE * m_pRead;					//!< ��ȡλ��
		BYTE * m_pWrite;				//!< д��λ��
		BYTE m_pBuf[NETWORK_BUF_SIZE];	//!< ���ݻ���
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

