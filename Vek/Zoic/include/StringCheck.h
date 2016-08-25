/*! @file
	@ingroup buildin
	@brief �ַ�������У��
*/
#ifndef __ZOIC_STRINGCHECK_H
#define __ZOIC_STRINGCHECK_H
#include "Types.h"
#include "NoCopy.h"
#include <vector>
#include <string>

namespace Zoic
{
	/*! @brief ����Ϣ�洢ģ��
		@ingroup buildin
	*/

	class StringCheck
		:public NoCopy
	{
	public:
		enum CHECK_RESULT
		{
			CR_NORMAL,			//�ַ�����������������Ҫ�����ַ�
			CR_FORBIDDEN		//������Ҫ�����ַ����滻�ɹ�
		};
		static bool isGB2312NoNumberSpaceCode(const char* pStr);
		static bool hasForbiddenString( const char* pStr,const std::vector<std::string>& forbiddenList);
		static CHECK_RESULT replaceForbiddenString( char* pStr,const std::vector<std::string>& forbiddenList);
	};
}

#endif
