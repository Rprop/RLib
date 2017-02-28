/********************************************************************
	Created:	2015/01/29  19:26
	Filename: 	RLib_GlobalizeString.cpp
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#include "RLib_GlobalizeString.h"
#include "RLib_StringInternal.h"
#include "RLib_Object.h"
using namespace System;
using namespace System::IO;
using namespace System::Text;

//-------------------------------------------------------------------------

void GlobalizeString::release_all()
{
	for each (auto &info in this->m_data)
	{
		if (info.isReleasable()) {
			String::Collect(info.pstr);
		} //if
	}
	memset(&this->m_data, 0, sizeof(this->m_data));
}

//-------------------------------------------------------------------------

GlobalizeString::GlobalizeString()
{
	memset(&this->m_data, 0, sizeof(this->m_data));
}

//-------------------------------------------------------------------------

GlobalizeString::GlobalizeString(const String &sourceString)
	: m_str(sourceString)
{
	memset(&this->m_data, 0, sizeof(this->m_data));
}

//-------------------------------------------------------------------------

GlobalizeString::GlobalizeString(LPCTSTR lpctstr, intptr_t length)
	: m_str(lpctstr, length)
{
	memset(&this->m_data, 0, sizeof(this->m_data));
}

//-------------------------------------------------------------------------

GlobalizeString::~GlobalizeString()
{
	this->release_all();
}

//-------------------------------------------------------------------------

GlobalizeString &GlobalizeString::operator = (const String &str)
{
	this->release_all();
	this->m_str = str;
	return *this;
}

//-------------------------------------------------------------------------

char *GlobalizeString::toGBK() const
{
	if (this->gbk.pstr == nullptr) {
#ifdef UNICODE
		this->gbk.pstr = ConvertToMultiByte(this->m_str.GetConstData(), this->m_str.Length, &this->gbk.size);
#else
		this->gbk.size = this->m_str.CanReadSize;
		this->gbk.pstr = this->m_str.c_str();
#endif // UNICODE
	} //if
	
	return this->gbk.pstr;
}

//-------------------------------------------------------------------------

wchar_t *GlobalizeString::toUnicode() const
{
	if (this->unicode.pstr == nullptr) {
#ifdef UNICODE
		this->unicode.size = this->m_str.CanReadSize;
		this->unicode.pstr = this->m_str.c_str();
#else
		this->unicode.pstr = ConvertToWideChar(this->m_str.GetConstData(), this->m_str.Length, &this->unicode.size);
#endif // UNICODE
	} //if

	return this->unicode.pstr;
}

//-------------------------------------------------------------------------

char *GlobalizeString::toUtf8() const
{
	if (this->utf8.pstr == nullptr) {
#ifdef UNICODE
		this->utf8.pstr = ConvertToMultiByte(this->m_str.GetConstData(), this->m_str.Length, &this->utf8.size, UTF8Encoding);
#else
		this->utf8.pstr = ConvertToMultiByte(this->toUnicode(), this->sizeofUnicode() / RLIB_SIZEOF(wchar_t), &this->utf8.size, UTF8Encoding);
#endif // UNICODE
	} //if

	return this->utf8.pstr;
}

//-------------------------------------------------------------------------

intptr_t GlobalizeString::sizeofGBK() const
{
	if (this->gbk.pstr == nullptr) {
		this->toGBK();
	} //if

	return this->gbk.getSize();
}

//-------------------------------------------------------------------------

intptr_t GlobalizeString::sizeofUnicode() const
{
	if (this->unicode.pstr == nullptr) {
		this->toUnicode();
	} //if

	return this->unicode.getSize();
}

//-------------------------------------------------------------------------

intptr_t GlobalizeString::sizeofUtf8() const
{
	if (this->utf8.pstr == nullptr) {
		this->toUtf8();
	} //if

	return this->utf8.getSize();
}

//-------------------------------------------------------------------------

template<typename char_t, typename src_char_t>
char_t *__string_convertion(const src_char_t *pstr,
							intptr_t length        /* = -1 */,
							intptr_t *out_size     /* = nullptr */,
							Encoding codepage /* = ASCIIEncoding */,
							size_t(__cdecl *strlen_t)(const src_char_t *),
							IO::BufferedStream *(*convert_t)(LPCVOID, intptr_t, Encoding))
{
	ManagedObject<BufferedStream> stream;

	if (length < 0) {
		length = static_cast<intptr_t>(strlen_t(pstr));
	} //if
	if (length == 0) goto __ret_empty_val;

	stream = convert_t(pstr, length * static_cast<intptr_t>(sizeof(src_char_t)), codepage);
	if (stream != nullptr) {
		auto _tstr = RLIB_INTERNAL_ALLOCATE_STRING_WITH_TYPE(stream->Length + RLIB_SIZEOF(char_t))->tryInit();
		if (_tstr != nullptr) {
			if (out_size != nullptr) *out_size = stream->Length;
			assert(stream->Position == 0);

			auto lpstr = reinterpret_cast<char_t *>(static_cast<TCHAR *>(_tstr->instanceString));
			stream->Read(lpstr, stream->Length);
			lpstr[stream->Length / sizeof(char_t)] = 0;

			return lpstr;
		} //if
	} //if

__ret_empty_val:
	if (out_size != nullptr) *out_size = 0;

	auto _tstr = RLIB_INTERNAL_ALLOCATE_STRING_WITH_TYPE(RLIB_MAX(sizeof(char_t), sizeof(wchar_t)))->tryInit();
	return reinterpret_cast<char_t *>(_tstr->tryToStringPointer());
}

//-------------------------------------------------------------------------

LPSTR GlobalizeString::ConvertToMultiByte(LPCWSTR pstr,
										  intptr_t length      /* = -1 */,
										  intptr_t *out_size   /* = nullptr */, 
										  Encoding to_codepage /* = ASCIIEncoding */)
{
	return __string_convertion<char, wchar_t>(pstr, length, out_size, 
											  to_codepage, wcslen, Encoder::WideCharTo);
}

//-------------------------------------------------------------------------

LPWSTR GlobalizeString::ConvertToWideChar(LPCSTR pstr,
			 							  intptr_t length        /* = -1 */,
										  intptr_t *out_size     /* = nullptr */,
										  Encoding from_codepage /* = ASCIIEncoding */)
{
	return __string_convertion<wchar_t, char>(pstr, length, out_size,
											  from_codepage, strlen, Encoder::ToWideChar);
}