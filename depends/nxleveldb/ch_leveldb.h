#pragma once
#include <string>
#include "leveldb/db.h"
#include <map>
#include <vector>
#include <mutex>


class CLeveldb
{
public:
	CLeveldb();
	virtual ~CLeveldb();
public:
	bool init();
	void unInit();
public:
	bool put(std::string key, std::string val);
	bool get(std::string key,std::string& val);
	bool del(std::string key);

private:
	leveldb::DB* __m_db;
	std::mutex __m_mtx;
};


extern CLeveldb g_leveldb;                 //leveldb操作句柄 