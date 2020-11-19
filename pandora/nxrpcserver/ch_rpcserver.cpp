#include "ch_rpcserver.h"
#include <unistd.h>
#include "ch_httpclient.h"
#include "ch_log.h"
#include "ch_lol.h"
#define MAX_SEND_BUF 102400

CRpcServer::CRpcServer(Json::Value conf,std::string name,int protoType):ActiveObject(name),_m_conf(conf),_m_protoType(protoType){}
CRpcServer::~CRpcServer(){}
bool CRpcServer::rpc_init(){}
bool CRpcServer::init(){
    if(!rpc_init()){
        return false;
    }
    printf("============init Rpcserver!\n");
    __m_thread[0] = std::make_shared<std::thread>(&CRpcServer::__sendtick, this);
    __m_thread[1] = std::make_shared<std::thread>(&CRpcServer::__senddepth, this);
    __m_thread[2] = std::make_shared<std::thread>(&CRpcServer::__sendrate, this);
    __m_thread[3] = std::make_shared<std::thread>(&CRpcServer::__sendallAB, this);
    return true;
}

int CRpcServer::run(){}

void CRpcServer::__sendallAB(){
    while(true)
    {
        Json::Value item = g_queuq_ab1[_m_protoType].popOne();
        if(item == NULL)
        {
            continue;
        }
        Json::Value one_;
        one_["cmd"] = "allAB";
        one_["data"]=item;
        std::string s_item = Json2Str(one_);
        _SET_SES set_allAB_sess = g_sub_allAB[_m_protoType];
        _push2apps(set_allAB_sess,s_item);
    }
}

void CRpcServer::__sendtick(){
    std::string sym;
    std::string s_item;
    while(true)
    {
        if(! customTicks(_m_protoType,sym,s_item)){
                continue;
        }
        _SET_SES set_tick_sess = g_sub_map_arr[_m_protoType][REQ_PROTO_SUB_TICK][sym];
        _push2apps(set_tick_sess,s_item);
    }
}
void CRpcServer::__senddepth(){
    while(true)
    {
        Json::Value item = g_bridge_symbol[_m_protoType][REQ_PROTO_SUB_DEPTH].popOne();
        if(item == NULL)
        {
            continue;
        }
        std::string sym = JsonStr(item,"symbol");
            //推depth的时候把最新的tick带过去
        item["data"]["tick"] = g_lol.getJsonPtr("g_last_tick",sym);
        std::string s_item = Json2Str(item);
        _SET_SES set_depth_sess = g_sub_map_arr[_m_protoType][REQ_PROTO_SUB_DEPTH][JsonStr(item,"symbol")];
        _push2apps(set_depth_sess,s_item);
    }
}
void CRpcServer::__sendrate(){
    while(true)
    {
        sleep(60);
        {
            {
                Json::Value j_rate;
                j_rate["cmd"] = "exTick";
                j_rate["data"] = getPriceSym2Money();
                std::string s_item = Json2Str(j_rate);
                _SET_SES set_rate_sess = g_sub_rate_set[_m_protoType];
                _push2apps(set_rate_sess,s_item);
            }
        }
    }
}
