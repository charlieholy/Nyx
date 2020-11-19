#pragma once
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