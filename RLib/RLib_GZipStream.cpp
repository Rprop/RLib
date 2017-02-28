/********************************************************************
Created:	2012/04/03  8:19
Filename: 	RLib_GZipStream.cpp
Author:		rrrfff
Url:	    http://blog.csdn.net/rrrfff
Used:       zlib 1.2.6
 *********************************************************************/
#include "RLib_GZipStream.h"
#include "support/zlib/zlib.h"
#include "support/zlib/zutil.h"

using namespace System::IO;
using namespace System::IO::Compression;

//-------------------------------------------------------------------------

extern RLIB_FORCE_INLINE void __rlib_initialize_z_stream(z_stream *ps);
extern int __rlib_compress(IN z_stream &c_stream, IN const Bytef *data,
						   uLong ndata, OUT Bytef *zdata, uLong *nzdata);

//-------------------------------------------------------------------------

GZipStream::GZipStream(Stream *stream, CompressionMode mode,
					   CompressionLevel level /* = BestCompression */, 
					   int windowBits /* = -RLIB_MAX_WBITS */) : CompressionStream(stream, mode)
{
	this->m_gzip_header_flag = true;

	int _result;
	if (mode == CompressionMode::Decompress) {
		_result = inflateInit2(reinterpret_cast<z_stream *>(&this->m_external_struct),
							   windowBits);
	} else {
		_result = deflateInit2(reinterpret_cast<z_stream *>(&this->m_external_struct),
							   static_cast<int>(level), Z_DEFLATED,
							   windowBits, 8, Z_DEFAULT_STRATEGY);
	} //if
	
	assert(_result == Z_OK);
}

//-------------------------------------------------------------------------

int GZip::Compress(IN const unsigned char *data, IN unsigned long ndata, 
				   OUT unsigned char *zdata, IN OUT unsigned long *nzdata, 
				   IN CompressionLevel level, IN OPTIONAL int windowBits /* = -RLIB_MAX_WBITS */)
{
	z_stream c_stream;
	__rlib_initialize_z_stream(&c_stream);
	if (deflateInit2(&c_stream, static_cast<int>(level), Z_DEFLATED, windowBits, 8, Z_DEFAULT_STRATEGY) != Z_OK) {
		*nzdata = 0;
		return -1;
	} //if

	return __rlib_compress(c_stream, data, ndata, zdata, nzdata);
}

//-------------------------------------------------------------------------

int GZip::Decompress(IN const unsigned char *zdata, IN unsigned long nzdata, 
					 OUT unsigned char *data, IN OUT unsigned long *ndata, 
					 IN OUT int windowBits /* = -RLIB_MAX_WBITS */)
{
	static Bytef dummy_head[] = { 0x8 + 0x7 * 0x10, (((0x8 + 0x7 * 0x10) * 0x100 + 30) / 31 * 31) & 0xFF };

	z_stream d_stream;
	__rlib_initialize_z_stream(&d_stream);
	if (inflateInit2(&d_stream, windowBits) != Z_OK) {
		*ndata = 0;
		return -1;
	} //if
	
	int d_result       = -1;
	d_stream.next_in   = const_cast<RLIB_TYPE(d_stream.next_in)>(zdata);
	d_stream.avail_in  = nzdata;
	d_stream.next_out  = data;
	d_stream.avail_out = *ndata;

	// single step decompression
__inflate_start:
	switch (inflate(&d_stream, Z_FINISH))
	{
	case Z_STREAM_END:
		d_result = 0;
		break;
	case Z_BUF_ERROR:
		// no more input data or the output buffer is full	
		d_result = RLIB_COMPILE_LINE;
		break;
	case Z_DATA_ERROR:
		d_stream.next_in      = dummy_head;
		d_stream.avail_in     = sizeof(dummy_head);			
		if (inflate(&d_stream, Z_NO_FLUSH) == Z_OK) {
			d_stream.next_in  = const_cast<RLIB_TYPE(d_stream.next_in)>(zdata);
			d_stream.avail_in = nzdata;
			goto __inflate_start;
		} //if
		#pragma todo("fix dummy_head")
	}

//__end_and_return:
	*ndata = d_stream.total_out;
	inflateEnd(&d_stream);
	return d_result;
}