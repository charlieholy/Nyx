#pragma once
#include "ch_json.h"

class CConfig{
public:
	CConfig();
    bool loadConfig(std::string filename);
    Json::Value getConfig(){return __m_json_conf;}
    std::string toString();
private:
    Json::Value __m_json_conf;
    std::string __m_filename;
};