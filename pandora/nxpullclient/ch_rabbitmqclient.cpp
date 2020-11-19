#include "ch_rabbitmqclient.h"
#include <unistd.h>
#include <ch_log.h>
#include <stdexcept>
#include <SimpleAmqpClient/BasicMessage.h>
#include "ch_tools.h"
#include <SimpleAmqpClient/Table.h>
#include <vector>
#include "ch_depthsnap.h"
#include <stdint.h>
#include "ch_lol.h"
#include "ch_proto.h"



//{"cmd":"depth","data":{"asks":null,"bids":[[1.0,458.495]],"symbol":"BTCETH"}}
//{"cmd":"tick","data":[{"amount":0.190066,"dir":1,"price":6668.3961}],"symbol":"BTCUSDT","ts":1531809790724}
//{"cmd":"depthplus","data":{"asks":[[0.016736,61.0],[0.016738,-118.4]],"bids":null,"symbol":"EOSETH"}}

CRabbitMqClient::CRabbitMqClient(Json::Value conf):ActiveObject("rabbitmqclient"),__m_conf(conf){}
CRabbitMqClient::~CRabbitMqClient(){}
int CRabbitMqClient::run(){
	if(__m_sync_all<1){
		__reqAll();
	}
	sleep(__m_config_syntime);
	__m_sync_all --;
	__reconn_push();
	
}

void CRabbitMqClient::__reqAll(){
	Json::Value j_msg;
	j_msg["cmd"] = "depth";
	j_msg["ts"] = getCurrentTime();
	j_msg["machineNum"] = __m_machineNum;
	std::string msg;
	msg = Json2Str(j_msg);
    AmqpClient::BasicMessage::ptr_t message = AmqpClient::BasicMessage::Create(msg);
    if(__m_rabit_conn[RABBIT_PUBLISH] == nullptr){
		L_INFO("______reqALL nullptr");
		__m_send_errorNum = 2;   //标记发送失败
		return ;
	}
	try{	
            __m_rabit_conn[RABBIT_PUBLISH]->BasicPublish(JsonStr(__m_product,"exchange"),JsonStr(__m_product,"routing"),message);
    }catch( std::runtime_error& e){
            LOG_INFOS(LOG_EMERGE,"catch rabbit send {}",e.what());
			__m_send_errorNum = 2;  //标记发送失败
			sleep(1);
    }
	LOG_INFOS(LOG_SYNC,"send {}",msg);
    LOG_INFOS(LOG_RABBITMQ,"send {}",msg);
}

void CRabbitMqClient::__protoParse(std::string qmsg_rev)
{
	Json::Value j_res;
	JsonFromStr(qmsg_rev, j_res);
	std::string cmd = JsonStr(j_res, "cmd");
	std::string symbol = JsonStr(j_res, "symbol");
	symbol = symbol2Pair(symbol);
	if (symbol.size() < 4) {
		return;
		LOG_INFOS(LOG_RABBITMQ, "cannot parse sym {} {}", symbol, qmsg_rev);
	}
	if (cmd == "depth") {
		Json2Snap(g_self_match_snap, j_res, symbol);  //更新深度快照
													  //LOG_INFOS(LOG_DEBUG,"mq depth rev alldata {}",qmsg_rev);
		//LOG_INFOS(LOG_DEBUG,"mq depth rev alldata {} {}", symbol, qmsg_rev);
		{
			__m_isAlldepth = true;
		}
		if(__m_sync_all < 2){
			__m_sync_all++;
		}

		LOG_INFOS(LOG_SYNC,"rev alldata ");
		
	}
	else if (cmd == "depthplus") {
		if (__m_isAlldepth == true) {
			JsonMergeSnap(g_self_match_snap, j_res, symbol); //合入增量深度快照
		}
		else {
			LOG_INFOS(LOG_RABBITMQ, "mq depthplus jump data! ");
		}
	}
	else if (cmd == "tick") {
		int64_t tick_ts = JsonInt64(j_res, "ts");
		Json::Value dataArr = JsonArr(j_res, "data");
		for (int i = 0; i < dataArr.size(); i++)
		{
			Json::Value tickOne = dataArr[i];
			//Json::Value j_m_pack;
			//j_m_pack["cmd"] = "tick";
			tickOne["symbol"] = symbol;
			tickOne["ts"] = tick_ts;
			if (JsonInt(tickOne, "dir")) {
				tickOne["dir"] = "ask";
			}
			else {
				tickOne["dir"] = "bid";
			}
			LOG_INFOS(LOG_TICK,"rabit tickOne {}",Json2Str(tickOne));
			__m_tick_queue.pushOne(tickOne);
		}
	}
}

void CRabbitMqClient::__recv(){
    //__m_isAlldepth.clear(); //清除所有全量标记
	__m_isAlldepth = false;
    __m_custom_tag = "";
	__resetTime();
	std::string url = JsonStr(__m_conf,"url");
	try{
		LOG_INFOS(LOG_RABBITMQ,"start conn...");
		__m_rabit_conn[RABBIT_CONSUMER]= AmqpClient::Channel::CreateFromUri(url);
		LOG_INFOS(LOG_RABBITMQ,"start conn... url {}",url);
		__m_rabit_conn[RABBIT_CONSUMER]->BindQueue(JsonStr(__m_custom,"queue"), JsonStr(__m_custom,"exchange"), JsonStr(__m_custom,"routing"));
		LOG_INFOS(LOG_RABBITMQ,"start binding...");
		sleep(1);
		__m_custom_tag = __m_rabit_conn[RABBIT_CONSUMER]->BasicConsume(JsonStr(__m_custom,"queue"));
		LOG_INFOS(LOG_RABBITMQ,"custom tag {}",__m_custom_tag);
	}catch( std::runtime_error& e){
		LOG_INFOS(LOG_RABBITMQ,"catah rabbit conn {}",e.what());
		sleep(1);
	}
    AmqpClient::Envelope::ptr_t envelope;
    while(true){
        try{
            if(!__m_rabit_conn[RABBIT_CONSUMER]->BasicConsumeMessage(__m_custom_tag,envelope,1000)){
                //LOG_INFOS(LOG_RABBITMQ,"recv timeout! {}",__m_timeout);
				__m_timeout--;
				__reconn();
				continue;
			}
			//__m_rabit_conn[RABBIT_CONSUMER]->BasicAck(envelope);
			__resetTime();
			std::string qmsg_rev = envelope->Message()->Body();
            LOG_INFOS(LOG_RABBITMQ,"rabbit recv {}",qmsg_rev);
			__protoParse(qmsg_rev);
        }
        catch( std::runtime_error& e){
			__m_timeout--;
			__reconn();
            LOG_INFOS(LOG_RABBITMQ,"catch rabbit rev {}",e.what());
			sleep(1);
         }
    }
}

void CRabbitMqClient::__reconn_push(){
	if(__m_send_errorNum != 2){   //如果发送失败标记生效重连发起
		return;
	}
	std::string url = JsonStr(__m_conf,"url");
	LOG_INFOS(LOG_SYNC,"reconn_push...url {}",url);
	try{
        __m_rabit_conn[RABBIT_PUBLISH]= AmqpClient::Channel::CreateFromUri(url);
		LOG_INFOS(LOG_SYNC,"reconn_create");
        __m_rabit_conn[RABBIT_PUBLISH]->BindQueue(JsonStr(__m_product,"queue"), JsonStr(__m_product,"exchange"), JsonStr(__m_product,"routing"));
		LOG_INFOS(LOG_SYNC,"reconn_bind");
   	}catch( std::runtime_error& e){
        LOG_INFOS(LOG_SYNC,"catch rabbit conn {}",e.what());
		__m_send_errorNum = 2;  //标记重连失败
        return ;
    }
	__m_send_errorNum = 0;   //标记重连成功
	LOG_INFOS(LOG_SYNC,"reconn_push...suc");
}

void CRabbitMqClient::__reconn(){
	if(__m_timeout < 1 ){
				__resetTime();
				std::string url = JsonStr(__m_conf,"url");
				try{
					LOG_INFOS(LOG_RABBITMQ,"start reconn...");
					__m_rabit_conn[RABBIT_CONSUMER]= AmqpClient::Channel::CreateFromUri(url);
					LOG_INFOS(LOG_RABBITMQ,"start reconn... url {}",url);
					__m_rabit_conn[RABBIT_CONSUMER]->BindQueue(JsonStr(__m_custom,"queue"), JsonStr(__m_custom,"exchange"), JsonStr(__m_custom,"routing"));
					LOG_INFOS(LOG_RABBITMQ,"start bind...");
					sleep(1);
					__m_custom_tag = __m_rabit_conn[RABBIT_CONSUMER]->BasicConsume(JsonStr(__m_custom,"queue"));
					LOG_INFOS(LOG_RABBITMQ,"custom tag {}",__m_custom_tag);
				}catch( std::runtime_error& e){
					LOG_INFOS(LOG_RABBITMQ,"catah rabbit conn {}",e.what());
					sleep(1);
				}
	}
}



bool CRabbitMqClient::init(){

    __m_thread_tick_job = std::make_shared<std::thread>(&CRabbitMqClient::__tick_job, this); ; 
	bool isMoke = JsonBool(__m_conf,"isMoke");  //isupdate  配置是否Moke((模仿数据))
	L_INFO("RabbitMq Moke {}",isMoke);
	if(isMoke){
		__m_thread_test = std::make_shared<std::thread>(&CRabbitMqClient::__test, this); //测试线程
		L_INFO("RabbitMq begin Moke");
		return false;
	}
	__m_machineNum = JsonInt(__m_conf,"machineNum");
	__m_config_syntime = JsonInt(__m_conf,"syntime");
	if(__m_config_syntime < 60){
		__m_config_syntime = 60;
	}
	__resetTime();
	L_INFO("synctime {} ",__m_config_syntime);
	L_INFO("RabbitMq wait conn...");
    std::string url = JsonStr(__m_conf,"url");

     __m_custom = JsonVal(__m_conf,"custom");
     __m_product = JsonVal(__m_conf,"product");
    LOG_INFOS(LOG_RABBITMQ,"custom {}",Json2Str(__m_custom));
    LOG_INFOS(LOG_RABBITMQ,"product {}",Json2Str(__m_product));
    try{
        __m_rabit_conn[RABBIT_PUBLISH]= AmqpClient::Channel::CreateFromUri(url);
        __m_rabit_conn[RABBIT_PUBLISH]->BindQueue(JsonStr(__m_product,"queue"), JsonStr(__m_product,"exchange"), JsonStr(__m_product,"routing"));
   }catch( std::runtime_error& e){
        LOG_INFOS(LOG_RABBITMQ,"catah rabbit conn {}",e.what());
        return false;
    }
    __m_thread = std::make_shared<std::thread>(&CRabbitMqClient::__recv, this);
    return true;
}


void CRabbitMqClient::__tick_job() {
	while(true){
		Json::Value data = __m_tick_queue.popOne();
        if(data == NULL)
        {
			//LOG_INFOS(LOG_TICK,"pop null");
            continue;
        }
		LOG_INFOS(LOG_TICK,"getOne");
		std::string symbol = JsonStr(data,"symbol");           
		double amount = JsonDouble(data,"amount");
		double price = JsonDouble(data,"price");
		if(amount <=0){
			LOG_INFOS(LOG_KLINE,"err amount<=0 {}",Json2Str(data));
			continue;
		}
		if(price <=0){
			LOG_INFOS(LOG_KLINE,"err price<=0 {}",Json2Str(data));
			continue;
		}

		g_tk_vec.addOnetick(symbol, data); //缓存最新的20根tick

    	int64_t ts = JsonInt64(data,"ts");

        data["symbol"] = symbol;
        {
			LOG_INFOS(LOG_TICK,"pushOne2 {}",Json2Str(data));
			g_bridge_symbol[REQ_PROTO_WEBSOCKET][REQ_PROTO_SUB_TICK].pushOne(data); //传递tick队列
			g_bridge_symbol[REQ_PROTO_TCP][REQ_PROTO_SUB_TICK].pushOne(data); //传递tick队列
			g_lol.setJsonPtr("g_last_tick",symbol,data);
		}
		g_day_tick.setDayVolume(symbol,amount);
		g_kl_calc_m.insertCalcTsSymbol(symbol, price, amount);
	}
}

void CRabbitMqClient::__depth_job() {
	while(true){

	}
}

///{"cmd":"depth","data":{"asks":null,"bids":[[1.0,458.495]],"symbol":"BTCETH"}}
std::string CRabbitMqClient::__mockDate_depth(std::string symbol,double price){
	Json::Value depth_moke;
    depth_moke["cmd"] = "depth";
	depth_moke["symbol"] = symbol;
	//depth_moke["ts"] = (int64_t)ch_getts_1s() * 1000 + 123;
	double amount = 1;
	Json::Value bidOne;
	bidOne.append(price);
	bidOne.append(amount);
	Json::Value bids;
	bids.append(bidOne);
	Json::Value data;
	data["bids"] = bids;
	depth_moke["data"] =  data;
	std::string s_mock = Json2Str(depth_moke);
	//L_INFO(" j tick {}", s_mock);
	return s_mock;
}
//{"cmd":"depthplus","data":{"asks":[[0.016736,61.0],[0.016738,-118.4]],"bids":null,"symbol":"EOSETH"}}
std::string CRabbitMqClient::__mockDate_depth_plus(std::string symbol,double price,double amount){
	Json::Value depth_moke;
    depth_moke["cmd"] = "depthplus";
	depth_moke["symbol"] = symbol;
	Json::Value bidOne;
	bidOne.append(price);
	bidOne.append(amount);
	Json::Value bids;
	bids.append(bidOne);
	Json::Value data;
	data["bids"] = bids;
	depth_moke["data"] =  data;
	std::string s_mock = Json2Str(depth_moke);
	//L_INFO(" j tick {}", s_mock);
	return s_mock;
}

std::string CRabbitMqClient::__mockDate_tick(std::string symbol,double price){
	Json::Value tick_moke;
    tick_moke["cmd"] = "tick";
	tick_moke["symbol"] = symbol;
	tick_moke["ts"] = (int64_t)ch_getts_1s() * 1000 + 123;
	Json::Value data;
	data["amount"] = 1;
	data["dir"] = 1;
	data["price"] = price;
	tick_moke["data"].append(data);
	std::string s_mock = Json2Str(tick_moke);
	//L_INFO(" j tick {}", s_mock);
	return s_mock;
}

// {"cmd":"tick","data":[{"amount":19.973,"dir":0,"price":0.010175}],"symbol":"LTCBTC","ts":1532941287636}

void CRabbitMqClient::__test() {
	//1,5,15,30,60,4*60,24*60,7*24*60,43200
	//{"cmd":"tick", "data" : [{"amount":0.190066, "dir" : 1, "price" : 6668.3961}], "symbol" : "BTCUSDT", "ts" : 1531809790724}
	//std::string hhh = "BYB_BTC";
	//sleep(100000);
#if 0
    sleep(1);
	sleep(10);
	__protoParse(__mockDate_tick("SYSETH",4444));
#endif
	double price1 = 7.1;
    double price2 = 70.1;
	double amount1 = 0.1;
	double amount2 = 10.1;
	std::string symbol1 = "EOSBTC";
	std::string symbol2 = "ETHBTC";
	__protoParse(__mockDate_depth(symbol1,price1));
	__protoParse(__mockDate_depth(symbol2,price2));


	std::string symbol3 = "BYBBTC";
	__protoParse(__mockDate_tick(symbol3,price2));

	///open high low close
	std::string symbol4 = "SYSBTC";
	double open = 8;
	double high = 11;
	double low = 6;
	double close = 7;
	__protoParse(__mockDate_tick(symbol4,open));
		__protoParse(__mockDate_tick(symbol4,high));
			__protoParse(__mockDate_tick(symbol4,low));
				__protoParse(__mockDate_tick(symbol4,close));

	while (true) {
		usleep(10000);
		//sleep(1);
		amount1 += 0.1;
		amount2 += 0.1;
		price1 ++;

	    __protoParse(__mockDate_tick(symbol1,price1));
		__protoParse(__mockDate_depth_plus(symbol1,price1,amount1));
		__protoParse(__mockDate_tick(symbol2,price2));
		__protoParse(__mockDate_depth_plus(symbol2,price2,amount2));

		std::string symbol4 = "ABCBTC";
	    __protoParse(__mockDate_depth_plus(symbol4,price2,amount2));

		
	}
}