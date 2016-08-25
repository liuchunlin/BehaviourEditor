/*! @file
	@brief 定义数据读取模板
*/
#ifndef __DATAREADER_H
#define __DATAREADER_H

#include <vector>
#include <set>
#include <utility>
#include <Zoic/include/NoCopy.h>
#include <Zoic/include/Message.h>
#include <Zoic/include/Log.h>
#include <Zoic/include/OutOfBoundException.h>
#include "DataImage.h"

/*! @brief 数据读取特性模板
	@param ITEM 元素类型
*/
template < class ITEM > struct DataReaderTraits
{
	typedef DWORD ID;	//!< 元素ID类型,默认为DWORD
};

/*! @brief 数据读取模板
	@param ITEM 元素类型
*/
template < class ITEM > class DataReader : public Zoic::NoCopy
{
public:
	class CmpID
	{
	public:
		const bool operator()(const ITEM * a,const ITEM *b) const
		{
			return a->id < b->id;
		}
	};
	typedef std::set<ITEM *,CmpID> SET;		//!< 条目集合


	~DataReader();

	//!获得安全数据(副本)
	void getData(std::vector<ITEM>& output) const;

	//!获得数据
	const SET & getUnSafeData() const;

	//!根据ID获得安全条目
	DataImage<ITEM> getItem(
		const typename DataReaderTraits<ITEM>::ID &id	//!< 条目ID
		) const;

	//!根据ID获得条目
	const ITEM * getUnSafeItem(
		const typename DataReaderTraits<ITEM>::ID &id,	//!< 条目ID
		bool useCache = true //!< 使用cache
		) const;	

protected:
	//!构造函数
	DataReader(
		const char *file	//!< 保存数据的文件名
		);
	DataReader();	

	/*! @brief 通过条目基类扩展数据
		
		主要用在派生类的构造函数中
	*/
	template < class BASE_ITEM>
	bool extend(
		const char *file	//< 保存数据的文件名
		);
	SET 		m_data;		//!< 条目数据
	bool		m_loaded;
	bool load(const char *file);

	void clearData();	

	// 08/14/2009-13:04 by YYBear [BEGIN]
	// 最近一次cache
	mutable bool m_cached; //!< 是否有cache
	mutable typename DataReaderTraits<ITEM>::ID m_cachedID; //!< 已经cache的ID
	mutable ITEM *m_cachedItem; //!< 已经cache的ITEM
	// 08/14/2009-13:04 by YYBear [END] 	

	template < class BASE_ITEM>
	class BaseReader
		:public DataReader<BASE_ITEM>
	{
	public:
		inline bool loadFile(const char* file){ return load(file); };
	};

	template < class BASE_ITEM>
	bool extend(
		const char *file,	//< 保存数据的文件名
		DataReaderTraits<BASE_ITEM> dummy
		);
};

template < class ITEM > DataReader<ITEM>::DataReader()
	:m_loaded(false), m_cached(false)
{
}

template < class ITEM > DataReader<ITEM>::DataReader(const char *file)
{
	if(!load(file))
	{
		log_f("Error:Can't load file %s",file);
		exit(0);
	}		
}

template < class ITEM > DataReader<ITEM>::~DataReader()
{	
	clearData();
}

template < class ITEM > const typename DataReader<ITEM>::SET & DataReader<ITEM>::getUnSafeData() const
{
	return m_data;
}

template < class ITEM > void DataReader<ITEM>::getData(std::vector<ITEM>& output) const
{
	output.clear();
	typename SET::const_iterator it = m_data.begin();
	for (; it != m_data.end(); ++it)
	{
		output.push_back(**it);
	}	
}

template < class ITEM > const ITEM * DataReader<ITEM>::getUnSafeItem(const typename DataReaderTraits<ITEM>::ID &id, bool useCache/* = true*/) const
{
	// 08/14/2009-13:09 by YYBear [BEGIN]
	// 使用cache
	if (m_cached && m_cachedItem)
	{
		if (!(id < m_cachedID) && !(m_cachedID < id))
		{
			return m_cachedItem;
		}
	}
	// 08/14/2009-13:09 by YYBear [END] 


	CMP_PTR(ITEM,pItem)
	pItem->id=id;
	typename SET::const_iterator it=m_data.find(pItem);
	if(it!=m_data.end())
	{
		// 08/14/2009-13:11 by YYBear [BEGIN]
		// 保存最近一次cache
		m_cached = true;
		m_cachedID = id;
		m_cachedItem = *it;
		// 08/14/2009-13:11 by YYBear [END] 

		return *it;
	}
	else
	{
		return NULL;
	}
}

template < class ITEM > DataImage<ITEM> DataReader<ITEM>::getItem(const typename DataReaderTraits<ITEM>::ID &id) const
{
	return DataImage<ITEM>(getUnSafeItem(id));
}

template < class ITEM > bool DataReader<ITEM>::load(const char *file)
{
	FILE *fp=fopen(file,"rb");
	if(!fp)
	{
		return false;
	}
	char pData[NETWORK_BUF_SIZE-NETWORK_MSG_HEADER];
	DWORD maxCount = 0;
	DWORD loadedCount = 0;
	DWORD dataCount = 0;
	DWORD dataNum;
	dataNum = static_cast<DWORD>( fread(&maxCount,1,sizeof(DWORD),fp) );
	if(dataNum)
	{
		DWORD nextDataSize = sizeof(pData);
		int remainDataSize = 0;
		Zoic::Message msg;
		msg.ID(0);
		while( dataNum=static_cast<DWORD>( fread(pData,1,nextDataSize,fp) ) )
		{
			msg.writeData(pData,dataNum);

			Zoic::MessageState state;
			msg.saveState(state);
			ITEM* pItem = new ITEM;
			try
			{
				msg>>*pItem;
				++loadedCount;
			}catch(Zoic::OutOfBoundException *e)
			{
				msg.loadState(state);
				delete pItem;
				delete e;
				remainDataSize = msg.getRemainingRData();
				nextDataSize = sizeof(pData) - remainDataSize;
				continue;
			}
			++dataCount;
			if( m_data.find(pItem) == m_data.end() )
			{
				m_data.insert(pItem);
			}
			else
			{
				delete pItem;
			}
			remainDataSize = msg.getRemainingRData();
			nextDataSize = sizeof(pData) - remainDataSize;
			if(remainDataSize)
			{
				msg.readData(pData,remainDataSize);
				msg.ID(0).writeData(pData,remainDataSize);
			}
			else
			{
				msg.ID(0);
			}
		}
		fclose(fp);
		while( remainDataSize )
		{
			ITEM* pItem = new ITEM;
			try
			{
				msg>>*pItem;
				++loadedCount;
			}catch(Zoic::OutOfBoundException *e)
			{
				log_f("DataReader Load\"%s\" Failed(%d/%d):%s",file,loadedCount,maxCount,e->what());
				delete pItem;
				delete e;
				return false;
			}
			++dataCount;
			if( m_data.find(pItem) == m_data.end() )
			{
				m_data.insert(pItem);
			}
			else
			{
				delete pItem;
			}
			remainDataSize = msg.getRemainingRData();
			if(remainDataSize)
			{
				msg.readData(pData,remainDataSize);
				msg.ID(0).writeData(pData,remainDataSize);
			}
			else
			{
				msg.ID(0);
			}
		}
		if(maxCount != loadedCount)
		{
			log_f("DataReader Load\"%s\" Failed(%d/%d):unexpected file end",file,loadedCount,maxCount);
			return false;
		}
	}
	m_loaded = true;
	m_cached = false; // 08/14/2009-13:13 by YYBear 	
	return true;
}

template < class ITEM >
template <class BASE_ITEM >
bool DataReader<ITEM>::extend(const char *file)
{
	BaseReader<BASE_ITEM> reader;
	if( !reader.loadFile(file) )
	{
		return false;
	}
	const typename DataReader<BASE_ITEM>::SET & data = reader.getUnSafeData();
	for(typename DataReader<BASE_ITEM>::SET::const_iterator it = data.begin();it!=data.end();++ it)
	{
		try
		{
			m_data.insert(new ITEM(**it));
		}
		catch(std::exception * e)
		{
			delete e;
		}
	}	
	return true;
}

template < class ITEM >
template <class BASE_ITEM >
bool DataReader<ITEM>::extend(const char *file, DataReaderTraits<BASE_ITEM> dummy)
{
	return extend<BASE_ITEM>(file);
}

template < class ITEM > void DataReader<ITEM>::clearData()
{
	for(typename SET::iterator it=m_data.begin();it!=m_data.end();++it)
	{
		delete *it;
	}
	m_data.clear();
	m_loaded = false;
	m_cached = false;
}

#endif
