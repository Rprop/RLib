/********************************************************************
	Created:	2015/04/14  19:43
	Filename: 	RLib_Compression.cpp
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#include "RLib_Compression.h"
#include "support/zlib/zlib.h"
#include "support/zlib/zutil.h"

using namespace System::IO::Compression;

//-------------------------------------------------------------------------

extern "C" voidpf ZLIB_INTERNAL __rlib_zcalloc(voidpf/* opaque*/, unsigned items, unsigned size)
{
	return RLIB_GlobalAlloc(static_cast<intptr_t>(items * size));
}

//-------------------------------------------------------------------------

extern "C" void ZLIB_INTERNAL __rlib_zcfree(voidpf/* opaque*/, voidpf ptr)
{
	RLIB_GlobalCollect(ptr);
}

//-------------------------------------------------------------------------

RLIB_FORCE_INLINE void __rlib_initialize_z_stream(z_stream *ps)
{
	ps->zalloc = __rlib_zcalloc;
	ps->zfree  = __rlib_zcfree;
//	ps->opaque = nullptr;
}

//-------------------------------------------------------------------------

CompressionStream::CompressionStream(Stream *stream, CompressionMode mode)
{
	assert(sizeof(this->m_external_struct) >= sizeof(z_stream));

	this->m_mode              = mode;
	this->m_underlying_stream = stream;
	this->m_buffer            = RLIB_GlobalAllocAny(unsigned char *,
													RLIB_DEFAULT_MAX_BUFFER_SIZE);
	z_stream &t_stream        = *reinterpret_cast<z_stream *>(&this->m_external_struct);
	if (mode == CompressionMode::Decompress) {
		t_stream.avail_in     = 0;
	} else {
		t_stream.next_out     = this->m_buffer;
		t_stream.avail_out    = RLIB_DEFAULT_MAX_BUFFER_SIZE;
	} //if
	__rlib_initialize_z_stream(&t_stream);
}

//-------------------------------------------------------------------------

CompressionStream::~CompressionStream()
{
	this->Close();
}

//-------------------------------------------------------------------------

void CompressionStream::__fetch_data() const
{
	z_stream &d_stream = *reinterpret_cast<z_stream *>(&const_cast<CompressionStream *>(this)->m_external_struct);
	d_stream.next_in   = this->m_buffer;
	d_stream.avail_in  = static_cast<uInt>(this->m_underlying_stream->Read(d_stream.next_in,
										   RLIB_MIN(RLIB_DEFAULT_MAX_BUFFER_SIZE, this->m_underlying_stream->MaxReadSize)));
}

//-------------------------------------------------------------------------

intptr_t CompressionStream::Read(LPVOID buffer, intptr_t count) const
{
	assert(this->m_mode == CompressionMode::Decompress);
	z_stream &d_stream    = *reinterpret_cast<z_stream *>(&const_cast<CompressionStream *>(this)->m_external_struct);
	if (d_stream.avail_in == 0) {
		// reads more compressed data
		this->__fetch_data();
	} //if

	d_stream.next_out     = reinterpret_cast<Bytef *>(buffer);
	d_stream.avail_out    = static_cast<uInt>(count);

	// single step decompression
__loop_start:
	switch (inflate(&d_stream, Z_FINISH))
	{
	case Z_STREAM_END:
		assert(d_stream.avail_in == 0);
		if (this->CanRead && d_stream.avail_out > 0) {
			// decompress data as much as possibile
			this->__fetch_data();
			goto __loop_start;
		} //if
		return d_stream.next_out - reinterpret_cast<Bytef *>(buffer);
	case Z_BUF_ERROR:
		if (d_stream.avail_out == 0) {
			// the output buffer is full
			return d_stream.next_out - reinterpret_cast<Bytef *>(buffer);
		} //if
		if (d_stream.avail_in == 0 && this->CanRead && d_stream.avail_out > 0) {
			// decompress data as much as possibile
			this->__fetch_data();
			goto __loop_start;
		} //if
		break;
	case Z_DATA_ERROR:
		if (this->m_gzip_header_flag) {
			static Bytef dummy_head[] = { 0x8 + 0x7 * 0x10, (((0x8 + 0x7 * 0x10) * 0x100 + 30) / 31 * 31) & 0xFF };
			auto next_in      = d_stream.next_in;
			d_stream.next_in  = dummy_head;
			auto avail_in     = d_stream.avail_in;
			d_stream.avail_in = sizeof(dummy_head);
			if (inflate(&d_stream, Z_NO_FLUSH) == Z_OK) {
				d_stream.next_in  = next_in;
				d_stream.avail_in = avail_in;
				goto __loop_start;
			} //if
			trace(!"fix dummy_head");
		} //if
	}

	trace(!"unexpected exception");
	return -1;
}

//-------------------------------------------------------------------------

intptr_t CompressionStream::Write(LPCVOID data, intptr_t count)
{
	assert(this->m_mode == CompressionMode::Compress);
	z_stream &c_stream   = *reinterpret_cast<z_stream *>(&this->m_external_struct);
	c_stream.next_in     = const_cast<RLIB_TYPE(c_stream.next_in)>(reinterpret_cast<const Bytef *>(data));
	c_stream.avail_in    = static_cast<uInt>(count);
	auto &previous_nextp = *reinterpret_cast<Bytef **>(&count);
	previous_nextp       = c_stream.next_out;
	while (c_stream.avail_in != 0/* && c_stream.avail_out > 0*/)
	{
		// sets to Z_NO_FLUSH in order to maximize compression
		if (deflate(&c_stream, Z_NO_FLUSH) != Z_OK) {
			if (c_stream.avail_out != 0) {
				trace(!"unexpected exception");
				return -1;
			} //if
			// the output buffer is full, flush pending data
			this->m_underlying_stream->Write(previous_nextp, 
											 c_stream.next_out - previous_nextp);
			previous_nextp     = this->m_buffer;
			c_stream.next_out  = previous_nextp;
			c_stream.avail_out = RLIB_DEFAULT_MAX_BUFFER_SIZE;
//			continue;
		} //if
	}
	// flush pending data if any
	this->m_underlying_stream->Write(previous_nextp,
									 c_stream.next_out - previous_nextp);
	return static_cast<intptr_t>(c_stream.total_out);
}

//-------------------------------------------------------------------------

void CompressionStream::Flush()
{
	if (this->m_mode == CompressionMode::Decompress) {
		return;
	} //if

	z_stream &c_stream = *reinterpret_cast<z_stream *>(&this->m_external_struct);
#ifdef _DEBUG
	if (c_stream.avail_in != 0) {
		trace(!"there might be more output pending");
	} //if
	assert(c_stream.total_in > 0);
#endif // _DEBUG

	auto previous_next_out = c_stream.next_out;
	if (c_stream.avail_out == 0){
		previous_next_out  = this->m_buffer;
		c_stream.next_out  = previous_next_out;
		c_stream.avail_out = RLIB_DEFAULT_MAX_BUFFER_SIZE;
	} //if

__loop_start:
	switch (deflate(&c_stream, Z_FINISH)) {
	case Z_OK:
		if (c_stream.avail_out == 0) {
			//the output buffer is full, flush pending data
			this->m_underlying_stream->Write(previous_next_out,
											 c_stream.next_out - previous_next_out);
			previous_next_out = this->m_buffer;
			c_stream.next_out = previous_next_out;
			c_stream.avail_out = RLIB_DEFAULT_MAX_BUFFER_SIZE;
		} //if
		goto __loop_start;
	case Z_STREAM_END:
		break;
	default:
		trace(!"unexpected exception");
		return;
	}

	// flushs pending data to the underlying stream if any
	this->m_underlying_stream->Write(previous_next_out,
									 c_stream.next_out - previous_next_out);
	// finishs compression
	this->m_underlying_stream->Position = 0;
}

//-------------------------------------------------------------------------

void CompressionStream::Close()
{
	if (this->m_buffer != nullptr) {
		RLIB_GlobalCollect(this->m_buffer);
		this->m_buffer = nullptr;

		int _result;
		if (this->m_mode == CompressionMode::Decompress) {
			_result = inflateEnd(reinterpret_cast<z_stream *>(&this->m_external_struct));
		} else {
			_result = deflateEnd(reinterpret_cast<z_stream *>(&this->m_external_struct));
		} //if

		assert(_result == Z_OK);
	} //if
}