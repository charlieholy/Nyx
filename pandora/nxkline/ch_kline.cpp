#include "ch_kline.h"
#include "ch_log.h"
#include <unistd.h> 
#include "ch_tools.h"
#include <iostream>
#include <map>

////1分 5分 15分 30分 1小时 4小时 日线 周线 月线
//week 604800   magic 259200 = 3day magic 345600 = 4day magic
int karr_len = 9;
int karrs[9] = {1,5,15,30,60,4*60,24*60,7*24*60,43200};
char* binance_interval[9] = {"1m","5m", "15m", "30m" ,"1h", "4h", "1d", "1w", "1M"};  //解析币安补线参数
char* jkey[5] = {"open","close","high","low","volume"};

TK_VEC g_tk_vec; //存储tick_vec
KL_CALC_MAP_ONE_M g_kl_calc_m; //存储计算分钟k

int CKline::__getWeek(int ts){
	int ts_week4 = ts / 604800 * 604800;
	if (ts >= (ts_week4 + 345600))
	{
		ts = ts_week4 + 345600;
	}
	else {
		ts = ts_week4 - 259200;
	}
	return ts;
}

int CKline::__getMonth(int ts)
{   
    time_t t = ts;
    struct tm *p;
    p=gmtime(&t);
    char s[15];
    strftime(s, sizeof(s), "%Y%m%d%H%M%S", p);
    memcpy(s+6,"01000000",8);
    struct tm tmp_time;
    strptime(s,"%Y%m%d%H%M%S",&tmp_time);
    return mktime(&tmp_time);
}

CKline::CKline():ActiveObject("klinemerge"){}
CKline::~CKline(){
    g_sqlite3_kxm.close();
}
bool CKline::init(){
    //初始化k线表
    if(!__open_table())
    {
        return false;
    }
    for(int i=0;i<karr_len;i++){
        binance2nyxperiodmap[binance_interval[i]] = karrs[i];
    }
    return true;
}

void Debug_kdata(KL_DATA kd){
    L_INFO("kd {} {} {} {} {}",kd.darr[0],kd.darr[1],kd.darr[2],kd.darr[3],kd.darr[4]);
}

static Json::Value CKline::getDayTickformdb(std::string symbol){
    Json::Value j_arr;
    j_arr.resize(0);
	
    std::string sql = string_format("select * from kline1440m where symbol = '%s' order by ts desc limit %d",
    symbol.c_str(),1);

        //L_INFO("sql {}",sql);
    CppSQLite3Table t;
    try{
        t = g_sqlite3_kxm.getTable(sql.c_str());
    }
	catch(CppSQLite3Exception& e)
	{
		//LOG_INFOS(LOG_DAYTICK,"{},{}\n", __func__, e.errorMessage());
        return j_arr;
	}
    for (int row = 0; row < t.numRows(); row++)
    {
        t.setRow(row);
        Json::Value item;
        for (int fld = 2; fld < t.numFields(); fld++)
        {
            std::string value = t.fieldValue(fld);
            if(fld == t.numFields()-1)
            {
                item.append(atoi(value.c_str()));  //ts
            }
            else 
            {
                item.append(atof(value.c_str()));  //open close high low volume
            }
        }
        j_arr.append(item);
    }
}

//获取某个币种的k线数据
Json::Value CKline::getJsonArrfromdb(int period,std::string symbol,int num){
    Json::Value j_arr;
    j_arr.resize(0);
	
    std::string sql = string_format("select * from kline%dm where symbol = '%s' order by ts desc limit %d",
    period,symbol.c_str(),num);
    
    //L_INFO("sql {}",sql);
    CppSQLite3Table t;
    try{
        t = g_sqlite3_kxm.getTable(sql.c_str());
    }
	catch(CppSQLite3Exception& e)
	{
		LOG_INFOS(LOG_KLINE,"{},{}\n", __func__, e.errorMessage());
        return j_arr;
	}
    for (int row = 0; row < t.numRows(); row++)
    {
        t.setRow(row);
        Json::Value item;
        for (int fld = 2; fld < t.numFields(); fld++)
        {
            std::string value = t.fieldValue(fld);
            if(fld == t.numFields()-1)
            {
                item.append(atoi(value.c_str()));  //ts
            }
            else 
            {
                item.append(atof(value.c_str()));  //open close high low volume
            }
        }
        j_arr.append(item);
    }

    KL_DATA k_data = g_kl_calc_m.getTsSym(symbol);  //拿缓存的k线数据过来聚合
    
    Json::Value item = JsonPos(j_arr, 0); 
    int  k_one_ts = JsonIntEx(JsonPos(item,5));

    //L_INFO("when begin get kline k_old k_data k_old ==================");
    KL_DATA k_old = __json2kl(item);  //json转kl  //数据库中的最新一根
    __lastmerge(k_data, k_old);
    //L_INFO("when end get kline ==================");
    Json::Value j_item;
    j_item.append(k_old.darr[0]);
    j_item.append(k_old.darr[1]);
    j_item.append(k_old.darr[2]);
    j_item.append(k_old.darr[3]);
    j_item.append(k_old.darr[4]);
    j_item.append(k_one_ts);
    j_arr[0] = j_item;
}

//OPEN CLOSE HIGH LOW VOLUME TIMESTAMP
std::string CKline::__create_table(int period){
    std::string s_sql = string_format("create table if not exists kline%dm( \
    id integer PRIMARY KEY autoincrement,\
    symbol varchar(20),\
    open double,\
    close double,\
    high double,\
    low double, \
    volume double,\
    ts int, \
    unique (symbol,ts) \
    );" \
    ,period);
    return s_sql;
}

void CKline::ex_insert2db(Json::Value j_data){
    std::string insertDml = string_format("replace into kline%dm(\
        symbol,\
        open,\
        close,\
        high,\
        low,\
        volume, \
        ts \
        ) values(\
			'%s','%s','%s','%s','%s','%s',%d \
		)", binance2nyxperiodmap[JsonStr(j_data,"interval")],
			JsonStr(j_data,"symbol").c_str(),
			JsonStr(j_data,"open").c_str(),
			JsonStr(j_data,"close").c_str(),
			JsonStr(j_data,"high").c_str(),
            JsonStr(j_data,"low").c_str(),
            JsonStr(j_data,"volume").c_str(),
            JsonInt64(j_data,"ts")/1000);
    try {
        LOG_INFOS(LOG_KLINE,"ex_Insert {}",insertDml);
        g_sqlite3_kxm.execDML(insertDml.c_str());
    }
    catch (CppSQLite3Exception& e)
    {
        LOG_INFOS(LOG_KLINE,"{},{},{}", insertDml.c_str(),__func__,e.errorMessage());
    }
}

void CKline::__delete_kline_data(int period,int ts){
     std::string deleteDml = string_format("delete from kline%dm \
        where ts < %d",period,ts);
    try {
        g_sqlite3_kxm.execDML(deleteDml.c_str());
    }
    catch (CppSQLite3Exception& e)
    {
        LOG_INFOS(LOG_KLINE,"{},{}",__func__,e.errorMessage());
    }
    LOG_INFOS(LOG_KLINE,"{}",deleteDml);
}

void CKline::__insert2db(int period,int ts,std::string symbol,KL_DATA k_d){
    std::string insertDml = string_format("replace into kline%dm(\
        symbol,\
        open,\
        close,\
        high,\
        low,\
        volume, \
        ts \
        ) values(\
			'%s',%f,%f,%f,%f,%f,%d \
		)", period,
			symbol.c_str(),
			k_d.darr[0],
			k_d.darr[1],
			k_d.darr[2],
            k_d.darr[3],
            k_d.darr[4],
            ts);
    //L_INFO("__insert2db {}",insertDml);
    try {
        g_sqlite3_kxm.execDML(insertDml.c_str());
    }
    catch (CppSQLite3Exception& e)
    {
        LOG_INFOS(LOG_KLINE,"{},{}",__func__,e.errorMessage());
    }
}

bool CKline::__open_table(){
    try{
        std::string table_name = "sqlite";
        if(!ch_mkdir(table_name))
        {
            LOG_ALARMS(LOG_KLINE);
            return false;
        }
		std::string tickDayTable = "klinedb";
		std::string pathdb = table_name + "/" +tickDayTable;
		g_sqlite3_kxm.open(pathdb.c_str());
        for(int i=0;i<karr_len;i++)
        {
            g_sqlite3_kxm.execDML(__create_table(karrs[i]).c_str());
        }
	}
	catch(CppSQLite3Exception& e)
	{
        LOG_INFOS(LOG_KLINE,"{},{}", __func__, e.errorMessage());
        return false;
	}
    return true;
}

void CKline::__lastmerge(KL_DATA mem,KL_DATA& db){  //缓存的数据跟db聚合 获取最新一根k线
    if(mem.darr[4] == 0){  //mem没有数据放弃聚合
        return;
    }
    if(db.darr[4] == 0){  //db没有量 复制open价
        db.darr[0] = mem.darr[0];
    }
    db.darr[1] = mem.darr[1];  ////复制 close
    if(db.darr[2] < mem.darr[2])   ////更新最高价
    {
        db.darr[2] = mem.darr[2];
    }
    if(db.darr[3] > mem.darr[3])   ////更新最低价
    {
        db.darr[3] = mem.darr[3];
    }
    db.darr[4] += mem.darr[4];  ////更新 volume
}

void CKline::__update(KL_DATA from,KL_DATA& to){
    //open close high low volume
   if(to.darr[4] == 0) ////如果volume是0  复制整根蜡烛
   {
       for(int i=0;i<5;i++){
            to.darr[i] = from.darr[i];
       }
       return ;
   }
   to.darr[1] = from.darr[1];  ////复制 close
   if(to.darr[2] < from.darr[2])   ////更新最高价
   {
       to.darr[2] = from.darr[2];
   }
   if(to.darr[3] > from.darr[3] || to.darr[3] == 0)   ////更新最低价
   {
       to.darr[3] = from.darr[3];
   }
   to.darr[4] += from.darr[4];  ////更新 volume
}

void CKline::__updateClose(KL_DATA from, KL_DATA& to) {
	for (int i = 0; i < 4; i++) {
		to.darr[i] = from.darr[1]; //拷贝close
	}
	to.darr[4] = 0;  //volume = 0
}

////获取一根待更新蜡烛
KL_DATA CKline::__getOnefromdb(int period,int ts,std::string symbol){
    KL_DATA k_it;
    std::string sql = string_format("select * from kline%dm \
    where symbol = '%s' and ts = %d order by ts desc limit 1",
    period,symbol.c_str(),ts);
    CppSQLite3Table t;
    try
    {
        t = g_sqlite3_kxm.getTable(sql.c_str());
        if(t.numFields() == 0)
        {
            return k_it;
        }
        for(int i=0;i<5;i++)
        {
            k_it.darr[i] = atof(t.fieldValue(2+i));
        }
    }
	catch(CppSQLite3Exception& e)
	{
        LOG_INFOS(LOG_KLINE,"{},{},{}",__func__,sql.c_str(), e.errorMessage());
    }
    return k_it;
}

////获取最新的蜡烛用于画close
KL_DATA CKline::__getLastOnefromdb(int period,std::string symbol){
    KL_DATA k_it;
    std::string sql = string_format("select * from kline%dm \
    where symbol = '%s'  order by ts desc limit 1",
    period,symbol.c_str());
    CppSQLite3Table t;
    try
    {
        t = g_sqlite3_kxm.getTable(sql.c_str());
        if(t.numFields() == 0)
        {
            return k_it;
        }
        for(int i=0;i<5;i++)
        {
            k_it.darr[i] = atof(t.fieldValue(2+i));
        }
    }
	catch(CppSQLite3Exception& e)
	{
        LOG_INFOS(LOG_KLINE,"{},{},{}",__func__,sql.c_str(), e.errorMessage());
    }
    return k_it;
}

KL_DATA CKline::__json2kl(Json::Value j_d){
    KL_DATA k_d;
    for(int i=0;i<5;i++)
    {
        k_d.darr[i] = JsonDoubleEx(JsonPos(j_d,i));
    }
    return k_d;
}


int CKline::run(){
	usleep(1000000);
	int new_1mts;
	if (!__isnew_ts1m(new_1mts)) {   //有新的分钟线产生
		return ;
	}
    int last_1mts = (new_1mts - 60);
    //L_INFO("new 1m {} last 1m {} ",new_1mts,last_1mts);
	//合入上一分钟线的数据
	ONESYM_DATA last1m_data = g_kl_calc_m.getTsAll();
	for (auto & it : last1m_data) {
		std::string symbol = it.first;
		KL_DATA k_new = (KL_DATA)it.second;
       // L_INFO("when  new 1m ==========================",symbol);
        if(k_new.darr[4] == 0){
            //L_INFO("=========================>nani");
            continue;
        }

		////存储分钟线
		__insert2db(1, last_1mts, symbol, k_new);
        ///更新币种对btc汇率
        g_btc_rate.put_btc_rate_tomem(symbol,k_new.darr[1]);
        g_all_sym.insertOne(symbol);
		//触发所有kline更新 根据当前symbol
		for (int i = 1; i<karr_len; i++)
		{
			int period = karrs[i];
			int ts_it = last_1mts / (period * 60) * (period * 60);
			if (period == 7 * 24 * 60)
			{
				ts_it = __getWeek(last_1mts);
			}
			else if (period == 43200)
			{
				ts_it = __getMonth(last_1mts);
			}
			//printf("period %d ts_it %d symbol %s\n",period,ts_it,symbol.c_str());
			KL_DATA k_it = __getOnefromdb(period, ts_it, symbol);
			__update(k_new, k_it);
			__insert2db(period, ts_it, symbol, k_it);
		}
	}
    g_kl_calc_m.resetMergeState();
    g_day_tick.reSet();
	//触发没有tick数据的画线 用上一个tick的close价格填充 volume=0
	for (int i = 0; i<karr_len; i++) {  //依次判断每个period是否触发新的时间戳
		int period = karrs[i];
		int new_period_ts = __period2ts(new_1mts, period);
        int last_period_ts = __period2ts(last_1mts, period);
		if (new_period_ts > last_period_ts) {
            if( period == 1440){
                LOG_INFOS(LOG_KLINE,"merge del newts {} lastts {} ",new_period_ts,last_period_ts);
                __delete_kline_data(1,last_period_ts); //
            }
			//L_INFO("copy close s_period_ts {} last_ts{} in {}", s_period_ts, s_last_period_ts, s_period);
			//将上一根蜡烛的close价拷贝到新的蜡烛 更新所有symbol
			Json::Value allsym = g_all_sym.getAllsym();
			for (auto it : allsym) {
				std::string symOne = JsonStrEx(it);
                //L_INFO("when keep close =====================");
				KL_DATA k_it = __getLastOnefromdb(period,symOne);
				KL_DATA k_close;
				__updateClose(k_it, k_close);
                //L_INFO("when end keep =====================");
				__insert2db(period, new_period_ts, symOne, k_close);
			}
		}
		else {
			//L_INFO("nothing s_period_ts {} last_ts{} in {}", s_period_ts, s_last_period_ts, s_period);
		}
	}
}

int CKline::__period2ts(int ts,int period)   //period2ts  
{
    if(period == 7*24*60)
    {
        return __getWeek(ts);
    }
    else if(period == 43200)
    {
        return __getMonth(ts);
    }
    return ts /  (period*60) * (period*60);
}

 bool CKline::__isnew_ts1m(int& newts1m) //判断是否产生新的分钟
 {
    newts1m = ch_getts_1m();
    if(newts1m > __m_last_time_1m){
        __m_last_time_1m = newts1m;
        return true;
    }
	//LOG_INFOS(LOG_KLINE, "no now1m {} last1m {}", s_ts_now_1m, s_last_1m);
    return false;
 }

