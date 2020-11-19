#include "ch_wsserverRpc.h"
#include <unistd.h>
#include "ut_ws_svr.h"
#include "nw_buf.h"
#include "nw_state.h"
#include "ch_log.h"
#include "ch_tools.h"
#include "ch_crypto.h"

static nw_cache *privdata_cache;

struct state_data {
    nw_ses      *ses;
    uint64_t    ses_id;
    uint64_t    request_id;
    sds         cache_key;
};

struct clt_info {
    bool        auth;
    uint32_t    user_id;
    char        *source;
};


static void on_rpc_upgrade(nw_ses *ses, const char *remote)
{   
    //LOG_INFOS(LOG_WS_SERVER,"{}",string_format("remote: %p  upgrade to websocket\n", ses));
    struct clt_info *info = ws_ses_privdata(ses);
    memset(info, 0, sizeof(struct clt_info));
}

static void on_rpc_close(nw_ses *ses, const char *remote)
{
    //LOG_INFOS(LOG_WS_SERVER,"{}",string_format("remote: %p websocket connection close\n", ses));
    gf_onclose(REQ_PROTO_WEBSOCKET,ses);
}

static int on_rpc_message(nw_ses *ses, const char *remote, const char *url, void *message, size_t size)
{
    char msg[size];
    sprintf(msg,message,size);
    std::string s_msg = msg;
    Json::Value j_res = gf_commonProto(ses,s_msg,REQ_PROTO_WEBSOCKET);
    std::string echo = Json2Str(j_res); 
    echo = gzip_base64(echo);
    ws_send_text(ses,echo.c_str());
    return size;
}

static void *on_rpc_privdata_alloc(void *svr)
{
    return nw_cache_alloc(privdata_cache);
}

static void on_rpc_privdata_free(void *svr, void *privdata)
{
    struct clt_info *info = privdata;
    if (info->source)
        free(info->source);
    nw_cache_free(privdata_cache, privdata);
}

static void on_rpc_timeout(nw_state_entry *entry)
{
    LOG_INFOS(LOG_WS_SERVER,"state id: %u timeout", entry->id);
    
}

static void on_rpc_release(nw_state_entry *entry)
{
    struct state_data *state = entry->data;
    if (state->cache_key)
        sdsfree(state->cache_key);
}


CWsServerRpc::CWsServerRpc(Json::Value conf):CRpcServer(conf,"wsserverRpc",REQ_PROTO_WEBSOCKET){}

CWsServerRpc::~CWsServerRpc(){}
bool CWsServerRpc::rpc_init(){
       ws_svr_cfg cfg;
    cfg.bind_count = 1;
    cfg.bind_arr = malloc(sizeof(nw_svr_bind));
    if (nw_sock_cfg_parse(JsonStr(_m_conf,"bind").c_str(), &cfg.bind_arr[0].addr, &cfg.bind_arr[0].sock_type) < 0){
        LOG_ALARMS(LOG_WS_SERVER);
        return false;
    }
    cfg.max_pkg_size = JsonInt(_m_conf,"max_pkg_size");
    cfg.buf_limit = 0;
    cfg.read_mem = 0;
    cfg.write_mem = 0;
    cfg.keep_alive = 120;
    cfg.protocol = JsonStr(_m_conf,"protocol").c_str();
    cfg.origin = "";

    ws_svr_type type;
    memset(&type, 0, sizeof(type));
    type.on_upgrade = on_rpc_upgrade;
    type.on_close = on_rpc_close;
    type.on_message = on_rpc_message;
    type.on_privdata_alloc = on_rpc_privdata_alloc;
    type.on_privdata_free = on_rpc_privdata_free;

    ws_svr *svr = ws_svr_create(&cfg, &type);
    if (svr == NULL)
        return -__LINE__;

    privdata_cache = nw_cache_create(sizeof(struct clt_info));
    if (privdata_cache == NULL)
        return -__LINE__;

    nw_state_type st;
    memset(&st, 0, sizeof(st));
    st.on_timeout = on_rpc_timeout;
    st.on_release = on_rpc_release;

    if(ws_svr_start(svr)<0)
    {
        return false;
    }
    printf("============init wsserverRpc!\n");
    return true;
}
int CWsServerRpc::run(){}

 void CWsServerRpc::_push2apps(_SET_SES sess,std::string data){
    if(sess.size() == 0){
        return;
    }
     std::string cryp_data = gzip_base64(data);
     int len = cryp_data.length();
     void *dat = cryp_data.c_str();
     for(auto ses:sess){
         ws_send_text_len(ses,dat,len);
     }
 }
