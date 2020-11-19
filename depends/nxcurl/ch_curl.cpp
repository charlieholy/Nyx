#include "ch_curl.h"

#include <cassert>
#include <chrono>
#include <thread> // sleep
//#include "ch_log.h"


namespace {
  
struct CurlStartup {
  CurlStartup()   { curl_global_init(CURL_GLOBAL_ALL); }
  ~CurlStartup()  { curl_global_cleanup(); }
}runCurlStartup;

// internal helpers
size_t recvCallback(void *contents, size_t size, size_t nmemb, void *userp) {
  auto &buffer = *static_cast<std::string *> (userp);
  auto n = size * nmemb;
  return buffer.append((char*)contents, n), n;
}

Json::Value doRequest(CURL *C,
                  const std::string &url) {
  std::string recvBuffer;
  curl_easy_setopt(C, CURLOPT_WRITEDATA, &recvBuffer);
                    
  curl_easy_setopt(C, CURLOPT_URL, url.c_str());
  curl_easy_setopt(C, CURLOPT_DNS_CACHE_TIMEOUT, 3600);

  goto curl_state;

retry_state:
  //LOG_INFOS(LOG_HTTP, "  Retry in 2 sec..." );
  std::this_thread::sleep_for(std::chrono::seconds(2));
  recvBuffer.clear();
  curl_easy_setopt(C, CURLOPT_DNS_CACHE_TIMEOUT, 0);

curl_state:
  CURLcode resCurl = curl_easy_perform(C);
  if (resCurl != CURLE_OK) {
    //LOG_INFOS(LOG_HTTP,"Error with cURL: {} URL {}" ,curl_easy_strerror(resCurl),url); 
    goto retry_state;
  }

  Json::Reader reader;
  Json::Value j_res;
  if (!reader.parse(recvBuffer, j_res))
  {
	  long resp_code;
	  curl_easy_getinfo(C, CURLINFO_RESPONSE_CODE, &resp_code);
  }
  return j_res;
}
}

void CRestApi::CURL_deleter::operator () (CURL *C) {
  curl_easy_cleanup(C);
}


Json::Value CRestApi::getRequest() {
  curl_easy_setopt(C.get(), CURLOPT_HTTPGET, true);
  return doRequest(C.get(), host);
}

CRestApi::CRestApi(std::string host)
    : C(curl_easy_init()), host(std::move(host)) {
  assert(C != nullptr);

  curl_easy_setopt(C.get(), CURLOPT_SSL_VERIFYPEER, false);
  curl_easy_setopt(C.get(), CURLOPT_CONNECTTIMEOUT, 10L);
  curl_easy_setopt(C.get(), CURLOPT_TIMEOUT, 20L);
  curl_easy_setopt(C.get(), CURLOPT_USERAGENT, "code of hammurabi");
  curl_easy_setopt(C.get(), CURLOPT_WRITEFUNCTION, recvCallback);
}

