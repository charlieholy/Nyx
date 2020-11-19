#include "ch_pipe.h"
#include <stdio.h>
#include <sys/types.h>  
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include "ch_log.h"
#include "ch_kline.h"

#define NYX_PIPE_PATH "/tmp/nyx_fifo4.pipe"
#define NYX_PIPE_BUFLEN 300

CPipe::CPipe():ActiveObject("piperpc"){}
CPipe::~CPipe(){}
bool CPipe::init(){
    if(access(NYX_PIPE_PATH,R_OK)!=0){
        int ret = mkfifo(NYX_PIPE_PATH, 0666 | S_IFIFO);
        if (ret == -1){
            LOG_ALARMS(LOG_PIPE_SERVER);
            return false;
         }
    }

    __m_pipe_handle = open(NYX_PIPE_PATH, O_RDWR);
    if(__m_pipe_handle <0){
        LOG_ALARMS(LOG_PIPE_SERVER);
        return false;
    }
    return true;
}


int CPipe::run(){
    char pr[NYX_PIPE_BUFLEN];
    read(__m_pipe_handle,pr,NYX_PIPE_BUFLEN);
    LOG_INFOS(LOG_PIPE_SERVER,"pipe read {}",pr);
    Json::Value jdata;
    JsonFromStr(pr,jdata);
    CKline::ex_insert2db(jdata);        //插入补线数据
}