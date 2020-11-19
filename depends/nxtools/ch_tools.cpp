#include "ch_tools.h"
#include <unistd.h>
#include <sys/stat.h>



bool ch_mkdir(std::string dir){
	if(access(dir.c_str(),R_OK)!=0)
	{
		if(mkdir(dir.c_str(),S_IRUSR | S_IWUSR | S_IXUSR)==0)
		{
			return true;
		}
		return false;
	}
	return true;
}

int64_t getCurrentTime()
{    
	struct timeval tv;    
	gettimeofday(&tv,NULL);    //该函数在sys/time.h头文件中
	return tv.tv_sec * 1000 + tv.tv_usec / 1000;    
}   

int ch_getts_1m(){
	struct timeval mtm;
    gettimeofday(&mtm, NULL);
    int ts =  mtm.tv_sec;
	return ts /60 * 60;
}

int ch_getts_5s(){
	struct timeval mtm;
    gettimeofday(&mtm, NULL);
    int ts =  mtm.tv_sec;
	return ts /5 * 5;
}

int ch_getts_1s() {
	struct timeval mtm;
	gettimeofday(&mtm, NULL);
	int ts = mtm.tv_sec;// +mtm.tv_usec;
	return ts;
}

int64_t ch_getts_1ms() {
	struct timeval mtm;
	gettimeofday(&mtm, NULL);
	return (int64_t)(mtm.tv_sec) * 1000 + mtm.tv_usec;
}




std::string  symbol2Pair(std::string sym)
{
	int len = sym.size();
	std::string sym1;
	std::string sym2;
	if (len < 4)
	{
		return "";
	}
	if (sym.substr(len - 4, 4) == "USDT")
	{
		sym2 = "USDT";
		sym1 = sym.substr(0, len - 4);
	}
	else if (sym.substr(len - 3, 3) == "BTC")
	{
		sym2 = "BTC";
		sym1 = sym.substr(0, len - 3);
	}
	else if (sym.substr(len - 3, 3) == "ETH")
	{
		sym2 = "ETH";
		sym1 = sym.substr(0, len - 3);
	}
	return sym1 + "_" + sym2;
}