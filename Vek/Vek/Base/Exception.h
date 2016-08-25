#pragma once

#include <exception>
#include <string>
#include <stdarg.h>


namespace Vek
{
	//�쳣����
	class  Exception: public std::exception
	{
	public:
		//���캯��
		Exception();
		Exception(
			const char * format,	//!< ��ʽ�����ַ���
			...
			);

		//����쳣��Ϣ
		const char *what() const throw();
		~Exception() throw();
	protected:
		std::string m_str;		//!< �쳣����
	};
}

//�����쳣�ַ���
#define MAKE_EXCEPTION_STRING(format) \
	va_list args; \
	char buf[8096]; \
	va_start(args, format); \
	vsnprintf(buf, sizeof(buf),format, args); \
	va_end(args); \
	buf[sizeof(buf)-1]='\0'; \
	m_str=buf; 


