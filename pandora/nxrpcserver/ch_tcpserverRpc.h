#pragma once
#include "ch_json.h"
#include "ch_rpcserver.h"

static const char* rpc_magic_head = "magicNyxV0.1";

extern int decode_pack(void *data);  
extern int rpc_pack_proto(std::string ss,char *pack);

class CTcpServerRpc : public  CRpcServer{
public:
    CTcpServerRpc(Json::Value);
    virtual ~CTcpServerRpc();
    bool rpc_init();
    int run();
    void _push2apps(_SET_SES sess,std::string data);

};