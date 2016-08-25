#include "include/Message.h"
#include "include/OutOfBoundException.h"
#include "include/MessageImpl.h"

namespace Zoic
{
	namespace MessageImpl
	{

		void setString(Message & msg,const char *str,WORD buffSize)
		{
			size_t strLength = strlen(str);
			if( strLength >= static_cast<size_t>( buffSize ) )
			{
				Zoic::throwOutOfBoundException("[ID:%d] Message::<< String size(%d) larger than %d",msg.ID(),strLength,buffSize-1);
				//msg.writeData(str, buffSize - 1 );
				//msg.writeData("\0", 1);
				//return;
			}
			msg.writeData(str, static_cast<int>( strLength + 1) );
		}

		void getString(Message & msg,char *str,WORD buffSize)
		{
			msg.readString(str,buffSize);
		}

	}
}
