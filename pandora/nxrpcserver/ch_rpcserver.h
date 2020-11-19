#pragma once
#include "activeObject.h"
#include "ch_json.h"
#include "nw_ses.h"
#include "ch_proto.h"
#include <string>
class CRpcServer : public ActiveObject{
public:
    CRpcServer(Json::Value,std::string,int);
    virtual ~CRpcServer();
    virtual bool rpc_init();
    bool init();
    virtual int run() override;
protected:
    Json::Value _m_conf;
    virtual void _push2apps(_SET_SES sess,std::string data) = 0;
    int _m_protoType;  //0:websocket  1:tpc
private:
    void __sendtick();
    void __senddepth();
    void __sendrate();
    void __sendallAB();
    std::shared_ptr<std::thread> __m_thread[4];
};