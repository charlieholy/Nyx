#include "ch_poseidon.h"


CPoseidon *CPoseidon::_instance = NULL;

CPoseidon *CPoseidon::Instance()
{
	if(_instance == NULL)
	{
		_instance = new CPoseidon();
	}
	return _instance;
}
CPoseidon::CPoseidon(){}
CPoseidon::~CPoseidon(){
    __m_wsserverRpc = NULL;
    __m_tcpserverRpc = NULL;
    __m_rabbitmqclient = NULL;
    __m_httpclient = NULL;
    __m_kline = NULL;
    __m_piperpc = NULL;
}
int CPoseidon::Initialize(){
    if(!__m_config.loadConfig("./config.json"))
    {
        L_INFO("config error!");
        return ;
    }
    L_INFO("config {}",__m_config.toString());
    Json::Value c_root = __m_config.getConfig();

    __m_httpclient = std::make_shared<CHttpClient>(); 
    __m_rabbitmqclient = std::make_shared<CRabbitMqClient>(JsonVal(c_root,"rabbitmqclient"));
    __m_tcpserverRpc = std::make_shared<CTcpServerRpc>(JsonVal(c_root,"tcpserverRpc"));
    __m_wsserverRpc = std::make_shared<CWsServerRpc>(JsonVal(c_root,"wsserverRpc"));
    __m_kline = std::make_shared<CKline>();
    __m_piperpc = std::make_shared<CPipe>();
    g_lol.init();
}
int CPoseidon::Start(){
    g_leveldb.init();
    __m_kline->active(); //kline启动
    __m_piperpc->active(); //开启pipe通道 用于补线
    __m_httpclient->active();
    __m_tcpserverRpc->active(false);
    __m_wsserverRpc->active(false);
    __m_rabbitmqclient->active();
}
int CPoseidon::Stop(){
    __m_tcpserverRpc->deactive();
    __m_wsserverRpc->deactive();
    __m_kline->deactive();
    __m_piperpc->deactive();
    __m_rabbitmqclient->deactive();
    __m_httpclient->deactive();
}
void CPoseidon::Listen(Subject *aSubject){}

 Json::Value CPoseidongetKlineData(int period,std::string symbol,int num);


