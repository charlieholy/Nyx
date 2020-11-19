#pragma once
#include "activeObject.h"
#include "ch_json.h"
enum{
   T_RATE,   //USD-CNY  美元对法币 
   T_TICK    //SYM-USD  数字货币对美元
};


//extern std::string string_rate[2];
extern  Json::Value getPriceSym2Money();  //获取对法币价格
extern  Json::Value getPriceSym2Btc();  //获取对btc价格
double __getUsd2MoneyPrice(std::string money); //USD-money price

class CHttpClient : public ActiveObject{
public :
    CHttpClient();
    virtual ~CHttpClient();
    int run();
    bool init();
private:
    void __update(int i); 
};