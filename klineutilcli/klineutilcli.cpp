#include <stdio.h>
#include <stdio.h>
#include <sys/types.h>  
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include "../depends/nxcurl/ch_curl.h"
#include "../depends/nxtools/ch_tools.h"

#define NYX_PIPE_PATH "/tmp/nyx_fifo4.pipe"
#define NYX_PIPE_BUFLEN 300



void print_help(char* exe){
    printf("PARAMS: %s  ", exe);
    printf("startTime  ");
    printf("endTime  " );
    printf("symbol  ");
    printf("interval  \n");
    printf("usage: %s  ", exe);
    printf("1525155240  ");
    printf("1525155300  " );
    printf("BTCUSDT  ");
    printf("1m \n");
}

int main(int argc, char *argv[]){
    if(argc < 5)
    {
        print_help(argv[0]);
        return -1;
    }
    int   __m_pipe_handle = open(NYX_PIPE_PATH, O_RDWR);
    char* startTime = argv[1];
    char* endTime = argv[2];
    char* symbol = argv[3];
    char* interval = argv[4];
    printf("input startTime [%s] endTime [%s] symbol [%s] interval [%s]\n\n", startTime,endTime,symbol,interval);
    printf("start get klineData from binance\n\n");
    std::string  binance_kline_url = "https://www.binance.com/api/v1/klines?";
    std::string param = string_format("symbol=%s&interval=%s&startTime=%s000&endTime=%s000",\
    symbol,interval,startTime,endTime);
    binance_kline_url += param;
    printf("url %s\n\n",binance_kline_url.c_str());
    CRestApi c(binance_kline_url);
    Json::Value  jdata = c.getRequest();
    std::string sdata = Json2Str(jdata);
    printf("jdata %s\n\n",sdata.c_str());
    if(sdata.length()<50){
        printf("res error!\n\n");
        return -1;
    }
    printf("end get klineData from binance\n\n");

   // https://github.com/binance-exchange/binance-official-api-docs/blob/master/rest-api.md
   //[
   // 1499040000000,      // Open time
   // "0.01634790",       // Open
   // "0.80000000",       // High
   // "0.01575800",       // Low
   // "0.01577100",       // Close
   // "148976.11427815",  // Volume
   //]

    printf("parse json data\n\n");
    for(auto item : jdata){
        Json::Value j_item;
        j_item["symbol"] = symbol2Pair(symbol);
        j_item["interval"] = interval;
        j_item["ts"] = JsonInt64Ex(JsonPos(item,0));
        j_item["open"] = JsonStrEx(JsonPos(item,1));
        j_item["high"] = JsonStrEx(JsonPos(item,2));
        j_item["low"] = JsonStrEx(JsonPos(item,3));
        j_item["close"] = JsonStrEx(JsonPos(item,4));
        j_item["volume"] = JsonStrEx(JsonPos(item,5));
        std::string  s_item = Json2Str(j_item);
        printf("call pipe %s\n\n",s_item.c_str());
        write(__m_pipe_handle,s_item.c_str(),NYX_PIPE_BUFLEN);
    }
    
    return 0;
}