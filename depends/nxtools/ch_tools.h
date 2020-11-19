#pragma once
#include <string>
#include <memory>
#include <sys/time.h>
#include <stdint.h>

template<typename ...Args>
std::string string_format(const std::string & format, Args ...args)
{
	size_t size = snprintf(nullptr, 0, format.c_str(), args ...) + 1; // Extra space for '\0'
	std::unique_ptr<char[]> buf(new char[size]);
	snprintf(buf.get(), size, format.c_str(), args ...);
	return std::string(buf.get(), buf.get() + size - 1); // We don't want the '\0' inside
}

int64_t getCurrentTime();     //直接调用这个函数就行了，返回值最好是int64_t，long long应该也可以


bool ch_mkdir(std::string dir);

int ch_getts_1m();

int ch_getts_5s();

int ch_getts_1s(); //��ȡ��ʱ���

int64_t ch_getts_1ms(); //��ȡ����ʱ���

std::string  symbol2Pair(std::string sym);  //EOSBTC -> EOS_BTC
