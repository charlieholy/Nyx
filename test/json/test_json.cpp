#include "ch_json.h"
#include <string>
#include "ch_tools.h"
#include <vector>
#include <mutex>
#include <thread>
#include <unistd.h>


#include <queue>
#include <mutex>
#include <condition_variable>

template<typename T>

class CQueue{

public:
    CQueue(){}
    ~CQueue(){}
    T  popOne(){
        T  t_one = NULL;
        std::unique_lock<std::mutex> lock(__m_mutex);
        while (__m_t_queue.empty()) {
            __m_cond.wait_for(lock, std::chrono::milliseconds(__m_loopTime));
            if (__m_t_queue.empty())
            {
                return t_one;
            }
            else
            {
                break;
            }
        }
        t_one  = __m_t_queue.front();
        __m_t_queue.pop();
        return t_one;
    }

    void pushOne(T t_queue){
        std::lock_guard<std::mutex> guard(__m_mutex);
        __m_t_queue.push(t_queue);
    }

private:
    std::queue<T > __m_t_queue;
    std::mutex __m_mutex;
    std::condition_variable __m_cond;
    int __m_loopTime = 1000;
};

CQueue<Json::Value > g_queue;
//缓存20个历史成交
class TK_VEC {
public:
	Json::Value getAllSym(std::string sym) {
		Json::Value j_tick20;
		j_tick20.resize(0);
		std::vector<Json::Value> que;
		{
			std::lock_guard<std::mutex> guard(__m_mutex);
			que = __m_tk_vec[sym];
		}
		for (auto it : que) {
			j_tick20.append(it);
		}
		return j_tick20;
	}
	void addOnetick(std::string sym, Json::Value j_tick) {
		std::vector<Json::Value> vec;
		{
			std::lock_guard<std::mutex> guard(__m_mutex);
			vec = __m_tk_vec[sym];
		}
		vec.emplace_back(j_tick);
		if (vec.size() > 20)vec.erase(vec.begin());
		{
			std::lock_guard<std::mutex> guard(__m_mutex);
			__m_tk_vec[sym] = vec;
		}
	}
private:
	std::map<std::string, std::vector<Json::Value>> __m_tk_vec;
	std::mutex __m_mutex;
};

 TK_VEC g_tk_vec;  //用于存储20个历史tick数据

void jo3(){
    while(1){
        //usleep(1000);
        Json::Value data;
         g_queue.pushOne(data);
         printf("pushOne\n");
    }
}

void jo2(){
    while(1){
        //usleep(1000);
        g_queue.popOne();
        printf("popOne\n");
    }
}

void jo1(){
    while(1){
        //usleep(1000);
        g_tk_vec.getAllSym("nani");
        printf("job1\n");
    }

}



int main( int argc, char* argv[] )
{
   


    //std::thread job1(jo1);
    std::thread job2(jo2);
    std::thread job3(jo3);

  //  job1.join();
    job2.join();
    job3.join();
    while(1){
        sleep(1);
    }

    return 0;
}