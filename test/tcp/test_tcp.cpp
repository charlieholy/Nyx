#include <stdio.h>
#include "ut_crc32.h"
#include <thread>
#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<netdb.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<errno.h>
#include "ch_json.h"
#include "ch_crypto.h"
#include "ch_tools.h"

#define MAXLINE 4096

static const char* magic_head = "magicNyxV0.1";

void print_help(char* exe){
    printf("PARAMS: %s  ", exe);
    printf("ip  ");
    printf("port  " );
    printf("jsonProto  ");
    printf("usage: %s  ", exe);
    printf("127.0.0.1  ");
    printf("8888  " );
    printf("{\"cmd\":\"ping\"}\n");
}

int sockfd;

int selectlidar( int fd, int sec, int usec)
{
	int ret;
	fd_set fds;
	struct timeval timeout;
 
	timeout.tv_sec = sec;
	timeout.tv_usec = usec;
 
	FD_ZERO(&fds); //每次循环都要清空集合，否则不能检测描述符变化
	FD_SET(fd,&fds); //添加描述符
 
	ret = select(fd+1,&fds,&fds,NULL,&timeout);
	if(0 > ret)
	{
		printf("lidar recv select error\n");
        	return -1;	
	}
	else if (ret == 0)
	{
	        printf("uart read select timeout\n");
	        return 0;
	}
    	else {
		if(FD_ISSET(fd,&fds)) //测试sock是否可读，即是否网络上有数据
		{
			return 1;
 
		}
		else
		{
			return 0;
		}
    }
}


void arr_print(Json::Value aorb)
{
    for(int i=0;i<aorb.size() && i < 5 ;i++){
        double price = JsonDoubleEx(JsonPos(JsonPos(aorb,i),0));
        double amount = JsonDoubleEx(JsonPos(JsonPos(aorb,i),1));
        printf("%d %lf %lf\n",i,price,amount);
    }
}

int packit(char send_buf[],std::string json_req){
    int len = json_req.size();
    memcpy(send_buf, magic_head, 12);
    unsigned short b16 = htobe16(len);
    memcpy(send_buf + 12,&b16,2);
    memcpy(send_buf + 14,json_req.c_str(),len);
    int32_t ch_crc32 = htobe32(generate_crc32c(send_buf,14+len));
    memcpy(send_buf+14+len,&ch_crc32,4);
    return 18+len;
}



void sentCmd(int fd,std::string cmd){
    char buf[200];
    int len = packit(buf,cmd);
    int res = write(fd,buf,len);
    printf("write cmd %d %s\n",res,cmd.c_str());
}

int num;

void ch_sendping(){
    while(true){
        num++;
        sleep(5);
        printf("send ping %d\n",num);
        std::string sping = string_format("{\"cmd\":\"ping\",\"id\":\"uuuu%d\"}",num);
        sentCmd(sockfd,sping);
        
    }
}



void ch_recv(){
    while(true){

        int ret = selectlidar(sockfd,1,0);
        if(ret <=0)
        continue;

        char buf[MAXLINE];
        char buf_all[MAXLINE];
        int all_len;
        int n = recv(sockfd, buf, MAXLINE,0);
        if(n<=0)
        {
            sleep(1);
            printf("readNull first! %d\n",n);
            continue ;
        }
        all_len = n;
        memcpy(buf_all,buf,n);
        printf("Response from server: %d %s\n",n,buf_all);

        char ch_head[13] = {0};
        memcpy(ch_head,buf_all,12);
        printf("read head %s\n",ch_head); 
        if(memcmp(ch_head,magic_head,11) != 0)
        {
            printf("err magic head!\n");
            continue ;
        }
        char *s = buf_all;
        unsigned short data_len = *(unsigned short*)(s+12);
        data_len = htobe16(data_len);
        printf("read len %d\n",data_len);

          //如果缓冲区读取的长度不够继续读
        while(all_len<data_len){
            n = read(sockfd,buf,MAXLINE);
            if(n<=0){
                printf("readNull %d\n",__LINE__);
                return ;
            }
            printf("readtail %d\n",n);
            memcpy(&buf_all[all_len],buf,n);
            all_len += n;
        }
        printf("all_len %d data_len %d\n",all_len,data_len);

        char ch_json[50000] = {0};
        int gz_res = gz_decompress(s+12+2,data_len, ch_json,50000);
        printf("read json %d %s\n",gz_res,ch_json);

        Json::Value j_res;
        JsonFromStr(ch_json,j_res);
        std::string cmd = JsonStr(j_res,"cmd");
        printf("cmd %s\n",cmd.c_str());
        if(cmd == "depth")
        {
            Json::Value data = JsonVal(j_res,"data");
            Json::Value a = JsonArr(data,"a");
            Json::Value b = JsonArr(data,"b");
            printf("asks:\n");
            arr_print(a);
            printf("bids:\n");
            arr_print(b);
        }
        
    }
}




int main(int argc, char *argv[]){

    if(argc < 3)
    {
        print_help(argv[0]);
        return -1;
    }
    char* ip = argv[1];
    char* port = argv[2];
    char* jsonProto = argv[3];
    printf("input ip [%s] port [%s] jsonProto [%s] \n\n", ip,port,jsonProto);
    struct sockaddr_in servaddr;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &servaddr.sin_addr);
    servaddr.sin_port = htons(strtoul(port, NULL, 0));
    
    Json::Value j_d;
    JsonFromStr(jsonProto,j_d);
    printf("cmd %s\n",JsonStr(j_d,"cmd").c_str());

    std::string json_req(jsonProto);
    short len = json_req.length();
    printf("len : %d\n",len);

    if( int res = connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) !=0){
        printf("conn err %d",res);
        return -1;
    }

    printf("conn\n");

    sentCmd(sockfd,json_req);



    std::thread th1(ch_recv); 
    //std::thread th2(ch_sendping);

    th1.join();
    //th2.join();
    return 0;
}