#pragma once
#include <iostream>  
#include <fstream>  
#include <sstream>
#include <map>
#include "ch_tools.h"

class FILE_OPRE{
public:
    FILE_OPRE(std::string filename){
        __m_file_name = std::string("property/") + filename;
    }
	void init(){
		 if(!ch_mkdir("property"))
		{
			printf(" %s failer!\n",__func__);
			return false;
		}
        std::ifstream __f_in(__m_file_name, std::ios::binary); 
		if(__f_in.is_open()){
			std::cout << " isOpen " << __m_file_name << std::endl;
		}
		else{
			std::ofstream file(__m_file_name, std::fstream::out);
			if (file) std::cout << " new file created" <<  __m_file_name << std::endl;
		}
	}

	void put(std::string value){
            //std::cout << "put " << value << std::endl;
            std::ofstream file(__m_file_name, std::ios::out);
            file << value;
	}

	std::string get(){
		std::ifstream __f_in(__m_file_name, std::ios::binary);  
	    std::ostringstream sin;	
	    sin <<  __f_in.rdbuf();	
	    std::string str = sin.str();	
        return str;
	}
private:
    std::string __m_file_name;
};


static const int f_len = 4; 
static const std::string map_files[f_len] = {"g_last_tick","g_last_depth","g_last_btc_rate","g_last_all_sym"};

class FILE_MAP{
public:
    
    void init(){
        for(int i=0;i<f_len;i++){
            std::string file_name = map_files[i];
            std::shared_ptr<FILE_OPRE> p_file_one =  std::make_shared<FILE_OPRE>(file_name);
            __m_map_all_file[map_files[i]] = p_file_one;
            __m_map_all_file[map_files[i]]->init();
        }
    }
    void put(std::string key,std::string value){
        auto it = __m_map_all_file.find(key);
        if (it != __m_map_all_file.end()){
             __m_map_all_file[key]->put(value);
        }
    }
    std::string get(std::string key){
        auto it = __m_map_all_file.find(key);
        if (it != __m_map_all_file.end()){
            return __m_map_all_file[key]->get();
        }
        return "";
        
    }
    
private:
    std::map<std::string,std::shared_ptr<FILE_OPRE>> __m_map_all_file;
};

extern FILE_MAP g_file_oper;