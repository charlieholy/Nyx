#pragma once
#include "ch_json.h"
#include "ch_leveldb.h"
#include "activeObject.h"
#include <list>
#include <thread>
#include <condition_variable>
#include "ch_kline.h"
#include "CppSQLite3.h"
#include <mutex>
#include "ch_tools.h"
#include <set>
#include <queue>
#include "ch_log.h"
#include "ch_queue.h"
#include "ch_proto.h"
#include "ch_lol.h"

//缓存20个历史成交
class TK_VEC {
public:
	Json::Value getAllSym(std::string sym) {
		Json::Value j_tick20;
		j_tick20.resize(0);
		std::vector<Json::Value> que;
		{
			std::lock_guard<std::mutex> guard(__m_mutex);
			que = __m_tk_vec[sym];
		}
		for (auto it : que) {
			j_tick20.append(it);
		}
		return j_tick20;
	}
	void addOnetick(std::string sym, Json::Value j_tick) {
		std::vector<Json::Value> vec;
		{
			std::lock_guard<std::mutex> guard(__m_mutex);
			vec = __m_tk_vec[sym];
		}
		vec.emplace_back(j_tick);
		if (vec.size() > 20)vec.erase(vec.begin());
		{
			std::lock_guard<std::mutex> guard(__m_mutex);
			__m_tk_vec[sym] = vec;
		}
	}
private:
	std::map<std::string, std::vector<Json::Value>> __m_tk_vec;
	std::mutex __m_mutex;
};

extern TK_VEC g_tk_vec;  //用于存储20个历史tick数据

static std::map<std::string,int> binance2nyxperiodmap;

typedef struct _KL_DATA{
    double darr[5] = {0}; //open close high low volume
} KL_DATA;

typedef std::map<std::string, KL_DATA> ONESYM_DATA;

//用于缓存计算1分钟k线
class KL_CALC_MAP_ONE_M {
public:
	ONESYM_DATA getTsAll() {  //获取该时间戳的所有1m数据
		std::lock_guard<std::mutex> guard(__m_kl_mutex);
		ONESYM_DATA kl_all_data = __m_kl_d;
		__m_kl_d_bak = __m_kl_d;
		__m_kl_d.clear();
		__m_isMergeing = true;
		return kl_all_data;
	}

	void resetMergeState(){
		__m_isMergeing = false;
	}
	//合入分钟线的时候备份数据
	//void clear(){
	//	std::lock_guard<std::mutex> guard(__m_kl_mutex);
	//	__m_kl_d.clear();
	//}
	
	KL_DATA getTsSym(std::string sym) {   //获取该时间戳该品种的1m数据
		std::lock_guard<std::mutex> guard(__m_kl_mutex);
		if(__m_isMergeing){
			return __m_kl_d_bak[sym];
		}
		return __m_kl_d[sym];
	}

	void insertCalcTsSymbol(std::string symbol, double price, double amount) {  //计算该时间戳该品种的1m数据
		KL_DATA oneK = getTsSym(symbol);
		double k_amunt = oneK.darr[4];
		if (k_amunt == 0.0) { oneK.darr[0] = price; }
		oneK.darr[1] = price;
		double high = oneK.darr[2];
		if (high < price) { oneK.darr[2] = price; }
		double low = oneK.darr[3];
		if (low > price) { oneK.darr[3] = price; }
		else if (low == 0.0) { oneK.darr[3] = price; }
		oneK.darr[4] += amount;
		__insertOnek(symbol, oneK);
	}
	void printKD(KL_DATA &data) {
		L_INFO("open {} close {} high {} low {} v {}", data.darr[0], data.darr[1], data.darr[2], data.darr[3], data.darr[4]);
	}
private:
	void __insertOnek(std::string sym, KL_DATA oneK) {  //更新该时间戳该品种的1m数据
		std::lock_guard<std::mutex> guard(__m_kl_mutex);
		__m_kl_d[sym] = oneK;
	}

	ONESYM_DATA __m_kl_d;
	ONESYM_DATA __m_kl_d_bak; //合入数据时备份数据
	bool __m_isMergeing = false;
	std::mutex __m_kl_mutex;
};

extern KL_CALC_MAP_ONE_M g_kl_calc_m;  //用于缓存计算1分钟k线


class CKline : public ActiveObject{
public :
    CKline();
    virtual ~CKline();
    int run();
    bool init();
	static Json::Value getDayTickformdb(std::string symbol);
    static Json::Value getJsonArrfromdb(int period,std::string symbol,int num); //获取某个币种的k线数据
    static void ex_insert2db(Json::Value j_data); ///对内提供补线接口

private:
    int __getMonth(int t);
    int __getWeek(int t_ts);
    int __getLastDayTs();
    std::string __create_table(int period);  //创建表
    bool __open_table();  //打开表
    KL_DATA __getOnefromdb(int period,int ts,std::string symbol); //获取待更新的蜡烛
	KL_DATA __getLastOnefromdb(int period,std::string symbol); //获取最新的蜡烛用于画close
    void __insert2db(int period,int ts,std::string symbol,KL_DATA);  //插入k线数据
	static void CKline::__lastmerge(KL_DATA mem,KL_DATA& db); ////缓存的数据跟db聚合 获取最新一根k线
    static void __update(KL_DATA from,KL_DATA& to); //更新k线数据
	void __updateClose(KL_DATA from, KL_DATA& to); //复制上一根k线数据的close
    static KL_DATA __json2kl(Json::Value);  //json转kl
    int __period2ts(int ts,int period);    //当前时间戳对应的k线篮子  
    bool __isnew_ts1m(int& newts1m); //判断是否产生新的分钟
	void __delete_kline_data(int period,int ts);  //删除一天之前的1分钟线
	int __m_last_time_1m = 0;
};







