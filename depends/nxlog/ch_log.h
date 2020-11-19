#pragma once
#include "spdlog/logger.h"
#include "spdlog/spdlog.h"
constexpr int getTagLen(){return 10;}
enum{
    LOG_NYX = 0,
    LOG_RABBITMQ,
    LOG_TCP_SERVER,
    LOG_WS_SERVER,
    LOG_PIPE_SERVER,
    LOG_KLINE,
    LOG_EMERGE,
    LOG_TICK,
    LOG_DB,
    LOG_SYNC,
};
extern  std::shared_ptr<spdlog::logger> fileLogs[getTagLen()];
extern  std::shared_ptr<spdlog::logger> console;

//初始化日志功能（级别）
#define L_INIT(level) {\
    for(auto it:fileLogs){\
        it->flush_on(level);\
    }\
}
#define LOG_UNINT() 
#define L_INFO(format, ...)     fileLogs[0]->info(format, ##__VA_ARGS__);console->info(format, ##__VA_ARGS__)
#define LOG_INFOS(i,format, ...)     fileLogs[i]->info(format, ##__VA_ARGS__)
#define LOG_ALARM() L_INFO("alarm {} {}",__FILE__,__LINE__)
#define LOG_ALARMS(i) LOG_INFOS(i,"alarm {} {}",__FILE__,__LINE__)