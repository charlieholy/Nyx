#pragma once
#include <vector>
#include <map>
#include <mutex>
#include "ch_json.h"
#include "ch_tools.h"
#include <string>
#include "ch_lol.h"

#define DEPTH_SIZE 200
///聚合下游depth数据
class CDepthItem {   //单个待排序元素
public:
	bool operator == (const CDepthItem &another) const         //重载去重
	{
		return price == another.price;
	}
       bool operator < (const CDepthItem &another) const
    {
        return price < another.price;
    }
    CDepthItem operator+(const CDepthItem &t) const{  
        CDepthItem temp;  
        temp.price = this->price;  
        temp.amount = this->amount + t.amount; 
        return temp;  
    }  
    std::string ToString(){
        char tos[100];
        sprintf(tos,"price: %g, amout: %g\n",price,amount);
        return std::string(tos);
    }
public:
	double price;
	double amount;
};

typedef std::vector<CDepthItem> V_DepthField;  //排序



typedef struct _S_MergeField {
	V_DepthField m_vfd[2];          ///asks bids
}S_MergeField;

typedef std::map<std::string, S_MergeField> MAP_SYMBOL_SNAP; // 交易对的depth



class CDepthSnap{
public:
    MAP_SYMBOL_SNAP getSnap(){
        std::lock_guard<std::mutex> guard(__m_mutex);
        return __m_snap;
    }
    void clear(std::string sym){
        std::lock_guard<std::mutex> guard(__m_mutex);
        __m_snap[sym].m_vfd[0].clear();
        __m_snap[sym].m_vfd[1].clear();
    }
    void inSert(std::string sym,S_MergeField mf){
        std::lock_guard<std::mutex> guard(__m_mutex);
        __m_snap[sym] = mf;
    }

private:
    MAP_SYMBOL_SNAP __m_snap;
    std::mutex __m_mutex;
};

class CSymFilter{
public:
    bool isfilter(std::string sym,int64_t ts){
        int64_t last = __m_sym_ts[sym];
        if(ts -  last < 250){
            return true;
        }
        //printf("last %lld  now %lld\n",last,ts);
        __m_sym_ts[sym] = ts;
        return false;
    }
private:
    std::map<std::string,int64_t> __m_sym_ts;
};

extern CSymFilter g_sym_filter;

extern std::map<std::string,std::vector<V_DepthField>> asks_stack; //打印异常增量栈回溯 //size==1 全量数据
 
void Json2field(V_DepthField &_f, Json::Value _arr); //转化待排序队列
void Json2Snap(CDepthSnap&_snap,Json::Value j_snap,std::string symbol);   //解析深度快照(snap)  //参考match2Nyx协议
void JsonMergeSnap(CDepthSnap&_snap,Json::Value j_plus,std::string symbol);   //增量更新深度快照(snap)
void filedInsert(V_DepthField &src,V_DepthField &_other,bool is_asks);  //合入增量挂单数据 asks/bids
void pushMergeSnap(std::string symbol,int64_t); //推送合并数据depth
void mergeField2(_S_MergeField _self,Json::Value& asksAndbids); //填充聚合数据 填充最优挂单  参考Nyx2app协议
void mergeField2Ab1(_S_MergeField _self,double* ask1,double* bid1);// 填充ask1 bid1
void Field2Json(V_DepthField _f,Json::Value& _j);  //vec数据转json数组

extern CDepthSnap  g_self_match_snap;  //自身交易所depth


