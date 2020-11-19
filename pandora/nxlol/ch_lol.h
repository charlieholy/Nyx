#pragma once
#include "ch_leveldb.h"
#include "ch_json.h"
#include <mutex>
#include <unistd.h>
#include "ch_log.h"
#include <thread>
#include <iostream>  
#include <fstream>  
#include <sstream>
#include <map>
#include "ch_fileoper.h"

class DB_JSON{
public:
	DB_JSON(std::string key):_key(key){}
	void init_fromdb(){
		std::string s_value;
		s_value = g_file_oper.get(_key);
		JsonFromStr(s_value,_m_json);
		//L_INFO("init_fromdb key  {} s_value {} json  {} ",_key, s_value,Json2Str(_m_json));
		LOG_INFOS(LOG_DB,"init_fromdb key  {} s_value {} json  {} ",_key, s_value,Json2Str(_m_json));
	}
	void put_todb(){
		std::string s_json= Json2Str(_m_json);
	    g_file_oper.put(_key,s_json);
		LOG_INFOS(LOG_DB,"put_todb key {} value {}",_key,s_json);
	}
protected:
	Json::Value _m_json;
	std::string _key;
};

//用于存储币种对BTC的汇率
#define MAC_BTC_RATE "g_last_btc_rate"
class BTC_RATE : public  DB_JSON{
public:
	BTC_RATE(std::string key):DB_JSON(key){}
	Json::Value btc_price_frommem() {    //反序列化json
		return _m_json;
	}
	void  put_btc_rate_tomem(std::string symbol,double price){
		if( price <= 1e-8 ){
			return;
		}
		int t_size = symbol.size();
		if(t_size < 4)
		{
			return;
		}
		std::string market_sym = symbol.substr(t_size - 3, t_size);
		if ("BTC" == market_sym) {
			std::string tsym = symbol.substr(0, t_size - 4);
			__put_btc_rate_tomem(tsym,price);
		}
	}
private:
	void __put_btc_rate_tomem(std::string symbol, double price) {
		std::lock_guard<std::mutex> guard(__m_mutex);
		_m_json[symbol] = price;
	}
	std::mutex __m_mutex;
};


#define MAC_ALL_SYM "g_last_all_sym"  //记录所有收到过的币种
class ALL_SYM : public DB_JSON{
public:
	ALL_SYM(std::string key):DB_JSON(key){}
	void insertOne(std::string sym){
		std::lock_guard<std::mutex> guard(__m_mutex);
		_m_json[sym] = sym;
	}
	Json::Value getAllsym(){
		return _m_json;
	}
private:
	std::mutex __m_mutex;
};


class ONE_JSON : public DB_JSON{
public:
	ONE_JSON(std::string key):DB_JSON(key){}
	Json::Value getJson(std::string sym){
		std::lock_guard<std::mutex> guard(__m_mutex);
		return JsonVal(_m_json,sym);
	}
	void setJson(std::string sym,Json::Value value){
		std::lock_guard<std::mutex> guard(__m_mutex);
		//L_INFO("setdata {}",Json2Str(value));
		_m_json[sym] = value;
	}
private:
	std::mutex __m_mutex;
};




////获取小时成交量 小时开盘价
extern void db_addDayDate(Json::Value &data,std::string);
class DAY_TICK{
public:
	void addDayDate(std::string sym,Json::Value &data){
		Json::Value item = __getDayTick(sym);
		//"day_high":"","day_low":"","day_open":"","day_ts":"","day_volume":""
		data["day_high"] = JsonDouble(item,"day_high");
		data["day_low"] = JsonDouble(item,"day_low");
		data["day_open"] = JsonDouble(item,"day_open");
		data["day_volume"] = (JsonDouble(item,"day_volume") + getDayVolume(sym));
		data["day_ts"] = JsonInt(item,"day_ts");
	}
	double getDayVolume(std::string sym){
		return __m_volume[sym];
	}

	void setDayVolume(std::string sym,double amount){
		__m_volume[sym] += amount;
	}
	void reSet(){
		//L_INFO("clear");
		__m_isupdate.clear();
		__m_volume.clear();
	}
private:
	Json::Value __getDayTick(std::string symbol){
		{
			std::lock_guard<std::mutex> guard(__m_mutex);
			if(2  == __m_isupdate[symbol] ){
				return JsonVal(__m_json,symbol);
			}
			//L_INFO("getDayTick");
			__m_isupdate[symbol] = 2;
		}
		db_addDayDate(__m_json,symbol);
		return JsonVal(__m_json,symbol);
	}
	std::map<std::string,int> __m_isupdate;
	Json::Value __m_json;
	std::map<std::string,double> __m_volume;
	std::mutex __m_mutex;
};
//g_day_tick.setDayVolume();


class CLol {
public :
    CLol();
    virtual ~CLol();
    void run();
    bool init();
	Json::Value getJsonPtr(std::string tag,std::string sym){
		Json::Value data;
		auto it = __m_map_forever.find(tag);
		if (it != __m_map_forever.end()){
			return __m_map_forever[tag]->getJson(sym);
		}
		return data;
	}
	void setJsonPtr(std::string tag,std::string sym,Json::Value jOne){
		auto it = __m_map_forever.find(tag);
		if (it != __m_map_forever.end()){
			//L_INFO("__m_map_forever {} sym {} json {}",tag,sym,Json2Str(jOne));
			__m_map_forever[tag]->setJson(sym,jOne);
		}
		
	}
private:
	std::mutex __m_mutex;
	std::map<std::string,std::shared_ptr<ONE_JSON>> __m_map_forever;
	std::shared_ptr<std::thread> __m_thread;
};


extern DAY_TICK g_day_tick;
extern BTC_RATE  g_btc_rate;//缓存币种相对于比特币的价格
extern ALL_SYM g_all_sym;//缓存记录所有收到过的币种
extern CLol g_lol;