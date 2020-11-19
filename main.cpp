#include "ch_poseidon.h"

const char *__process__ = "Nyx";

int main(int argc,char* argv[]) 
{   


    std::uniform_int_distribution<int> distribution(0, 99); // 以離散型均勻分佈方式產生 int 亂數，範圍落在 0
    std::mt19937 engine; // 建立亂數生成引擎
    auto generator = std::bind(distribution, engine); // 利用 bind 將亂數生成引擎和分布組合成一個亂數生成物件
    int random = generator(); // 產生亂數
    printf("%d\n",random);


    L_INFO("process: {} version: {}, compile date: {} {}\n\n\n", __process__, "0.1.0", __DATE__, __TIME__);
    L_INFO("this pid {}\n",getpid());
#if 1
    if (process_exist(__process__) != 0 ) {
        printf("process: %s exist\n", __process__);
        exit(EXIT_FAILURE);
    }
#endif
    if(argc > 1){
        char* deamon = argv[1];
        if(memcmp("-daemon",deamon,6) == 0)
        {
            L_INFO("daemon start!");
            daemon(1, 1);
            process_keepalive();
        }
    }

    CPoseidon::Instance()->Initialize();
    CPoseidon::Instance()->Start();
    nw_loop_run();
    CPoseidon::Instance()->Stop();

    return 0;
}
