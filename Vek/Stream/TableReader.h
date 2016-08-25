#pragma once

#include <vector>
#include <set>
#include <utility>
#include "Stream.h"
#include "../Base/Exception.h"


//数据读取模板
template < class ITEM > class TableReader
{
public:
	typedef std::set<ITEM *, typename ITEM::CmpID> SET;		//!< 条目集合


	~TableReader();

	//!获得安全数据(副本)
	void getData(std::vector<ITEM>& output) const;

	//!获得数据
	const SET & getData() const;

	//!根据ID获得条目
	const ITEM * getItem(	const typename ITEM::IDType &id ) const;	

protected:
	//!构造函数
	TableReader( const char *file );
	TableReader();	

	/*! @brief 通过条目基类扩展数据
		
		主要用在派生类的构造函数中
	*/
	template < class BASE_ITEM>
	bool extend( const char *file );
	SET 		m_data;
	bool		m_loaded;
	bool load(const char *file);

	void clearData();	

	template < class BASE_ITEM>
	class BaseReader: public TableReader<BASE_ITEM>
	{
	public:
		inline bool loadFile(const char* file){ return load(file); };
	};
};

template < class ITEM > TableReader<ITEM>::TableReader()
	:m_loaded(false)
{
}

template < class ITEM > TableReader<ITEM>::TableReader(const char *file)
{
	if(!load(file))
	{
		log_f("Error:Can't load file %s",file);
		exit(0);
	}		
}

template < class ITEM > TableReader<ITEM>::~TableReader()
{	
	clearData();
}

template < class ITEM > const typename TableReader<ITEM>::SET & TableReader<ITEM>::getData() const
{
	return m_data;
}

template < class ITEM > void TableReader<ITEM>::getData(std::vector<ITEM>& output) const
{
	output.clear();
	typename SET::const_iterator it = m_data.begin();
	for (; it != m_data.end(); ++it)
	{
		output.push_back(**it);
	}	
}

template < class ITEM > const ITEM * TableReader<ITEM>::getItem(const typename ITEM::IDType &id) const
{
	ITEM  cmpItem;
	cmpItem.SetID(id);
	typename SET::const_iterator it=m_data.find(&cmpItem);
	if(it!=m_data.end())
	{
		return *it;
	}
	else
	{
		return NULL;
	}
}

template < class ITEM > bool TableReader<ITEM>::load(const char *file)
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
		Vek::Stream stream;
		stream.ID(0);
		while( dataNum=static_cast<DWORD>( fread(pData,1,nextDataSize,fp) ) )
		{
			stream.writeData(pData,dataNum);

			Vek::StreamState state;
			stream.saveState(state);
			ITEM* pItem = new ITEM;
			try
			{
				stream>>*pItem;
				++loadedCount;
			}catch(Vek::Exception *e)
			{
				stream.loadState(state);
				delete pItem;
				delete e;
				remainDataSize = stream.getRemainingRData();
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
			remainDataSize = stream.getRemainingRData();
			nextDataSize = sizeof(pData) - remainDataSize;
			if(remainDataSize)
			{
				stream.readData(pData,remainDataSize);
				stream.ID(0).writeData(pData,remainDataSize);
			}
			else
			{
				stream.ID(0);
			}
		}
		fclose(fp);
		while( remainDataSize )
		{
			ITEM* pItem = new ITEM;
			try
			{
				stream>>*pItem;
				++loadedCount;
			}catch(Vek::Exception *e)
			{
				//OutputDebugString("DataReader Load\"%s\" Failed(%d/%d):%s",file,loadedCount,maxCount,e->what());
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
			remainDataSize = stream.getRemainingRData();
			if(remainDataSize)
			{
				stream.readData(pData,remainDataSize);
				stream.ID(0).writeData(pData,remainDataSize);
			}
			else
			{
				stream.ID(0);
			}
		}
		if(maxCount != loadedCount)
		{
			//OutputDebugString("DataReader Load\"%s\" Failed(%d/%d):unexpected file end",file,loadedCount,maxCount);
			return false;
		}
	}
	m_loaded = true;
	return true;
}

template < class ITEM >
template <class BASE_ITEM >
bool TableReader<ITEM>::extend(const char *file)
{
	BaseReader<BASE_ITEM> reader;
	if( !reader.loadFile(file) )
	{
		return false;
	}
	const typename TableReader<BASE_ITEM>::SET & data = reader.getUnSafeData();
	for(typename TableReader<BASE_ITEM>::SET::const_iterator it = data.begin();it!=data.end();++ it)
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

template < class ITEM > void TableReader<ITEM>::clearData()
{
	for(typename SET::iterator it=m_data.begin();it!=m_data.end();++it)
	{
		delete *it;
	}
	m_data.clear();
	m_loaded = false;
}

