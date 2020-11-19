#include "ch_httpclient.h"
#include "ch_curl.h"
#include <unistd.h>
#include "ch_rabbitmqclient.h"
#include "ch_kline.h"
#include <string>
#include "ch_lol.h"


std::string string_rate[2];
std::string rest_db_kl[2] = {"EXCHANGE_RATE_DB","EXCHANGE_TICK_DB"};
std::string rest_url[2] = {\
"https://openexchangerates.org/api/latest.json?app_id=679d1fc3a76b428fb42ee55853e09fc0",\
"https://api.coinmarketcap.com/v2/ticker/"\
};

std::string  h_money[2] = {"USD","CNY"};

CHttpClient::CHttpClient():ActiveObject("httpclient"){}
CHttpClient::~CHttpClient(){}

void CHttpClient::__update(int i){
    CRestApi A(rest_url[i]);
    Json::Value j_data = A.getRequest();
    string_rate[i] = Json2Str(j_data);
    g_leveldb.put(rest_db_kl[i], string_rate[i].c_str());
}
static int s_chttp_flag=0;
int CHttpClient::run(){
    for(int i=0;i<2;i++)
    {
        g_leveldb.get(rest_db_kl[i], string_rate[i]);
        if(string_rate[i].size() < 20)
        {
           __update(i);
        }
    }
    //每一个小时更新rate  每60秒更新tick
    sleep(300);
    __update(1);
    s_chttp_flag++;
    if(s_chttp_flag > 60){
        s_chttp_flag = 0;
        __update(0);
    }
}
bool CHttpClient::init(){
  
    return true;
}

Json::Value getPriceSym2Btc(){
        Json::Value j_res;
        Json::Value j_money;
	    JsonFromStr(string_rate[T_TICK], j_money);
    	Json::Value data = JsonVal(j_money, "data");
        double btc_ust_price = 0; //
        for (auto it : data)
        {	
            std::string symbol = JsonStr(it, "symbol");
            double sym_usd_price = JsonDouble(JsonVal(JsonVal(it, "quotes"), "USD"), "price"); ///sym -usd
            if(symbol == "BTC"){
                btc_ust_price = sym_usd_price;
                j_res[symbol] = 1;
                continue;
            }
            j_res[symbol] = sym_usd_price / btc_ust_price;
        }
		//获取本地交易所产生的对btc的价格
        Json::Value btcprice = g_btc_rate.btc_price_frommem();
        //LOG_INFOS(LOG_DEBUG,"btcprice {}",Json2Str(btcprice));
        Json::Value::Members mem = btcprice.getMemberNames();      
        for (auto iter = mem.begin(); iter != mem.end(); iter++)  {
            //LOG_INFOS(LOG_DEBUG,"key {} v {}",(*iter).c_str(), JsonDouble(btcprice,*iter));
            j_res[*iter] =  JsonDouble(btcprice,*iter);
        }

        return j_res;
}

Json::Value getPriceSym2Money(){
    Json::Value j_res;
    
    /////
    Json::Value j_money;
	JsonFromStr(string_rate[T_TICK], j_money);
	Json::Value data = JsonVal(j_money, "data");
    for(auto money:h_money)
    {
        double usd_money_price = __getUsd2MoneyPrice(money); // usd - money 
        Json::Value j_res_data;
        Json::Value j_symbol_price;
        for (auto it : data)
        {	
            std::string s_it = Json2Str(it);
            std::string symbol = JsonStr(it, "symbol");
            double sym_usd_price = JsonDouble(JsonVal(JsonVal(it, "quotes"), "USD"), "price"); ///sym -usd
            double sym_money = (sym_usd_price*usd_money_price);
            j_symbol_price[symbol] =  sym_money;
        }
        j_res_data["money"] = money;
        j_res_data["symbols"] = j_symbol_price;
        j_res.append(j_res_data);
    }   
    return j_res;
}


double __getUsd2MoneyPrice(std::string money) //USD-money price
{
	Json::Value j_money;
    JsonFromStr(string_rate[T_RATE],j_money);
	Json::Value rates = JsonVal(j_money, "rates");
	return  JsonDouble(rates, money);
}

