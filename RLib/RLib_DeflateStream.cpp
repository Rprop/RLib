/********************************************************************
Created:	2012/04/03  8:19
Filename: 	RLib_DeflateStream.cpp
Author:		rrrfff
Url:	    http://blog.csdn.net/rrrfff
Used:       zlib 1.2.6
 *********************************************************************/
#include "RLib_DeflateStream.h"
#include "support/zlib/zlib.h"
#include "support/zlib/zutil.h"

using namespace System::IO::Compression;

//-------------------------------------------------------------------------

extern RLIB_FORCE_INLINE void __rlib_initialize_z_stream(z_stream *ps);
int __rlib_compress(IN z_stream &c_stream, IN const Bytef *data, uLong ndata, 
					OUT Bytef *zdata, uLong *nzdata)
{
	int c_result       = -1;
	c_stream.next_in   = const_cast<RLIB_TYPE(c_stream.next_in)>(data);
	c_stream.avail_in  = ndata;
	c_stream.next_out  = zdata;
	c_stream.avail_out = *nzdata;
	while (c_stream.avail_in != 0/* && c_stream.avail_out > 0*/) 
	{
		// sets to Z_NO_FLUSH in order to maximize compression
		if (deflate(&c_stream, Z_NO_FLUSH) != Z_OK) {
			if (c_stream.avail_out == 0) {
				// the output buffer is full
				c_result = RLIB_COMPILE_LINE;
			} //if
			goto __end_and_return;
		} //if
	}

#ifdef _DEBUG
	if (c_stream.avail_in != 0) {
		trace(!"there might be more output pending");
	} //if
#endif // _DEBUG

__loop_start:
	switch (deflate(&c_stream, Z_FINISH)) 
	{
	case Z_OK:
		if (c_stream.avail_out == 0) {
			// the output buffer is full
			c_result = RLIB_COMPILE_LINE;
			goto __end_and_return;
		} //if
		goto __loop_start;
	case Z_STREAM_END:
		c_result = 0;
		goto __end_and_return;
	}

__end_and_return:
	*nzdata = c_stream.total_out;
	deflateEnd(&c_stream);
	return c_result;
}

//-------------------------------------------------------------------------

DeflateStream::DeflateStream(Stream *stream, CompressionMode mode,
							 CompressionLevel level /* = BestCompression */) : CompressionStream(stream, mode)
{
	this->m_gzip_header_flag = false;

	int _result;
	if (mode == CompressionMode::Decompress) {
		_result = inflateInit(reinterpret_cast<z_stream *>(&this->m_external_struct));
	} else {
		_result = deflateInit(reinterpret_cast<z_stream *>(&this->m_external_struct),
							  static_cast<int>(level));
	} //if

	assert(_result == Z_OK);
}

//-------------------------------------------------------------------------

int Deflate::Compress(IN const unsigned char *data, IN unsigned long ndata, 
					  OUT unsigned char *zdata, IN OUT unsigned long *nzdata, IN CompressionLevel level)
{
	z_stream c_stream;
	__rlib_initialize_z_stream(&c_stream);
	if (deflateInit(&c_stream, static_cast<int>(level)) != Z_OK) {
		*zdata = 0;
		return -1;
	} //if

	return __rlib_compress(c_stream, data, ndata, zdata, nzdata);
}

//-------------------------------------------------------------------------

int Deflate::Decompress(IN const unsigned char *zdata, IN unsigned long nzdata,
						OUT unsigned char *data, IN OUT unsigned long *ndata)
{
	int      d_result = -1;
	z_stream d_stream; 
	__rlib_initialize_z_stream(&d_stream);
	if (inflateInit(&d_stream) == Z_OK) {
		d_stream.next_in   = const_cast<RLIB_TYPE(d_stream.next_in)>(zdata);
		d_stream.avail_in  = nzdata;
		d_stream.avail_out = *ndata;
		d_stream.next_out  = data;
		// single step decompression
		switch (inflate(&d_stream, Z_FINISH)) 
		{
		case Z_STREAM_END:
			d_result = 0;
			break;
		case Z_BUF_ERROR:
			// no more input data or the output buffer is full
			d_result = RLIB_COMPILE_LINE;
			break;
		}		
	}
	// not all of the stream is provided or not enough output space is provided

//__end_and_return:
	*ndata = d_stream.total_out;
	inflateEnd(&d_stream);
	return d_result;
}
