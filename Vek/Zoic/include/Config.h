/*! @file
	@ingroup utils
	@brief 定义配置类
*/

#ifndef __ZOIC_CONFIG_H
#define __ZOIC_CONFIG_H

#include <string>
#include <map>
#include "DateTime.h"

namespace Zoic
{
	/*!
		@brief 配置类
		@ingroup utils


		读取配置文件
	*/
	class Config
	{
		protected:
			Config();
			//!载入配置文件
			void load(
				const char *file	//!< 文件名
				);
			std::map<std::string,std::string>	m_vars;	//! 配置变量
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
