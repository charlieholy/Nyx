#include <stdio.h>
#include <set>
#include <mutex>
#include <vector>
#include <map>
#include <algorithm>
#include "ch_json.h"

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

//std::string ch_printffield(V_DepthField ch_v)；


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
    clear(){
        std::lock_guard<std::mutex> guard(__m_mutex);
        __m_snap.clear();
    }
    inSert(std::string sym,S_MergeField mf){
        std::lock_guard<std::mutex> guard(__m_mutex);
        __m_snap[sym] = mf;
    }
    bool isFilter(std::string symbol,bool askbid,double price){               //filter
        MAP_SYMBOL_SNAP  tmp_snap;
        {
            std::lock_guard<std::mutex> guard(__m_mutex);
            tmp_snap = __m_snap;
        }
        S_MergeField s_mf = tmp_snap[symbol];
        //filter  根据本地挂单过滤上游tick  
        // filter update_ask_tick > local_ask0
        // filter update_bid_tick < local_bid0
        if(askbid){
            if(s_mf.m_vfd[0].size()>0){
                if(price > s_mf.m_vfd[0][0].price){
                    return true;
                }
            }
        }
        else {
            if(s_mf.m_vfd[1].size()>0){
                if(price < s_mf.m_vfd[1][0].price){
                    return true;
                }
            }
        }
        return false;
    }
private:
    MAP_SYMBOL_SNAP __m_snap;
    std::mutex __m_mutex;
};

void Json2field(V_DepthField &_f, Json::Value _arr) //转化待排序队列
{
    for (int i = 0; i < _arr.size(); i++)
	{
		CDepthItem _m;
		_m.price = JsonDoubleEx((JsonPos(JsonPos(_arr, i), 0)));
		_m.amount = JsonDoubleEx((JsonPos(JsonPos(_arr, i), 1)));
		_f.emplace_back(_m);
	}
}

void filedInsert(V_DepthField &ch_old,V_DepthField &ch_new,bool is_asks=true)  //合入增量挂单数据 asks/bids
{
    std::map<double, double> tmp; //price amount   //加上old
	for (int i = 0; i < ch_old.size(); i++) {
		double value = +ch_old[i].amount;
		if (value != 0) {
			tmp.emplace(ch_old[i].price, value);
		}
	}
	for (int i = 0; i < ch_new.size(); i++) {        //加上new
		double ppp = (ch_new[i].amount + tmp[ch_new[i].price]);
		tmp[ch_new[i].price] = ppp;
	}
	V_DepthField v_res;   //增量数据合入vec
	for (auto it : tmp)
	{
		CDepthItem c;
		c.price = it.first;
		c.amount = it.second;
		if(c.amount >= 0.00000001){
            printf(">=0.000000001");
            v_res.emplace_back(c);
        }
		
	}	
	//ch_vprint(v_res);
	if (!is_asks) 
	{
		std::reverse(v_res.begin(), v_res.end());
	}
	ch_old =  v_res;
}

int main(){
  
  Json::Value asks;
  double price = 4334;
  double amount = 0.000000000001;
  Json::Value ask;
  ask.append(price);
  ask.append(amount);
  asks.append(ask);

  Json::Value asks_new;

  

#if 0
  double price_new = 4334;
  double amount_new = -0.1;
  Json::Value ask_new;
  ask_new.append(price_new);
  ask_new.append(amount_new);
  asks_new.append(ask_new);
#endif
  printf("%s\n",Json2Str(asks).c_str());
 
  V_DepthField ch_old;
  V_DepthField ch_new;
  Json2field(ch_old,asks);
  Json2field(ch_new,asks_new);
  printf("old: \n");
  for(auto it : ch_old){
      printf("%s ",it.ToString().c_str());
  }

  filedInsert(ch_old,ch_new);
  printf("new old: \n");
  for(auto it : ch_old){
      printf("%s ",it.ToString().c_str());
  }

  printf("begin!\n");
  return 0;
}