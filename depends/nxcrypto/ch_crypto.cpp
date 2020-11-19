# include <string.h>
# include <openssl/bio.h>
# include <openssl/evp.h>
# include <openssl/buffer.h>
# include "ch_crypto.h"



int gz_decompress(const char *src, int srcLen, const char *dst, int dstLen){
	z_stream strm;
	strm.zalloc=NULL;
	strm.zfree=NULL;
	strm.opaque=NULL;
	 
	strm.avail_in = srcLen;
	strm.avail_out = dstLen;
	strm.next_in = (Bytef *)src;
	strm.next_out = (Bytef *)dst;
	 
	int err=-1, ret=-1;
	err = inflateInit2(&strm, MAX_WBITS+16);
	if (err == Z_OK){
	    err = inflate(&strm, Z_FINISH);
	    if (err == Z_STREAM_END){
	        ret = strm.total_out;
	    }
	    else{
	        inflateEnd(&strm);
	        return err;
	    }
	}
	else{
	    inflateEnd(&strm);
	    return err;
	}
	inflateEnd(&strm);
	return err;
}


int gz_compress(const char *src, int srcLen, char *dest, int destLen)
{
	z_stream c_stream;
	int err = 0;
	int windowBits = 15;
	int GZIP_ENCODING = 16;

	if (src && srcLen > 0)
	{
		c_stream.zalloc = (alloc_func)0;
		c_stream.zfree = (free_func)0;
		c_stream.opaque = (voidpf)0;
		if (deflateInit2(&c_stream, Z_DEFAULT_COMPRESSION, Z_DEFLATED,
			windowBits | GZIP_ENCODING, 8, Z_DEFAULT_STRATEGY) != Z_OK) return -1;
		c_stream.next_in = (Bytef *)src;
		c_stream.avail_in = srcLen;
		c_stream.next_out = (Bytef *)dest;
		c_stream.avail_out = destLen;
		while (c_stream.avail_in != 0 && c_stream.total_out < (uInt)destLen)
		{
			if (deflate(&c_stream, Z_NO_FLUSH) != Z_OK) return -1;
		}
		if (c_stream.avail_in != 0) return c_stream.avail_in;
		for (;;) {
			if ((err = deflate(&c_stream, Z_FINISH)) == Z_STREAM_END) break;
			if (err != Z_OK) return -1;
		}
		if (deflateEnd(&c_stream) != Z_OK) return -1;
		return c_stream.total_out;
	}
	return -1;
}

std::string b64_encode(const char* data,int len)
{
	BIO* b64 = BIO_new(BIO_f_base64());
	BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);

	BIO* bmem = BIO_new(BIO_s_mem());
	b64 = BIO_push(b64, bmem);

	BIO_write(b64, data, len);
	BIO_flush(b64);

	BUF_MEM* bptr = NULL;
	BIO_get_mem_ptr(b64, &bptr);

	std::string output(bptr->data, bptr->length);
	BIO_free_all(b64);

	return output;
}

std::string gzip_base64(const std::string _src)
{
    char des[102400];
    int len = gz_compress(_src.c_str(),_src.length(),des,102400);
    return b64_encode(des,len);
}

