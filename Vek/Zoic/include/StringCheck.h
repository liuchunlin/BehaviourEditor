/*! @file
	@ingroup buildin
	@brief 字符串基本校验
*/
#ifndef __ZOIC_STRINGCHECK_H
#define __ZOIC_STRINGCHECK_H
#include "Types.h"
#include "NoCopy.h"
#include <vector>
#include <string>

namespace Zoic
{
	/*! @brief 类信息存储模板
		@ingroup buildin
	*/

	class StringCheck
		:public NoCopy
	{
	public:
		enum CHECK_RESULT
		{
			CR_NORMAL,			//字符串正常，不存在需要屏蔽字符
			CR_FORBIDDEN		//存在需要屏蔽字符，替换成功
		};
		static bool isGB2312NoNumberSpaceCode(const char* pStr);
		static bool hasForbiddenString( const char* pStr,const std::vector<std::string>& forbiddenList);
		static CHECK_RESULT replaceForbiddenString( char* pStr,const std::vector<std::string>& forbiddenList);
	};
}

#endif
