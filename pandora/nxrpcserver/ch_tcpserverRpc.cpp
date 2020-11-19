#include "ch_tcpserverRpc.h"
#include <unistd.h>
#include "nw_ses.h"
#include "ut_log.h"
#include "nw_svr.h"
#include "ch_log.h"
#include "ut_crc32.h"
#include "ch_crypto.h"
#include "ch_httpclient.h"
#include "ch_proto.h"
#include "ch_tools.h"
#include "ut_sds.h"
#include "ut_misc.h"
#define MAGIC_HEAD_LEN 12
#define BODY_LEN 2
#define REQ_MAX_LEN 20
#define TCP_BUFF_MAX 102400

static int on_rpc_new_connection(nw_ses *ses){
}

static int on_rpc_connection_close(nw_ses *ses){
    gf_onclose(REQ_PROTO_TCP,ses);
}

static int rpc_decode_pkg(nw_ses *ses, void *data, size_t max)
{
    if(max< REQ_MAX_LEN)
        return -1;
    if (memcmp(data, rpc_magic_head, MAGIC_HEAD_LEN) != 0)
        return -2;
    char *s = data;
    unsigned short data_len = *(unsigned short*)(s+12);
    data_len = htobe16(data_len);
    if(data_len > 1000)
    {
        return -3;
    }
    uint32_t crc_res = *(uint32_t*)(s+14+data_len);
    crc_res = be32toh(crc_res);
    uint32_t crc_calc = generate_crc32c(s,14+data_len);
    if(crc_res != crc_calc)
    {
        return -4;
    }
    int res_len = 14+data_len+4;
    return res_len;
}


static void on_rpc_recv_pkg(nw_ses *ses, void *data, size_t size)
{
    char *message = data;
    unsigned short data_len = *(unsigned short*)(message+12);
    data_len = htobe16(data_len);
    char msg[size];
    memset(msg,0,size);
    memcpy(msg,message+14,data_len);
    std::string s_msg = msg;

    Json::Value j_res = gf_commonProto(ses,s_msg,REQ_PROTO_TCP);
    std::string echo = Json2Str(j_res); 
    char send_buf[102400];
    int len = rpc_pack_proto(echo,send_buf);
    nw_ses_send(ses,send_buf,len);
}

static void on_rpc_error_msg(nw_ses *ses, const char *msg)
{
    //LOG_INFOS(LOG_TCP_SERVER,string_format("%p peer: %s error: %s",ses, nw_sock_human_addr(&ses->peer_addr), msg));
    Json::Value  j_res;
    j_res["code"] = -1;
    j_res["msg"] = msg;
    //rpc_send_tcp_pack(ses,Json2Str(j_res));
}

CTcpServerRpc::CTcpServerRpc(Json::Value conf):CRpcServer(conf,"tcpserverRpc",REQ_PROTO_TCP){}
CTcpServerRpc::~CTcpServerRpc(){}
bool CTcpServerRpc::rpc_init(){
     nw_svr_cfg  cfg;
    cfg.bind_count = 1;
    cfg.bind_arr = malloc(sizeof(nw_svr_bind));
    if (nw_sock_cfg_parse(JsonStr(_m_conf,"bind").c_str(), &cfg.bind_arr[0].addr, &cfg.bind_arr[0].sock_type) < 0){
        LOG_ALARMS(LOG_TCP_SERVER);
        return false;
    }

    cfg.max_pkg_size = JsonInt(_m_conf,"max_pkg_size");
    cfg.buf_limit = 0;
    cfg.read_mem = 0;
    cfg.write_mem = 0;

    nw_svr_type type;
    memset(&type, 0, sizeof(type));
    type.decode_pkg = rpc_decode_pkg;
    type.on_recv_pkg = on_rpc_recv_pkg;
    type.on_error_msg = on_rpc_error_msg;
    type.on_new_connection = on_rpc_new_connection;
    type.on_connection_close = on_rpc_connection_close;

    nw_svr* svr = nw_svr_create(&cfg, &type, NULL);
    if (svr == NULL)
        return false;
    if (nw_svr_start(svr) < 0)
        return false;

    printf("============init tcpserverRpc!\n");
    return true;
}
int CTcpServerRpc::run(){
    sleep(1);
    printf("im tcpserverRpc\n");
}

int rpc_pack_proto(std::string ss,char *send_buf){
    memcpy(send_buf,rpc_magic_head,MAGIC_HEAD_LEN);
    char des[TCP_BUFF_MAX];
    int len = gz_compress(ss.c_str(),ss.length(),des,10000);
    unsigned short b16 = htobe16(len);
    memcpy(send_buf + MAGIC_HEAD_LEN,&b16,2);
    memcpy(send_buf + MAGIC_HEAD_LEN + 2,des,len);
    int pac_crc32 = htobe32(generate_crc32c(send_buf, MAGIC_HEAD_LEN+2+len));
    memcpy(send_buf + MAGIC_HEAD_LEN + 2 + len ,&pac_crc32,4);
    return  MAGIC_HEAD_LEN + 2 + len +4;
}

void CTcpServerRpc::_push2apps(_SET_SES sess,std::string data){
    if(sess.size() ==0){
        return;
    }
    char send_buf[TCP_BUFF_MAX];
    int len = rpc_pack_proto(data,send_buf);
    for(auto ses:sess){
        nw_ses_send(ses,send_buf,len);
    }
}
