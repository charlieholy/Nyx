#pragma once
#include "ch_object.h"
#include "ch_rabbitmqclient.h"
#include "ch_config.h"
#include "ch_log.h"
#include "ch_leveldb.h"
#include "ch_kline.h"
#include "ch_proto.h"
#include "ch_httpclient.h"
#include "ch_pipe.h"
#include "nw_ses.h"
#include "ch_tools.h"
#include "ut_misc.h"
#include "ch_tcpserverRpc.h"
#include "ch_wsserverRpc.h"
#include "ch_lol.h"

class CPoseidon : public  Observer{
public:
    static CPoseidon *Instance();
    ~CPoseidon();
    int Initialize();
    int Start();
    int Stop();
    virtual void Listen(Subject *aSubject);
    Json::Value getKlineData(int period,std::string symbol,int num);
private:
    std::shared_ptr<CHttpClient> __m_httpclient;
    std::shared_ptr<CRabbitMqClient> __m_rabbitmqclient;
    std::shared_ptr<CKline> __m_kline;
    std::shared_ptr<CPipe> __m_piperpc;
    std::shared_ptr<CTcpServerRpc> __m_tcpserverRpc;
    std::shared_ptr<CWsServerRpc> __m_wsserverRpc;
    CConfig __m_config;
private:
	CPoseidon();
private:
	static CPoseidon *_instance;
};

