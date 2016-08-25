/*! @file
	@ingroup utils
	@brief ����������
*/

#ifndef __ZOIC_CONFIG_H
#define __ZOIC_CONFIG_H

#include <string>
#include <map>
#include "DateTime.h"

namespace Zoic
{
	/*!
		@brief ������
		@ingroup utils


		��ȡ�����ļ�
	*/
	class Config
	{
		protected:
			Config();
			//!���������ļ�
			void load(
				const char *file	//!< �ļ���
				);
			std::map<std::string,std::string>	m_vars;	//! ���ñ���
			bool isLoaded() const;
		private:
			bool m_loaded;
	};

	inline bool Config::isLoaded() const
	{
		return m_loaded;
	}
}

#endif
