/*! @file
	@ingroup utils
	@brief 替换CONFIG_DECLARE,实现配置文件的自动读取
*/
#ifdef CONFIG_DEFINE
#undef CONFIG_DEFINE
#endif

/*! @brief 声明配置项
	@param TYPE 配置项类型
	@param NAME 配置项名称
*/
#define CONFIG_DEFINE(TYPE,NAME) \
	private: \
		TYPE m_##NAME; \
	public: \
		typedef TYPE NAME##_TYPE; \
		const NAME##_TYPE & get##NAME() \
		{ \
			ASSERT( isLoaded() ) \
			return m_##NAME; \
		}


/*! @file
	@ingroup utils
	@brief 替换CONFIG_DECLARE,实现配置文件的自动读取,如果没有找到配置项则使用默认值
*/
#ifdef CONFIG_DEFINE_EX
#undef CONFIG_DEFINE_EX
#endif

/*! @brief 声明配置项
	@param TYPE 配置项类型
	@param NAME 配置项名称
	@param VALUE 配置项默认值
*/
#define CONFIG_DEFINE_EX(TYPE,NAME,VALUE) \
	private: \
		TYPE m_##NAME; \
	public: \
		typedef TYPE NAME##_TYPE; \
		const NAME##_TYPE & get##NAME() \
		{ \
			ASSERT( isLoaded() ) \
			return m_##NAME; \
		}

#ifndef __CONFIGDECLARE_H
#define __CONFIGDECLARE_H

#endif
