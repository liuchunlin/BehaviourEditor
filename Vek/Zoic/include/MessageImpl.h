/*! @file
	@ingroup utils
	@brief �����ַ���ģ��
*/

#ifndef __ZOIC_MESSAGEIMPL_H
#define __ZOIC_MESSAGEIMPL_H

#include "Message.h"
#include "Binary.h"
#include "FixArray.h"

namespace Zoic
{
	//!�׳�Խ���쳣��
	extern void throwOutOfBoundException(
		const char * format,	//!< ��ʽ�����ַ���
		...);
}

namespace Zoic
{
	template<class T>
	void addBeforeMessage(const T& arg,const Zoic::Message& msgOrg,Zoic::Message& msgOutput)
	{
		msgOutput.ID(msgOrg.ID());
		msgOutput << arg;

		WORD sizeOrg=msgOrg.getSize() - NETWORK_MSG_HEADER;
		WORD sizeBegin=msgOutput.getSize();
		if(sizeBegin+sizeOrg>NETWORK_BUF_SIZE)
		{
			Zoic::throwOutOfBoundException("[ID:%d] Message::addBeforeMessage %d + %d out of bound",msgOrg.ID(),sizeBegin,sizeOrg);
		}
		BYTE * buf=const_cast<BYTE *>(msgOutput.getData());
		*(WORD *)buf=sizeBegin+sizeOrg;
		memmove(buf+sizeBegin,msgOrg.getData()+NETWORK_MSG_HEADER,sizeOrg);
	}

	//!��std::vector�п��Դ�������Ԫ������
	const int	VECTOR_MAX_ELEMENTS = 0xFF;

	template<class T>
	Zoic::Message& operator << (Zoic::Message& msg,const std::vector<T> & arg)
	{
		if(arg.size() > VECTOR_MAX_ELEMENTS)
		{
			Zoic::throwOutOfBoundException("[ID:%d] Message::<< vector size(%d) larger than %d",msg.ID(),arg.size(),VECTOR_MAX_ELEMENTS);
		}
		BYTE size=static_cast<BYTE>(arg.size());
		msg << size;
		for(BYTE i=0;i<size;++i)
		{
			msg << arg[i];
		}		
		return msg;
	}

	template<class T>
	Zoic::Message& operator >> (Zoic::Message& msg,std::vector<T>& arg)
	{
		BYTE size;
		arg.clear();
		T tObj;
		msg >> size;
		for(BYTE i=0;i<size;++i)
		{
			msg >> tObj;
			arg.push_back(tObj);
		}
		return msg;
	}

	template<class T,unsigned int LEN>
	Zoic::Message& operator << (Zoic::Message& msg,const Zoic::FixArray<T,LEN> & arg)
	{
		for(typename Zoic::FixArray<T,LEN>::size_type i=0;i<LEN;++i)
		{
			msg << arg[i];
		}		
		return msg;
	}

	template<class T,unsigned int LEN>
	Zoic::Message& operator >> (Zoic::Message& msg,Zoic::FixArray<T,LEN> & arg)
	{
		for(typename Zoic::FixArray<T,LEN>::size_type i=0;i<LEN;++i)
		{
			msg >> arg.getData()[i];
		}
		return msg;
	}

	template<class T,unsigned int LEN,class SIZE_TYPE>
	Zoic::Message& operator << (Zoic::Message& msg,const Zoic::VarArray<T,LEN,SIZE_TYPE> & arg)
	{
		typename Zoic::VarArray<T,LEN,SIZE_TYPE>::size_type size=arg.size();
		if(size > LEN)
		{
			Zoic::throwOutOfBoundException("[ID:%d] Message::<< VarArray size(%d) larger than %d",msg.ID(),size,LEN);
		}
		msg << size;
		for(WORD i=0;i<size;++i)
		{
			msg << arg[i];
		}		
		return msg;
	}

	template<class T,unsigned int LEN,class SIZE_TYPE>
	Zoic::Message& operator >> (Zoic::Message& msg,Zoic::VarArray<T,LEN,SIZE_TYPE> & arg)
	{
		typename Zoic::VarArray<T,LEN,SIZE_TYPE>::size_type size;
		arg.clear();
		T tObj;
		msg >> size;
		if(size > LEN)
		{
			Zoic::throwOutOfBoundException("[ID:%d] Message:: >> VarArray size(%d) larger than %d",msg.ID(),size,LEN);
		}
		for(WORD i=0;i<size;++i)
		{
			msg >> tObj;
			arg.push_back(tObj);
		}
		return msg;
	}

	template<WORD LEN>
	Zoic::Message& operator << (Zoic::Message& msg,const Zoic::Binary<LEN> & arg)
	{
		WORD size=arg.size();
		if(size > LEN)
		{
			Zoic::throwOutOfBoundException("[ID:%d] Message::<< Binary size(%d) larger than %d",msg.ID(),size,LEN);
		}
		msg << size;
		msg.writeData(arg.getData(),size);
		return msg;
	}

	template<WORD LEN>
	Zoic::Message& operator >> (Zoic::Message& msg,Zoic::Binary<LEN> & arg)
	{
		WORD size;
		arg.clear();
		msg >> size;
		if(size > LEN)
		{
			Zoic::throwOutOfBoundException("[ID:%d] Message:: >> Binary size(%d) larger than %d",msg.ID(),size,LEN);
		}
		if(size > msg.getRemainingRData())
		{
			Zoic::throwOutOfBoundException("[ID:%d] Message::ReadData > getRemainingRData()",msg.ID());
		}
		const char * data = reinterpret_cast<const char*>(msg.getReadPosition());
		msg.skipData(size);
		arg.setData( data , size);
		return msg;
	}

	namespace MessageImpl
	{
		void setString(Zoic::Message & msg,const char *str,WORD buffSize);
		void getString(Zoic::Message & msg,char *str,WORD buffSize);
	}
}


#endif
