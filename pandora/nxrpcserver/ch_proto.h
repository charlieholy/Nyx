#pragma once
#include <set>
#include "ch_queue.h"
#include "ch_json.h"
#include "CppSQLite3.h"
#include "ch_leveldb.h"
#include "nw_ses.h"

enum reqErrorType {
	REQ_OK = 0,               // 0  请求正常
	REQ_ALL_LEN_OUT_ERROR,    // -1 请求包数据长度小于20   
	REQ_PART_MAGIC_ERROR,     // -2 magic错误             
	REQ_PART_LEN_ERROR,		  // -3 请求包len解析错误
	REQ_PART_CRC_ERROR,       // -4 crc32校验码错误
	REQ_PART_JSON_ERROR,      // -5 json解析失败
	REQ_PART_JSON_CMD_ERROR,  // -6 未知的cmd
	REQ_PART_JSON_PARAM_ERROR,// -7 参数错误
	REQ_DOJOB_ERROR,          // -8 获取请求数据失败
};

enum reqProtoType {
	REQ_PROTO_WEBSOCKET = 0,  //websocket 协议
	REQ_PROTO_TCP,            //tcp 协议
};

enum reqProtoSubType {
	REQ_PROTO_SUB_TICK = 0,   //tick 订阅类型
	REQ_PROTO_SUB_DEPTH,      //depth 订阅类型
};

typedef std::set<nw_ses*> _SET_SES;
extern _SET_SES g_sub_rate_set[2]; //websocket tcp  ////存储汇率订阅
extern _SET_SES g_sub_allAB[2];                    //存储aks1bid1订阅

extern void gf_sub_clear_all(nw_ses *ses,int protoType,int subType);
extern void gf_onclose(int type,nw_ses *ses); //websocket tcp

extern std::map<std::string,_SET_SES> g_sub_map_arr[2][2];   // websockt tcp //tick depth

extern std::string g_db_keys[2];  //leveldb key 存储最新 tick depth




extern CQueue<Json::Value> g_bridge_symbol[2][2];   // websockt tcp //tick depth
extern CQueue<Json::Value> g_queuq_ab1[2]; ///  ab1   ///websockt tcp //tick depth

extern CppSQLite3DB g_sqlite3_kxm;			//sqlite3操作句柄 

extern Json::Value g_tick2rate;  			//数字货币对法币
//统一协议rpc解析
extern Json::Value gf_commonProto(nw_ses *ses,std::string,int type); //type 0 websocket 1 tcp

///tick custom
extern bool customTicks(int type,std::string& symbol,std::string& s_tick);   //websockt tcp
