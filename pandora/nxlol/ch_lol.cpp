#include "ch_lol.h"
#include "ch_kline.h"


DAY_TICK g_day_tick;
BTC_RATE  g_btc_rate(MAC_BTC_RATE);//缓存币种相对于比特币的价格
ALL_SYM g_all_sym(MAC_ALL_SYM);//缓存记录所有收到过的币种


void db_addDayDate(Json::Value &data,std::string symbol)
{
    Json::Value j_karr = CKline::getDayTickformdb(symbol);
    int day_ts = JsonIntEx(JsonPos(JsonPos(j_karr,0),5));
    double day_volume = JsonDoubleEx(JsonPos(JsonPos(j_karr,0),4));
    double day_open = JsonDoubleEx(JsonPos(JsonPos(j_karr,0),0));
	double day_high = JsonDoubleEx(JsonPos(JsonPos(j_karr, 0), 2));
	double day_low = JsonDoubleEx(JsonPos(JsonPos(j_karr, 0), 3));
    
    Json::Value sym_data;
    sym_data["day_volume"] = day_volume;
	sym_data["day_open"] = day_open;// == 0 ? open : day_open;
	sym_data["day_high"] = day_high;
	sym_data["day_low"] = day_low;
	sym_data["day_ts"] = day_ts;
    data[symbol] = sym_data;
    
    //LOG_INFOS(LOG_DAYTICK,"symbol {} DayDate {}",symbol,Json2Str(data));
}

CLol::CLol(){}
CLol::~CLol(){}
void CLol::run(){
    while(true){
        sleep(60);
        for(auto it : __m_map_forever){
            it.second->put_todb();
        }
        g_btc_rate.put_todb();
        g_all_sym.put_todb();
    }
}

const int mac_len = 2;
const std::string mac_keys[mac_len] = {"g_last_depth","g_last_tick"};

bool CLol::init(){
    L_INFO("lol form db ");
    g_file_oper.init();

    for(int i=0;i<mac_len;i++){
        std::string s_key = mac_keys[i];
        std::shared_ptr<ONE_JSON> it =  std::make_shared<ONE_JSON>(s_key);
        __m_map_forever[s_key] = it;
    }
    for(auto it : __m_map_forever){
        it.second->init_fromdb();
    }
    g_btc_rate.init_fromdb();
    g_all_sym.init_fromdb();
    __m_thread = std::make_shared<std::thread>(&CLol::run,this);
    return true;
}

CLol g_lol;