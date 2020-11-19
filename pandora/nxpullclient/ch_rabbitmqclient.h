#pragma once
#include <SimpleAmqpClient/SimpleAmqpClient.h>
#include "activeObject.h"
#include "ch_kline.h"

enum{
    RABBIT_PUBLISH = 0,   //请求链路  
    RABBIT_CONSUMER,     //消费链路
};

class CRabbitMqClient : public ActiveObject{
public:
    CRabbitMqClient(Json::Value);
    virtual ~CRabbitMqClient();
    int run();
    bool init();
private:
	void __protoParse(std::string rev); //解析mq协议
    void __recv();  //接收线程
	void __test();  //模拟测试线程
    void __reconn(); ///接收端重连
    void __reconn_push();///发送端重连
    void __tick_job(); //
    void __depth_job(); //
    void __reqAll(); //请求全量
    std::string __mockDate_tick(std::string symbol,double price);
    std::string __mockDate_depth(std::string symbol,double price);
    std::string __mockDate_depth_plus(std::string symbol,double price, double amount);
    Json::Value __m_conf;  //配置
    AmqpClient::Channel::ptr_t __m_rabit_conn[2]; //pub  cus  //链接句柄
    Json::Value __m_custom;  //消费者的配置
    Json::Value __m_product;  
    std::shared_ptr<std::thread> __m_thread;  //接收线程
    std::string __m_custom_tag;   //消费Tag
    //std::map<std::string,bool> __m_isAlldepth; //标记是否已经收到全量数据
    bool __m_isAlldepth = false; //标记是否已经收到全量数据
    std::mutex __m_mtx;
	std::shared_ptr<std::thread> __m_thread_test; //测试线程
    std::shared_ptr<std::thread> __m_thread_tick_job; //tick
    std::shared_ptr<std::thread> __m_thread_depth_job; //depth
    std::shared_ptr<std::thread> __m_keepalive; //keepalive
    int __m_sync_all=0;
    int __m_machineNum=0;
    int __m_timeout=0;
    int __m_config_syntime=0;
    int __m_send_errorNum=0;  //标记发送失败
    CQueue<Json::Value> __m_tick_queue; //消费tick
    void __resetTime(){ __m_timeout = (__m_config_syntime*4);}
};