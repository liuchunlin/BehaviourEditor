/*! @file
	@brief �������ݶ�ȡģ��
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

/*! @brief ���ݶ�ȡ����ģ��
	@param ITEM Ԫ������
*/
template < class ITEM > struct DataReaderTraits
{
	typedef DWORD ID;	//!< Ԫ��ID����,Ĭ��ΪDWORD
};

/*! @brief ���ݶ�ȡģ��
	@param ITEM Ԫ������
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
	typedef std::set<ITEM *,CmpID> SET;		//!< ��Ŀ����


	~DataReader();

	//!��ð�ȫ����(����)
	void getData(std::vector<ITEM>& output) const;

	//!�������
	const SET & getUnSafeData() const;

	//!����ID��ð�ȫ��Ŀ
	DataImage<ITEM> getItem(
		const typename DataReaderTraits<ITEM>::ID &id	//!< ��ĿID
		) const;

	//!����ID�����Ŀ
	const ITEM * getUnSafeItem(
		const typename DataReaderTraits<ITEM>::ID &id,	//!< ��ĿID
		bool useCache = true //!< ʹ��cache
		) const;	

protected:
	//!���캯��
	DataReader(
		const char *file	//!< �������ݵ��ļ���
		);
	DataReader();	

	/*! @brief ͨ����Ŀ������չ����
		
		��Ҫ����������Ĺ��캯����
	*/
	template < class BASE_ITEM>
	bool extend(
		const char *file	//< �������ݵ��ļ���
		);
	SET 		m_data;		//!< ��Ŀ����
	bool		m_loaded;
	bool load(const char *file);

	void clearData();	

	// 08/14/2009-13:04 by YYBear [BEGIN]
	// ���һ��cache
	mutable bool m_cached; //!< �Ƿ���cache
	mutable typename DataReaderTraits<ITEM>::ID m_cachedID; //!< �Ѿ�cache��ID
	mutable ITEM *m_cachedItem; //!< �Ѿ�cache��ITEM
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
		const char *file,	//< �������ݵ��ļ���
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
	// ʹ��cache
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
		// �������һ��cache
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
