#include "ch_log.h"
#include "ch_tools.h"
/*
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
*/

const std::string ch_log_name[getTagLen()] = {"Nyx","RabbitMq","TcpServer",\
"WsServer","PipeServer","Kline",\
"Emerge","Tick","Db","Sync"};
std::shared_ptr<spdlog::logger> fileLogs[getTagLen()]; 
std::shared_ptr<spdlog::logger> console;

bool log_init()
{
    if(!ch_mkdir("log"))
    {
        printf(" %s failer!\n",__func__);
        return false;
    }
    for(int i=0;i<getTagLen();i++){
        std::string s_log_name = "log/" + std::string(ch_log_name[i]);

        if(!ch_mkdir(s_log_name.c_str())){
            printf("mkdir %s failer!\n",s_log_name.c_str());
            return false;
        }
        std::string filename = s_log_name + "/" + std::string(ch_log_name[i]) + ".log";
        fileLogs[i] = spdlog::rotating_logger_mt( std::string(ch_log_name[i]), filename, 1024 * 1024 * 20, 10);
    }
    console = spdlog::stdout_logger_mt("console");
    L_INIT(spdlog::level::debug);
    L_INFO("==================================================");
    L_INFO("{} ok!",__func__);
    return true;
}

bool is_logok = log_init();