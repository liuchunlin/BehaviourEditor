

namespace Vek
{
	//!抛出越界异常类
	extern void throwOutOfBoundException(
		const char * format,	//!< 格式控制字符串
		...);
}

namespace Vek
{
	template<class T>
	void addBeforeMessage(const T& arg,const Vek::Stream& msgOrg,Vek::Stream& msgOutput)
	{
		msgOutput.ID(msgOrg.ID());
		msgOutput << arg;

		WORD sizeOrg=msgOrg.getSize() - NETWORK_MSG_HEADER;
		WORD sizeBegin=msgOutput.getSize();
		if(sizeBegin+sizeOrg>NETWORK_BUF_SIZE)
		{
			Vek::throwOutOfBoundException("[ID:%d] Message::addBeforeMessage %d + %d out of bound",msgOrg.ID(),sizeBegin,sizeOrg);
		}
		BYTE * buf=const_cast<BYTE *>(msgOutput.getData());
		*(WORD *)buf=sizeBegin+sizeOrg;
		memmove(buf+sizeBegin,msgOrg.getData()+NETWORK_MSG_HEADER,sizeOrg);
	}

	//!在std::vector中可以存放在最大元素数量
	const int	VECTOR_MAX_ELEMENTS = 0xFF;

	template<class T>
	Vek::Stream& operator << (Vek::Stream& msg,const std::vector<T> & arg)
	{
		if(arg.size() > VECTOR_MAX_ELEMENTS)
		{
			Vek::throwOutOfBoundException("[ID:%d] Message::<< vector size(%d) larger than %d",msg.ID(),arg.size(),VECTOR_MAX_ELEMENTS);
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
	Vek::Stream& operator >> (Vek::Stream& msg,std::vector<T>& arg)
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

	void setString(Vek::Stream & msg,const char *str,WORD buffSize);
	void getString(Vek::Stream & msg,char *str,WORD buffSize);
}


