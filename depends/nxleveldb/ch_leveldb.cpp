#include "ch_leveldb.h"

CLeveldb::CLeveldb()
{
}


CLeveldb::~CLeveldb()
{
}

bool CLeveldb::init()
{
	unInit();
	leveldb::Options options;
	options.create_if_missing = true;
	std::string dbpath = "leveldb";
	leveldb::Status status = leveldb::DB::Open(options, dbpath, &__m_db);
	return status.ok();
}

void CLeveldb::unInit()
{
	if (__m_db)
	{
		delete __m_db;
	}
}
#include <iostream>
bool CLeveldb::put(std::string key, std::string val)
{
	if (!__m_db)
	{
		return false;
	}
	leveldb::Status status;
	{
		std::lock_guard<std::mutex> guard(__m_mtx);
        // write_options_.sync = true;
		status = __m_db->Put(leveldb::WriteOptions(), key, val);
	}
	return status.ok();
}

bool CLeveldb::get(std::string key, std::string& val)
{
	if (!__m_db)
	{
		return false;
	}
	leveldb::Status status;
	{
		status = __m_db->Get(leveldb::ReadOptions(), key, &val);
	}
	return status.ok();
}

bool CLeveldb::del(std::string key)
{
	if (!__m_db)
	{
		return false;
	}
	leveldb::Status status;
	{
		std::lock_guard<std::mutex> guard(__m_mtx);
		status = __m_db->Delete(leveldb::WriteOptions(), key);
	}
	return status.ok();
}


CLeveldb g_leveldb;                 //leveldb操作句柄 


