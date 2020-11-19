#pragma once
#include "activeObject.h"
#include <thread>
#include "ch_kline.h"

class  CPipe : public ActiveObject{
public:
    CPipe();
    ~CPipe();
    int run();
    bool init();
    void insert2db(Json::Value);
private:
    int __m_pipe_handle;
};