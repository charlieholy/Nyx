#include "ch_depthsnap.h"
#include "ch_log.h"
#include "ch_proto.h"

CDepthSnap  g_self_match_snap;  //自身交易所depth
std::map<std::string,std::vector<V_DepthField>> asks_stack; //打印异常增量栈回溯 //size==1 全量数据
CSymFilter g_sym_filter;

void Json2field(V_DepthField &_f, Json::Value _arr)     //转化待排序队列
{	
	for (int i = 0; i < _arr.size(); i++)
	{
		CDepthItem _m;
		_m.price = JsonDoubleEx((JsonPos(JsonPos(_arr, i), 0)));
		_m.amount = JsonDoubleEx((JsonPos(JsonPos(_arr, i), 1)));
		_f.emplace_back(_m);
	}
}

void Json2Mf(S_MergeField &_mf,Json::Value _item)         ////转化待排序队列 asks bids
{
    Json2field(_mf.m_vfd[0],JsonArr(_item,"asks"));
    Json2field(_mf.m_vfd[1],JsonArr(_item,"bids"));
}

void Json2Snap(CDepthSnap&_snap,Json::Value j_snap,std::string symbol){  // //解析深度快照(snap)  //参考match2Nyx协议
    Json::Value j_depth = JsonVal(j_snap,"data");
    S_MergeField _mf;
    Json2Mf(_mf,j_depth);
    _snap.clear(symbol);   //清除单个币种depth
    int64_t ts = JsonInt64(j_depth,"ts");
    _snap.inSert(symbol,_mf);
    pushMergeSnap(symbol,ts);            //新的depth触发合并产生新的聚合depth
}

//static int64_t g_ts = 0; //控制聚合瞬时冲击
void JsonMergeSnap(CDepthSnap&_snap,Json::Value j_plus,std::string symbol){
    Json::Value j_depth = JsonVal(j_plus,"data");
    S_MergeField _des;
    Json2Mf(_des,j_depth);
    int64_t ts = JsonInt64(j_depth,"ts");
    S_MergeField _src = _snap.getSnap()[symbol]; //根据symbol获取存储的快照
    filedInsert(_src.m_vfd[0],_des.m_vfd[0],true); //合入增量挂单asks
    filedInsert(_src.m_vfd[1],_des.m_vfd[1],false);  //合入增量挂单bids
    _snap.inSert(symbol,_src);  //存入新的快照
    pushMergeSnap(symbol,ts);            //新的depth触发合并产生新的聚合depth
        
}

std::string ch_printffield(V_DepthField ch_v)
{
    std::string s_res = " :";
    for(auto it : ch_v){
       s_res+= string_format(" %f %f ",it.price,it.amount);
    }
    return s_res;
}
//合入增量挂单
void filedInsert(V_DepthField &ch_old,V_DepthField &ch_new,bool is_asks){
	std::map<double, double> tmp; //price amount   //加上old
	for (int i = 0; i < ch_old.size() && i< DEPTH_SIZE; i++) {
		double value = +ch_old[i].amount;
		if (value != 0) {
			tmp.emplace(ch_old[i].price, value);
		}
	}
	for (int i = 0; i < ch_new.size()  && i<DEPTH_SIZE; i++) {        //加上new
		double ppp = (ch_new[i].amount + tmp[ch_new[i].price]);
		tmp[ch_new[i].price] = ppp;
	}
	V_DepthField v_res;   //增量数据合入vec
	for (auto it : tmp)
	{
		CDepthItem c;
		c.price = it.first;
		c.amount = it.second;
		if(c.amount >= 1e-8)
		v_res.emplace_back(c);
	}	
	if (!is_asks) 
	{
		std::reverse(v_res.begin(), v_res.end());
	}
	ch_old =  v_res;
}

  

void mergeField2(_S_MergeField _self, Json::Value& asksAndbids){
    Field2Json(_self.m_vfd[0],asksAndbids["a"]);
    Field2Json(_self.m_vfd[1],asksAndbids["b"]);
}

void mergeField2Ab1(_S_MergeField _self,double* ask1,double* bid1){
    if(_self.m_vfd[0].size()>0){
        *ask1 = _self.m_vfd[0][0].price;
    }
    if(_self.m_vfd[1].size()>0){
        *bid1 = _self.m_vfd[1][0].price;
    }
}


void Field2Json(V_DepthField _f,Json::Value& _j){
    int i = 0;
    for(auto it:_f){
        //
        if(i++ > DEPTH_SIZE){
            return;
        }
        Json::Value item;
        item.append(it.price);
        item.append(it.amount);
        _j.append(item);
    }
}

void pushMergeSnap(std::string symbol,int64_t ts){
     int64_t now_ts = getCurrentTime();
    g_sym_filter.isfilter(symbol,now_ts);

    S_MergeField self_mf = g_self_match_snap.getSnap()[symbol];
    Json::Value depdata;
    mergeField2(self_mf,depdata["data"]);
    //send to user
    depdata["cmd"] = "depth";
    depdata["symbol"] = symbol;
    depdata["ts"] = now_ts;

    //提取ab1数据
    double ask1 = 0;
    double bid1 = 0;
    mergeField2Ab1(self_mf,&ask1,&bid1);
    Json::Value ab1;
    ab1.append(symbol);
    ab1.append(ask1);
    ab1.append(bid1);
	//LOG_INFOS(LOG_RABBITMQ,Json2Str(depdata));
    {
        g_queuq_ab1[REQ_PROTO_TCP].pushOne(ab1); //only tcp
        g_bridge_symbol[REQ_PROTO_WEBSOCKET][REQ_PROTO_SUB_DEPTH].pushOne(depdata); //传递depth队列
        g_bridge_symbol[REQ_PROTO_TCP][REQ_PROTO_SUB_DEPTH].pushOne(depdata); //传递depth队列
        g_lol.setJsonPtr("g_last_depth",symbol,depdata);
    }
}