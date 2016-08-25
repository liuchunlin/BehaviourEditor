#ifndef __RANA_EVENT_H_
#define __RANA_EVENT_H_

#include <vector>
#include <string>
#include "RanaHeader.h"
#include "RanaMessage.h"
#include "RanaSortLable.h"

class RanaSpace;
class RanaEvent
{
	friend class RanaEventBank;
public:
/*	struct Argument	//事件参数
	{
		enum ArgumentType	//事件参数类型定义
		{
			AT_BOOL,	//布尔型
			AT_INTEGER,	//整型
			AT_STRING,	//字符串型
		} type;	//ArgumentType
		union BODY
		{
			bool			bVal;	//布尔型值 (type==AT_BOOL)
			int				iVal;	//整型值 (type==AT_INTEGER)
			const char *	sVal;	//字符串值 (type==AT_STRING)
		} body;	
		operator int () const;
		operator bool() const;
		operator const char * () const;
		Argument & operator=(int val);
		Argument & operator=(bool val);
		Argument & operator=(std::string val);
		Argument & operator=(const char * val);
	};*/
	RanaEvent();
	virtual ~RanaEvent();
	virtual bool operator ==(const RanaEvent &obj) const;

	// 预检查
	virtual bool		preValidate(RanaMessage *) = 0;
	virtual void		getArgument(std::vector<RanaVariant> &) = 0;

	void				setType(RanaEventType _type);
	RanaEventType		getType();
	void				setOnOff(bool _isOn);
	bool				getOnOff();

	void				setTriggerHandle(const RanaWORD &_triggerHandle);
	const RanaWORD&		getTriggerHandle();
	const RanaWORD&		getHandle();

	RanaSpace*			getSelfSpace();
#if 0	//FORNAME
	void				setName(const RanaEventName &_name);
	const RanaEventName &getName();
#endif
	virtual const RanaSortLable* getSortLable() const
	{
		return NULL;
	}
protected:
	std::vector<RanaVariant>	m_args;	//参数列表
private:
#if 0	//FORNAME
	RanaEventName				m_name;
#endif
	RanaSpaceName				m_spaceName;
	RanaWORD					m_triggerHandle;
	RanaWORD					m_handle;
	RanaEventType				m_type;
	bool						m_isOn;
	RanaSpace*					m_pRanaSpace;
};

inline void RanaEvent::setTriggerHandle(const RanaWORD &_triggerHandle)
{
	m_triggerHandle = _triggerHandle;
}

inline const RanaWORD& RanaEvent::getTriggerHandle()
{
	return m_triggerHandle;
}

inline const RanaWORD& RanaEvent::getHandle()
{
	return m_handle;
}
/*
inline RanaEvent::Argument::operator bool () const
{
	if(type==AT_BOOL)
	{
		return body.bVal;
	}
	throw static_cast<RanaBadEventArgument *>(0);
}

inline RanaEvent::Argument::operator int () const
{
	if(type==AT_INTEGER)
	{
		return body.iVal;
	}
	throw static_cast<RanaBadEventArgument *>(0);
}

inline RanaEvent::Argument::operator const char * () const
{
	if(type==AT_STRING)
	{
		return body.sVal;
	}
	throw static_cast<RanaBadEventArgument *>(0);
}

inline RanaEvent::Argument & RanaEvent::Argument::operator=(bool val)
{
	type=AT_BOOL;
	body.bVal=val;
	return *this;
}

inline RanaEvent::Argument & RanaEvent::Argument::operator=(int val)
{
	type=AT_INTEGER;
	body.iVal=val;
	return *this;
}

inline RanaEvent::Argument & RanaEvent::Argument::operator=(const char * val)
{
	type=AT_STRING;
	body.sVal=val;
	return *this;
}

inline RanaEvent::Argument & RanaEvent::Argument::operator=(std::string val)
{
	type=AT_STRING;
	body.sVal=val.c_str();
	return *this;
}
*/
#endif //__RANA_EVENT_H_
