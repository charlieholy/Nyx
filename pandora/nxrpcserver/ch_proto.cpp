#include "ch_proto.h"
#include "ch_kline.h"
#include "ch_httpclient.h"
#include "ch_log.h"

//记录订阅品种
std::map<std::string,_SET_SES> g_sub_map_arr[2][2];   // websockt tcp //tick depth
_SET_SES g_sub_rate_set[2]; //websocket tcp  ////存储汇率订阅
_SET_SES g_sub_allAB[2];                    //存储aks1bid1订阅

std::string g_db_keys[2] = {"tick_","depth_"};   //leveldb key 存储最新 tick depth

CQueue<Json::Value> g_bridge_symbol[2][2];       // websockt tcp //tick depth
CQueue<Json::Value> g_queuq_ab1[2];   ///  ab1
CppSQLite3DB g_sqlite3_kxm;         //K线操作句柄

std::string g_rate;                 //法币汇率
Json::Value g_tick2rate;          //数字货币对法币

void gf_sub_clear_all(nw_ses *ses,int protoType,int subType)
{
    for(auto it : g_sub_map_arr[protoType][subType])
    {   
        g_sub_map_arr[protoType][subType][it.first].erase(ses);
    }
}

void gf_onclose(int type,nw_ses *ses)  //websocket tcp
{
    gf_sub_clear_all(ses,type,REQ_PROTO_SUB_TICK);
    gf_sub_clear_all(ses,type,REQ_PROTO_SUB_DEPTH);
    g_sub_rate_set[type].erase(ses);
    g_sub_allAB[type].erase(ses);
}



bool customTicks(int type,std::string& symbol,std::string& s_tick)  //websockt tcp
{
       Json::Value item = g_bridge_symbol[type][REQ_PROTO_SUB_TICK].popOne();
        if(item == NULL)
        {
            return false;
        }
        Json::Value j_tick;
        j_tick["cmd"] = "tick";
        std::string sym = JsonStr(item,"symbol");
        j_tick["symbol"] = sym;
        g_day_tick.addDayDate(sym,item);
        item.removeMember("symbol");
        j_tick["data"] = item;
        s_tick =  Json2Str(j_tick);
        symbol = sym;
        return true;
}


//统一协议rpc解析
Json::Value gf_commonProto(nw_ses *ses,std::string s_req,int type){
    Json::Value j_res;
    Json::Reader jread;
    Json::Value parse_json;
    if(!jread.parse(s_req,parse_json))
    {
        j_res["code"] = -REQ_PART_JSON_ERROR;
        j_res["msg"] = "json parse error!";
        return j_res;
    }
    std::string id = JsonStr(parse_json,"id");
    if(id!="")
    {
        j_res["id"] = id;
    }
    std::string cmd = JsonStr(parse_json,"cmd");
    j_res["cmd"] = cmd;
    j_res["code"] = 0;
    if("ping" == cmd)
    {   
        j_res["cmd"] = "pong";
    }
    else if("depth" == cmd)
    {
        std::string channel = JsonStr(parse_json,"channel");
        std::string symbol = JsonStr(parse_json,"symbol");
        if(channel != "add" && channel != "del")
        {
            j_res["code"] = -REQ_PART_JSON_PARAM_ERROR;
            j_res["msg"] = "unknown channel " + channel;
            return j_res;
        }
        if(symbol == "")
        {   
            j_res["code"] = -REQ_PART_JSON_PARAM_ERROR;
            j_res["msg"] = "unknown symbol " + symbol;
            return j_res;
        }
        j_res["channel"] = channel;
        j_res["symbol"] = symbol;

        j_res["data"] = JsonVal(g_lol.getJsonPtr("g_last_depth",symbol),"data");
        j_res["data"]["tick"] = g_lol.getJsonPtr("g_last_tick",symbol);
		j_res["data"]["ticks"] = g_tk_vec.getAllSym(symbol);
        if("add" == channel)
        {   
            g_sub_map_arr[type][REQ_PROTO_SUB_DEPTH][symbol].insert(ses);
        }
        else if("del" == channel)
        {
            g_sub_map_arr[type][REQ_PROTO_SUB_DEPTH][symbol].erase(ses);
        }
    }
    else if("ticks" == cmd)  ////一次订阅多个品种
    {
        std::string channel = JsonStr(parse_json,"channel");
        Json::Value symbols = JsonArr(parse_json,"symbols");
        Json::Value datas;
        if(channel != "add" && channel != "del")
        {
            j_res["code"] = -REQ_PART_JSON_PARAM_ERROR;
            j_res["msg"] = "unknown channel " + channel;
            return j_res;
        }
        for (int i = 0; i < symbols.size(); i++)
        {   
            std::string symbol = JsonStrEx(JsonPos(symbols,i));
     
            if("add" == channel)
            {   
                Json::Value data;
                data = g_lol.getJsonPtr("g_last_tick",symbol);
                g_day_tick.addDayDate(symbol,data);
                datas.append(data);
                g_sub_map_arr[type][REQ_PROTO_SUB_TICK][symbol].insert(ses);
            }
            else if("del" == channel)
            {
                g_sub_map_arr[type][REQ_PROTO_SUB_TICK][symbol].erase(ses);
            }
        }
        j_res["channel"] = channel;
        j_res["symbols"] = symbols;
        j_res["datas"] = datas;
        j_res["code"] = 0;
    }
    else if("chart" == cmd)
    {   
        std::string symbol = JsonStr(parse_json,"symbol");
        int period = JsonInt(parse_json,"period");
        int bar = JsonInt(parse_json,"bar");
        if(symbol == "")
        {
            j_res["code"] = -REQ_PART_JSON_PARAM_ERROR;
            j_res["msg"] = "unknown symbol " + symbol;
            return j_res;
            
        }
        if(period == -1)
        {
            j_res["code"] = -REQ_PART_JSON_PARAM_ERROR;
            j_res["msg"] = "unknown symbol " + period;
            return j_res;
        }
        if(bar == -1)
        {
            j_res["code"] = -REQ_PART_JSON_PARAM_ERROR;
            j_res["msg"] = "unknown symbol " + period;
        }
        j_res["symbol"] = symbol;
        j_res["period"] = period;
        j_res["bar"] = bar;
        ////
        Json::Value j_karr = CKline::getJsonArrfromdb(period,symbol,bar);
        j_res["code"] = REQ_OK;
        j_res["data"] = j_karr;
    }
    else if("exTick" == cmd)
    {
        Json::Value data = getPriceSym2Money();
        j_res["data"] = data;
        g_sub_rate_set[type].insert(ses);
    }
    else if("exTicBTC" == cmd)
    {
        Json::Value data = getPriceSym2Btc();
        j_res["data"] = data;
    }
    else if("allAB" == cmd){
        g_sub_allAB[type].insert(ses);
    }
    else {
        j_res["code"] = -REQ_PART_JSON_CMD_ERROR;
        j_res["msg"] = "unknown cmd " + cmd;
    }
    return j_res;
}