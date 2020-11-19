#pragma once


#include <string>
#include "zlib.h"


std::string b64_encode(const char* data,int len);
int gz_decompress(const char *src, int srcLen, const char *dst, int dstLen);
int gz_compress(const char *src, int srcLen, char *dest, int destLen);
std::string gzip_base64(const std::string src);




