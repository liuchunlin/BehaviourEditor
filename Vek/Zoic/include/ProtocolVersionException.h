/*! @file
	@ingroup buildin
	@brief ����Э��汾�쳣��
*/

#ifndef __ZOIC_PROTOCOLVERSIONEXCEPTION_H
#define __ZOIC_PROTOCOLVERSIONEXCEPTION_H

#include "Exception.h"

namespace Zoic
{
	/*! @brief Э��汾�쳣��
		@ingroup buildin
	*/
	class  ProtocolVersionException:
		public Exception
	{
	public:
		//!���캯��
		ProtocolVersionException(
			DWORD localVersion,	//!< ���ذ汾
			DWORD farVersion	//!< Զ�̰汾
			);
		virtual	~ProtocolVersionException() throw();
	};
	
	inline ProtocolVersionException::ProtocolVersionException(DWORD localVersion,DWORD farVersion):
			Exception("Invalid protocol version.(local:%d,far:%d)",
			localVersion,
			farVersion)
	{
	}
	
	inline ProtocolVersionException::~ProtocolVersionException() throw()
	{
	}
}

#endif
