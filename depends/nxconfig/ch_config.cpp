#include "ch_config.h"
#include <iostream>  
#include <fstream>  
CConfig::CConfig(){}

bool CConfig::loadConfig(std::string filename){

    std::ifstream in(filename, std::ios::binary);  
    if( !in.is_open() )    
    {   
        std::cout << "Error opening file\n";   
        return false;   
    } 
    Json::Reader reader;  
    if(!reader.parse(in,__m_json_conf))
    {
        std::cout << "Json parse error\n";
        return false;
    }  
    return true;
}

std::string CConfig::toString(){
    return Json2Str(__m_json_conf);
}