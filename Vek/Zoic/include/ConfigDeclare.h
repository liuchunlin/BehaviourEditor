/*! @file
	@ingroup utils
	@brief �滻CONFIG_DECLARE,ʵ�������ļ����Զ���ȡ
*/
#ifdef CONFIG_DEFINE
#undef CONFIG_DEFINE
#endif

/*! @brief ����������
	@param TYPE ����������
	@param NAME ����������
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
	@brief �滻CONFIG_DECLARE,ʵ�������ļ����Զ���ȡ,���û���ҵ���������ʹ��Ĭ��ֵ
*/
#ifdef CONFIG_DEFINE_EX
#undef CONFIG_DEFINE_EX
#endif

/*! @brief ����������
	@param TYPE ����������
	@param NAME ����������
	@param VALUE ������Ĭ��ֵ
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
