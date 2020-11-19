#pragma once
#include "ch_json.h"
#include "ch_rpcserver.h"

class CWsServerRpc : public  CRpcServer{
public:
    CWsServerRpc(Json::Value);
    virtual ~CWsServerRpc();
    bool rpc_init();
    int run();
    void _push2apps(_SET_SES sess,std::string data);
    

};