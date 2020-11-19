#ifndef RESTAPI_H
#define RESTAPI_H

#include "curl/curl.h"
#include <iostream>
#include <memory>
#include <string>
#include "ch_json.h"

class CRestApi
{
  struct CURL_deleter
  {
    void operator () (CURL *);
  };

  typedef std::unique_ptr<CURL, CURL_deleter> unique_curl;
private:
  unique_curl C;
  const std::string host;

public:
  CRestApi              (std::string host);
  Json::Value getRequest   ();
};

#endif
