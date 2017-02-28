/********************************************************************
	Created:	2011/06/30  19:31
	Filename: 	RLib_String.cpp
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#include "RLib_String.h"
#include "RLib_StringInternal.h"
#include "RLib_Utility.h"
#include "RLib_GlobalizeString.h"
#include "RLib_StringHelper.h"
#include "RLib_MemoryPool.h"
#include <strsafe.h>

using namespace System::IO;
using namespace System::Text;
using namespace System::Collections::Generic;

#ifndef _UNICODE
# include "RLib_Helper.h"
# include <locale.h>
# pragma optimize("", off)
RLIB_STATIC({
	_configthreadlocale(_DISABLE_PER_THREAD_LOCALE);
	_tsetlocale(LC_ALL, _T("chs"));
});
# pragma optimize("", on)
#endif // _UNICODE

//-------------------------------------------------------------------------

TCHAR String::nullpstr[1]      = { _T('\0') };
const TCHAR *String::nullpcstr = String::nullpstr;

//-------------------------------------------------------------------------

bool String::is_releasable() const
{
	assert(this->m_size <= 0 || (this->m_size > 0 && this->m_pstr != nullptr));
	return this->m_size > 0;
}

//-------------------------------------------------------------------------

bool String::is_writable() const
{
	return this->is_releasable() && this->getInternalString()->refCount == 0;
}

//-------------------------------------------------------------------------

struct InternalString *String::getInternalString() const
{
#ifdef _DEBUG
	assert(this->m_pstr != nullptr);
	auto lpstr = InternalString::fromStringPtr(this->m_pstr);
	assert(lpstr->refCount >= 0);
	return lpstr;
#else
	return InternalString::fromStringPtr(this->m_pstr);
#endif // _DEBUG

}

//-------------------------------------------------------------------------

void String::release_local_data()
{
	assert(this->is_releasable());
	this->Collect(this->m_pstr);
}

//-------------------------------------------------------------------------

String::String()
{
	this->m_pstr = nullptr;
	this->m_len  = 0;
	this->m_size = 0;
}

//-------------------------------------------------------------------------

String::String(intptr_t length)
{
    this->m_size = TSIZE_ALIGNED(length);
	this->m_len  = 0;
	this->m_pstr = RLIB_INTERNAL_ALLOCATE_STRING_WITH_TYPE(this->m_size)
				   ->tryInit()
				   ->tryToStringPointer();
}

//-------------------------------------------------------------------------

String::String(TCHAR *lptstr, intptr_t length /* = -1 */)
{
    this->m_len  = length < 0 ? TLEN(lptstr) : length;
    this->m_size = TSIZE_ALIGNED(this->m_len);
	this->m_pstr = RLIB_INTERNAL_ALLOCATE_STRING_WITH_TYPE(this->m_size)
				   ->tryInit()
				   ->tryToStringPointer();
	
	String::FastStringCopy(this->m_pstr, lptstr, this->m_len);
	this->m_pstr[this->m_len] = _T('\0');
}

//-------------------------------------------------------------------------

String::String(const STRInfoA &si)
{
#ifndef UNICODE
	this->m_pstr = const_cast<char *>(si.lpstr);
	this->m_len  = si.length;
	this->m_size = 0;
#else 
	this->m_pstr = GlobalizeString::ConvertToWideChar(si.lpstr, si.length, &this->m_size);
	update_reference(this->m_pstr,
					 this->m_size + RLIB_SIZEOF(TCHAR),
					 this->m_size / RLIB_SIZEOF(TCHAR));
#endif 
}

//-------------------------------------------------------------------------

String::String(const char *lpstr)
{
#ifndef UNICODE
	this->m_pstr = const_cast<char *>(lpstr);
	this->m_len  = RLIB_INTERNAL_CALC_ON_NEED;
	this->m_size = 0;
#else 
	this->m_pstr = GlobalizeString::ConvertToWideChar(lpstr, -1, &this->m_size);
	update_reference(this->m_pstr,
					 this->m_size + RLIB_SIZEOF(TCHAR),
					 this->m_size / RLIB_SIZEOF(TCHAR));
#endif 
}

//-------------------------------------------------------------------------

String::String(const char *lpstr, intptr_t length)
{
#ifndef UNICODE
	RLIB_InitClass(this, String(const_cast<TCHAR *>(lpstr), length));
#else 
	this->m_pstr = GlobalizeString::ConvertToWideChar(lpstr, length, &this->m_size);
	update_reference(this->m_pstr, 
					 this->m_size + RLIB_SIZEOF(TCHAR),
					 this->m_size / RLIB_SIZEOF(TCHAR));
#endif 
}

//-------------------------------------------------------------------------

String::String(const STRInfoW &si)
{
#ifdef UNICODE
	this->m_pstr = const_cast<wchar_t *>(si.lpstr);
	this->m_len  = si.length;
	this->m_size = 0;
#else 
	this->m_pstr = GlobalizeString::ConvertToMultiByte(si.lpstr, si.length, &this->m_size);
	update_reference(this->m_pstr,
					 this->m_size + RLIB_SIZEOF(TCHAR),
					 this->m_size / RLIB_SIZEOF(TCHAR));
#endif 
}

//-------------------------------------------------------------------------

String::String(const wchar_t *lpwstr)
{
#ifdef UNICODE
	this->m_pstr = const_cast<wchar_t *>(lpwstr);
	this->m_len  = RLIB_INTERNAL_CALC_ON_NEED;
	this->m_size = 0;
#else 
	this->m_pstr = GlobalizeString::ConvertToMultiByte(lpwstr, -1, &this->m_size);
	update_reference(this->m_pstr,
					 this->m_size + RLIB_SIZEOF(TCHAR),
					 this->m_size / RLIB_SIZEOF(TCHAR));
#endif 
}

//-------------------------------------------------------------------------

String::String(const wchar_t *lpwstr, intptr_t length)
{
#ifdef UNICODE
	RLIB_InitClass(this, String(const_cast<TCHAR *>(lpwstr), length));
#else 
	this->m_pstr = GlobalizeString::ConvertToMultiByte(lpwstr, length, &this->m_size);
	update_reference(this->m_pstr, 
					 this->m_size + RLIB_SIZEOF(TCHAR),
					 this->m_size / RLIB_SIZEOF(TCHAR));
#endif 
}

//-------------------------------------------------------------------------

String::String(const String &str)
{
	assert(this != &str);

	// copy directly
	Utility::copy(str, *this);

	if (this->is_releasable()) {
		this->getInternalString()->increaseReference();
	} //if
}

//-------------------------------------------------------------------------

String::String(String &&tmpstr)
{
	assert(this != &tmpstr);

	// copy directly
	Utility::copy(tmpstr, *this);

	// release it
	tmpstr.m_size = 0;
}

//-------------------------------------------------------------------------

String::~String()
{
	if (this->is_releasable()) {
		this->release_local_data();
	} //if

#ifdef _DEBUG
	this->m_pstr = nullptr;
	this->m_len  = -1;
	this->m_size = -1;
#endif // _DEBUG
}

//-------------------------------------------------------------------------

void String::Collect(LPVOID ptr)
{
	InternalString::fromStringPtr(ptr)->decreaseReference();
}

//-------------------------------------------------------------------------

String String::Reserve(intptr_t length)
{
	assert(length >= this->Length);
	return String(length).copy(*this);
}

//-------------------------------------------------------------------------

String &String::reserve(intptr_t length, bool keep /* = true */)
{
	this->pre_allocate(length, keep);
	return *this;
}

//-------------------------------------------------------------------------

void String::pre_allocate(intptr_t length, bool copyval /* = true */)
{
	intptr_t old_length = copyval ? this->Length : 0;

    if (this->is_releasable())
    {
		intptr_t size_unaligned = TSIZE(length); // !length, in TCHARs

		if (this->m_size < size_unaligned || this->getInternalString()->refCount > 0) {
			// aligned size
			this->m_size = RLIB_ROUNDUP(size_unaligned);
			auto t_pstr  = RLIB_INTERNAL_ALLOCATE_STRING_WITH_TYPE(this->m_size)->tryInit();
			if ( t_pstr != nullptr ) {
				if (copyval) {
					size_unaligned = Utility::select_min(this->m_size - RLIB_SIZEOF(TCHAR), // leave room for null terminator
														 old_length * RLIB_SIZEOF(TCHAR));
					memcpy(t_pstr->instanceString, this->m_pstr, static_cast<size_t>(size_unaligned));
					StringTermAtBytes(t_pstr->instanceString, size_unaligned);
				} else {
					this->m_len = 0;
				} //if
			} else {
				this->m_size = 0;
				this->m_len  = 0;
			} //if
			this->release_local_data();
			this->m_pstr = t_pstr->tryToStringPointer();
		} //if
    } else {
		// forces to allocate new string
		this->m_size = TSIZE_ALIGNED(length);
		auto t_pstr  = RLIB_INTERNAL_ALLOCATE_STRING_WITH_TYPE(this->m_size)->tryInit();
		if ( t_pstr != nullptr ) {
			if (copyval) {
				auto capacity = Utility::select_min(this->m_size - sizeof(TCHAR), // leave room for null terminator
													old_length * sizeof(TCHAR));
				memcpy(t_pstr->instanceString, this->m_pstr, capacity);
				StringTermAtBytes(t_pstr->instanceString, capacity);
			} else {
				this->m_len = 0;
			} //if
			this->m_pstr = t_pstr->instanceString;
		} else {
			this->m_size = 0;
			this->m_len  = 0;
			this->m_pstr = nullptr;
		} //if
    } //if
}

//-------------------------------------------------------------------------

void String::update_reference(LPTSTR pstr, intptr_t size, intptr_t length)
{
    this->m_pstr  = pstr;
    this->m_len   = length;
    this->m_size  = size;
    assert(TSIZE(this->m_len) <= this->m_size);
}

//-------------------------------------------------------------------------

String::operator TCHAR *()
{
    return this->GetData();
}

//-------------------------------------------------------------------------

LPTSTR String::GetData()
{
	this->pre_allocate(this->Length);
    
	this->m_len = RLIB_INTERNAL_CALC_ON_NEED;
    return this->m_pstr;
}

//-------------------------------------------------------------------------

LPCTSTR String::GetConstData() const
{
	if (this->IsNull()) {
		return String::nullpcstr;
	} //if

    return this->m_pstr;
}

//-------------------------------------------------------------------------

String &String::operator = (STRNull null_t)
{
	this->~String();
	RLIB_InitClass(this, String(null_t));
	return *this;
}

//-------------------------------------------------------------------------

String &String::operator = (TCHAR *lptstr)
{
    intptr_t length = TLEN(lptstr);
	if (length == 0) {
		this->Empty();
	} else {
		this->copy(lptstr, length);
	} //if

	return *this;
}

//-------------------------------------------------------------------------

String &String::operator = (const STRInfoA &si)
{
	if (si.length != 0) {
#ifdef UNICODE
		if (this->is_releasable()) {
			this->release_local_data();
		} //if
		this->m_pstr = GlobalizeString::ConvertToWideChar(si.lpstr, si.length, &this->m_size);
		update_reference(this->m_pstr,
						 this->m_size + RLIB_SIZEOF(wchar_t),
						 this->m_size / RLIB_SIZEOF(wchar_t));
#else
		if (this->is_releasable()) {
			// first, try copy
			if (this->tryCopy(si.lpstr, si.length)) {
				return *this;
			} //if
			// otherwise, release and prepare making reference
			this->release_local_data();
		}
		// just make reference
		this->m_size = 0;
		this->m_len  = si.length;
		this->m_pstr = const_cast<char *>(si.lpstr);
#endif // UNICODE
	} else {
		this->Empty();
	} //if

	return *this;
}

//-------------------------------------------------------------------------

String &String::operator = (const char *lpstr)
{
#ifdef _UNICODE
	*this = STRInfoA { lpstr, static_cast<intptr_t>(strlen(lpstr)) } ;
#else
	intptr_t length = -1;
	if (this->is_releasable()) {
		// first, try copy
		length = static_cast<intptr_t>(strlen(lpstr));
		if (this->tryCopy(lpstr, length)) {
			return *this;
		} //if
		// otherwise, release and prepare making reference
		this->release_local_data();
	}
	// just make reference
	this->m_size = 0;
	this->m_len  = length;
	this->m_pstr = const_cast<char *>(lpstr);
#endif // _UNICODE

	return *this;
}

//-------------------------------------------------------------------------

String &String::operator = (const STRInfoW &si)
{
	if (si.length != 0) {
#ifdef UNICODE
		if (this->is_releasable()) {
			// first, try copy
			if (this->tryCopy(si.lpstr, si.length)) {
				return *this;
			} //if
			// otherwise, release and prepare making reference
			this->release_local_data();
		} //if
		
		// just make reference
		this->m_size = 0;
		this->m_len  = si.length;
		this->m_pstr = const_cast<wchar_t *>(si.lpstr);
#else
		if (this->is_releasable()) {
			this->release_local_data();
		} //if
		this->m_pstr = GlobalizeString::ConvertToMultiByte(si.lpstr, si.length, &this->m_size);
		update_reference(this->m_pstr,
						 this->m_size + RLIB_SIZEOF(char), 
						 this->m_size / RLIB_SIZEOF(char));
#endif // UNICODE
	} else {
		this->Empty();
	} //if

	return *this;
}

//-------------------------------------------------------------------------

String &String::operator = (const wchar_t *lpwstr)
{
#ifdef UNICODE
	intptr_t length = -1;
	if (this->is_releasable()) {
		// first, try copy
		length = static_cast<intptr_t>(wcslen(lpwstr));
		if (this->tryCopy(lpwstr, length)) {
			return *this;
		} // if
		// otherwise, release and prepare making reference
		this->release_local_data();
	}
	// just make reference
	this->m_size = 0;
	this->m_len  = length;
	this->m_pstr = const_cast<wchar_t *>(lpwstr);
#else
	*this = STRInfoW { lpwstr, static_cast<intptr_t>(wcslen(lpwstr)) };
#endif // UNICODE

	return *this;
}

//-------------------------------------------------------------------------

String &String::operator = (const String &str)
{
	if (this == &str) return *this;

	if (this->is_releasable()) {
		// try copy
		if (this->tryCopy(str, str.Length)) {
			return *this;
		} //if

		// discard current instance
		this->~String();
	} //if

	RLIB_InitClass(this, String(str));

	return *this;
}

//-------------------------------------------------------------------------

String &String::operator = (String &&tmpstr)
{
	assert(this != &tmpstr);

	if (tmpstr.m_size > this->m_size) {
		// force discard current instance
		this->~String();
	} else if (this->is_releasable()) {
		// try copy
		if (this->tryCopy(tmpstr, tmpstr.Length)) {
			return *this;
		} //if

		// discard current instance in this case
		this->~String();
	} //if

	// copy directly
	Utility::copy(tmpstr, *this);

	// release it
	tmpstr.m_size = 0;

	return *this;
}

//-------------------------------------------------------------------------

intptr_t String::GetSize() const
{
    return this->m_size;
}

//-------------------------------------------------------------------------

intptr_t String::GetLength() const
{
	// minus means RLIB_INTERNAL_CALC_ON_NEED
	if (this->m_len < 0 || 
		(this->is_releasable() && this->getInternalString()->refCount > 0)) {
        this->m_len = TLEN(this->m_pstr);
    } //if
    return this->m_len;
}

//-------------------------------------------------------------------------

intptr_t String::GetCanReadSize() const
{
    return TSIZE(this->Length - 1);
}

//-------------------------------------------------------------------------

TCHAR *String::c_str() const
{
	if (this->is_releasable()) {
		this->getInternalString()->increaseReference();
		return this->m_pstr;
	} //if

	if (this->IsNull()) {
		// empty string
		return RLIB_INTERNAL_ALLOCATE_STRING_WITH_TYPE(RLIB_SIZEOF(TCHAR))->tryInit()->tryToStringPointer();
	} //if

	auto tstr = RLIB_INTERNAL_ALLOCATE_STRING_WITH_TYPE(TSIZE(this->Length))->tryInit();
	if (tstr != nullptr) {
		String::FastStringCopy(tstr->instanceString, this->m_pstr, this->m_len);
		tstr->instanceString[this->m_len] = _T('\0');
		return tstr->instanceString;
	} //if

    return nullptr;
}

//-------------------------------------------------------------------------

bool String::Contains(LPCTSTR value) const
{
	if (this->IsNull()) return false;

	return !value[0] || _tcsstr(this->m_pstr, value) != nullptr;
}

//-------------------------------------------------------------------------

bool String::Contains(TCHAR c) const
{
	if (this->IsNull()) return false;

	return !c || _tcschr(this->m_pstr, c) != nullptr;
}

//-------------------------------------------------------------------------

bool String::ContainsNoCase(LPCTSTR value) const
{
	if (this->IsNull()) return false;

	if (!value[0]) return true;

	return Utility::_tcsistr(this->m_pstr, value) != nullptr;
}

//-------------------------------------------------------------------------

String &String::copy(const char *pstr, intptr_t len /* = -1 */)
{
	if (len < 0) {
		len = static_cast<intptr_t>(strlen(pstr));
	} //if
	if (len == 0) return this->Empty();

#ifndef UNICODE
	if (this->tryCopy(pstr, len)) {
		return *this;
	} //if

	this->pre_allocate(len, false);
	this->m_len = len;
	String::FastStringCopy(this->m_pstr, pstr, this->m_len);
	this->m_pstr[this->m_len] = _T('\0');
#else 
	// char to wchar_t
	intptr_t out_size = 0;
	auto t_pstr = GlobalizeString::ConvertToWideChar(pstr, len, &out_size);
	trace(t_pstr != nullptr);
	if (t_pstr != nullptr) {
		if (this->is_releasable()) {
			if (this->tryCopy(t_pstr, out_size / RLIB_SIZEOF(wchar_t))) {
				String::Collect(t_pstr);
				return *this;
			} //if
			this->release_local_data();
		} //if
		
		this->m_size = out_size + RLIB_SIZEOF(wchar_t);
		this->m_len  = out_size / RLIB_SIZEOF(wchar_t);
		this->m_pstr = t_pstr;
	} //if
#endif 
	return *this;
}

//-------------------------------------------------------------------------

String &String::copy(const wchar_t *pstr, intptr_t len /* = -1 */)
{
	if (len < 0) {
		len = static_cast<intptr_t>(wcslen(pstr));
	} //if
	if (len == 0) return this->Empty();

#ifdef UNICODE
	if (this->tryCopy(pstr, len)) {
		return *this;
	} //if

	this->pre_allocate(len, false);
	this->m_len = len;
	String::FastStringCopy(this->m_pstr, pstr, this->m_len);
	this->m_pstr[this->m_len] = _T('\0');
#else 
	// char to wchar_t
	intptr_t out_size = 0;
	auto t_pstr = GlobalizeString::ConvertToMultiByte(pstr, len, &out_size);
	assert(t_pstr != nullptr);
	if (this->is_releasable()) {
		if (this->tryCopy(t_pstr, out_size / RLIB_SIZEOF(char))) {
			String::Collect(t_pstr);
			return *this;
		} //if
		this->release_local_data();
	} //if

	this->m_size = out_size + RLIB_SIZEOF(char);
	this->m_len  = out_size / RLIB_SIZEOF(char);
	this->m_pstr = t_pstr;
#endif 
	return *this;
}

//-------------------------------------------------------------------------

String &String::copy(const String &str, intptr_t len /* = -1 */)
{
	assert(str.m_pstr != nullptr || len == 0 || str.Length == 0);
	return this->copy(str.m_pstr, len < 0 ? str.Length : len);
}

//-------------------------------------------------------------------------

bool String::tryCopy(LPCTSTR pstr, intptr_t len)
{
	if (this->is_writable()) {
		if (len == 0) {
			this->Empty();
			return true;
		} //if

		if (TSIZE(len) <= this->m_size) {
			this->m_len = len;
			String::FastStringCopy(this->m_pstr, pstr, len);
			this->m_pstr[this->m_len] = _T('\0');
			return true;
		} //if
	} //if

	return false;
}

//-------------------------------------------------------------------------

String &String::append(const TCHAR c)
{
	return this->append(&c, 1);
}

//-------------------------------------------------------------------------

String &String::append(const TCHAR *pstr, intptr_t len /* = -1 */)
{
	if (len < 0) {
		len = TLEN(pstr);
	} //if
	if (len == 0) goto __return;

	if (this->IsNullOrEmpty()) {
		this->copy(pstr, len);
		goto __return;
	} //if

	this->pre_allocate(len + this->Length); 

	assert(this->m_len >= 0);
    String::FastStringCopy(&this->m_pstr[this->m_len], pstr, len);

    this->m_len += len;
    this->m_pstr[this->m_len] = _T('\0');

__return:
	return *this;
}

//-------------------------------------------------------------------------

String &String::append(const String &str, intptr_t len /* = -1 */)
{
	return this->append(str.GetConstData(), len < 0 ? str.Length : len);
}

//-------------------------------------------------------------------------

String &__cdecl String::appendf(LPCTSTR pstrFormat, ...)
{
#if RLIB_HAS_FLOAT_SUPPORT
	float enable_float;
	enable_float = 0.0f;
#endif // RLIB_HAS_FLOAT_SUPPORT

	if (!this->is_writable()) {
		this->pre_allocate(RLIB_STRING_FORMAT_LENGTH, true);
	} //if

	auto lpstr = this->m_pstr + this->Length;
	auto nsize = this->m_size / sizeof(TCHAR) - this->m_len;

	va_list argList;
	va_start(argList, pstrFormat);
	auto rLen = _vsntprintf_s(lpstr,
							  nsize, // with null terminator
							  _TRUNCATE,
							  pstrFormat,
							  argList);
	while (rLen < 0) {
		assert(rLen == -1); // -1 indicating that truncation occurred
		rLen = _vsctprintf(pstrFormat, argList);
		if (rLen > 0) {
			this->pre_allocate(rLen + this->m_len, true);
			lpstr = this->m_pstr + this->m_len;
			nsize = this->m_size / sizeof(TCHAR) - this->m_len;
			rLen = _vsntprintf_s(lpstr,
								 nsize, // with null terminator
								 _TRUNCATE,
								 pstrFormat,
								 argList);
			if (rLen > 0) {
				break;
			} //if
		} //if

		// failed to write formatted output
		this->m_pstr[this->m_len] = 0;
		rLen = 0;
		break;
	}

	va_end(argList);
	this->m_len += rLen;

	return *this;
}

//-------------------------------------------------------------------------

String &__cdecl String::copyf(LPCTSTR pstrFormat, ...)
{
#if RLIB_HAS_FLOAT_SUPPORT
	float enable_float;
	enable_float = 0.0f;
#endif // RLIB_HAS_FLOAT_SUPPORT

	if (!this->is_writable()) {
		this->pre_allocate(RLIB_STRING_FORMAT_LENGTH, false);
	} //if

	va_list argList;
	va_start(argList, pstrFormat);
	auto rLen = _vsntprintf_s(this->m_pstr,
							  this->m_size / sizeof(TCHAR), // with null terminator
							  _TRUNCATE,
							  pstrFormat,
							  argList);
	while (rLen < 0) {
		assert(rLen == -1); // -1 indicating that truncation occurred
		rLen = _vsctprintf(pstrFormat, argList);
		if (rLen > 0) {
			this->pre_allocate(rLen, false);
			rLen = _vsntprintf_s(this->m_pstr,
								 this->m_size / sizeof(TCHAR), // with null terminator
								 _TRUNCATE,
								 pstrFormat,
								 argList);
			if (rLen > 0) {
				break;
			} //if
		} //if

		// failed to write formatted output
		this->m_pstr[0] = 0;
		rLen = 0;
		break;
	}

	va_end(argList);
	this->m_len = rLen;

	return *this;
}

//-------------------------------------------------------------------------

String operator + (const String &src, const TCHAR *pstr)
{
    intptr_t length = TLEN(pstr);
    if (length == 0) return src;

    return String(src.Length + length).copy(src).append(pstr, length);
}

//-------------------------------------------------------------------------

String operator + (const String &src, const STRInfo &si)
{
	return String(src.Length + si.length).copy(src).append(si.lpstr, si.length);
}

//-------------------------------------------------------------------------

String operator + (const String &sl, const String &sr)
{
	return String(sl.Length + sr.Length).copy(sl).append(sr);
}

//-------------------------------------------------------------------------

String &String::operator += (const TCHAR c)
{
	return this->append(&c, 1);
}

//-------------------------------------------------------------------------

String &String::operator += (const TCHAR *pstr)
{
    return this->append(pstr);
}

//-------------------------------------------------------------------------

String &String::operator += (const String &src)
{
    return this->append(src);
}

//-------------------------------------------------------------------------

intptr_t String::Compare(LPCTSTR lpsz) const
{
    return _tcscmp(this->GetConstData(), lpsz);
}

//-------------------------------------------------------------------------

intptr_t String::CompareNoCase(LPCTSTR lpsz) const
{
    return _tcsicmp(this->GetConstData(), lpsz);
}

//-------------------------------------------------------------------------

bool String::operator == (LPCTSTR str) const
{
    return this->Compare(str) == 0;
}

//-------------------------------------------------------------------------

bool String::operator != (LPCTSTR str) const
{
    return this->Compare(str) != 0;
}

//-------------------------------------------------------------------------

bool String::operator <= (LPCTSTR str) const
{
    return this->Compare(str) <= 0;
}

//-------------------------------------------------------------------------

bool String::operator < (LPCTSTR str) const
{
    return this->Compare(str) < 0;
}

//-------------------------------------------------------------------------

bool String::operator >= (LPCTSTR str) const
{
    return this->Compare(str) >= 0;
}

//-------------------------------------------------------------------------

bool String::operator > (LPCTSTR str) const
{
    return this->Compare(str) > 0;
}

//-------------------------------------------------------------------------

bool String::operator == (const String &str)const
{
	if (this->IsNull()) return str.IsNull();
	if (str.IsNull()) return false;
    return this->operator == (str.GetConstData());
}

//-------------------------------------------------------------------------

bool String::operator != (const String &str) const
{
	if (this->IsNull()) return !str.IsNull();
	if (str.IsNull()) return true;
    return this->operator != (str.GetConstData());
}

//-------------------------------------------------------------------------

bool String::operator <= (const String &str) const
{
    return this->operator <= (str.GetConstData());
}

//-------------------------------------------------------------------------

bool String::operator < (const String &str) const
{
    return this->operator < (str.GetConstData());
}

//-------------------------------------------------------------------------

bool String::operator >= (const String &str) const
{
    return this->operator >= (str.GetConstData());
}

//-------------------------------------------------------------------------

bool String::operator > (const String &str) const
{
    return this->operator > (str.GetConstData());
}

//-------------------------------------------------------------------------

const TCHAR &String::operator [] (intptr_t nIndex) const
{
    return this->get_const_char(nIndex);
}

//-------------------------------------------------------------------------

TCHAR &String::operator [] (intptr_t nIndex)
{
	return this->get_char(nIndex);
}

//-------------------------------------------------------------------------

bool String::IsEmpty() const
{
    return this->Length == 0;
}

//-------------------------------------------------------------------------

bool String::IsNull() const
{
    return this->m_pstr == nullptr;
}

//-------------------------------------------------------------------------

bool String::IsNullOrEmpty() const
{
	return this->IsNull() || this->IsEmpty();
}

//-------------------------------------------------------------------------

TCHAR String::front() const
{
	return this->m_pstr ? this->m_pstr[0] : _T('\0');
}

//-------------------------------------------------------------------------

TCHAR String::back() const
{
	return this->m_pstr && this->Length ? this->m_pstr[this->m_len - 1] : _T('\0');
}

//-------------------------------------------------------------------------

bool String::StartsWith(TCHAR c) const
{
    return this->Length > 0 && this->GetAt(0) == c;
}

//-------------------------------------------------------------------------

bool String::EndsWith(TCHAR c) const
{
    return this->Length > 0 && this->GetAt(this->Length - 1) == c;
}

//-------------------------------------------------------------------------

bool String::StartsWith(LPCTSTR pstr, intptr_t length /* = -1 */) const
{
	if (length < 0) {
		length = static_cast<intptr_t>(_tcslen(pstr));
	} //if
	if (this->Length < length) return false;
	
	while (--length >= 0) {
		if (this->m_pstr[length] != pstr[length]) return false;
	}
	return true;
}

//-------------------------------------------------------------------------

bool String::EndsWith(LPCTSTR pstr, intptr_t length /* = -1 */) const
{
	if (length < 0) {
		length = static_cast<intptr_t>(_tcslen(pstr));
	} //if
	if (this->Length < length) return false;

	auto suffixstr = this->m_pstr + this->m_len - length;
	while (--length >= 0) {
		if (suffixstr[length] != pstr[length]) return false;
	}
	return true;
}

//-------------------------------------------------------------------------

String &String::Empty()
{
    this->SetAt(0, 0);
	this->m_len = 0;
	//memset(this->m_pstr, 0, this->m_size);
	return *this;
}

//-------------------------------------------------------------------------

bool String::IsConst() const
{
	return !this->is_writable();
}

//-------------------------------------------------------------------------

const TCHAR &String::get_const_char(intptr_t index) const
{
	assert(index >= 0);
	if (this->IsNull()) {
		return String::nullpstr[0];
	} //if

	assert(index < this->Length);
    return this->m_pstr[index];
}

//-------------------------------------------------------------------------

TCHAR &String::get_char(intptr_t index)
{
	assert(index >= 0);
	if (this->IsNull()) {
		return String::nullpstr[0];
	} //if
	assert(index < this->Length);

	this->pre_allocate(this->Length);
	this->m_len = RLIB_INTERNAL_CALC_ON_NEED;

	return this->m_pstr[index];
}

//-------------------------------------------------------------------------

TCHAR String::GetAt(intptr_t nIndex) const
{
	if (this->IsNull()) {
		return String::nullpstr[0];
	} //if
    return this->m_pstr[nIndex];
}

//-------------------------------------------------------------------------

void String::SetAt(intptr_t nIndex, TCHAR ch)
{
    this->pre_allocate(nIndex + 1);
    
    this->m_pstr[nIndex] = ch;
	this->m_len          = RLIB_INTERNAL_CALC_ON_NEED;
}

//-------------------------------------------------------------------------

void String::CopyTo(TCHAR *pstr, intptr_t max_length_with_null)
{
	RLIB_RENAME(--max_length_with_null, max_length);

	if (this->Length < max_length) {
		max_length = this->m_len;
	} //if

	String::FastStringCopy(pstr, this->m_pstr, max_length);
	pstr[max_length] = _T('\0');
}

//-------------------------------------------------------------------------

intptr_t String::IndexOf(TCHAR c, intptr_t begin /* = 0 */) const
{
	if (!this->IsNullOrEmpty() && begin < this->Length) {
		TCHAR *pstr = _tcschr(this->m_pstr + begin, static_cast<unsigned int>(c));
		if (pstr != nullptr) {
			return pstr - this->m_pstr;
		} //if
	} //if
    
	return -1;
}

//-------------------------------------------------------------------------

intptr_t String::LastIndexOf(TCHAR c) const
{
    if (!this->IsNullOrEmpty()) {
		TCHAR *pstr = _tcsrchr(this->m_pstr, static_cast<unsigned int>(c));
		if (pstr != nullptr) {
			return pstr - this->m_pstr;
		} //if
    } //if
    
	return -1;
}

//-------------------------------------------------------------------------

#define _indexof(p,begin,finder) { \
		if (!this->IsNullOrEmpty()) \
		{ \
			if (begin >= this->Length) goto _return_no_found; \
			p = finder(this->m_pstr + begin, p); \
			if (p != nullptr) return (p - this->m_pstr); \
		} \
		_return_no_found: \
		return -1; \
}

//-------------------------------------------------------------------------

intptr_t String::IndexOf(const TCHAR *p, intptr_t begin /* = 0 */) const
{
	_indexof(p, begin, _tcsstr);
}

//-------------------------------------------------------------------------

intptr_t String::IndexOfNoCase(const TCHAR *p, intptr_t begin /* = 0 */) const
{
	_indexof(p, begin, Utility::_tcsistr);
}

//-------------------------------------------------------------------------

#define _last_indexof(p,target_strlen,finder) { \
		if (!this->IsNullOrEmpty()) \
		{ \
			TCHAR *p2 = this->m_pstr - target_strlen, *p3 = nullptr; \
			while ((p2 = finder(p2 + target_strlen, p)) != nullptr) p3 = p2; \
			if (p3 != nullptr) return (p3 - this->m_pstr); \
		} \
		return -1; \
}

//-------------------------------------------------------------------------

#define _last_indexof_R(p,target_strlen,finder) { \
		if (!this->IsNullOrEmpty()) \
		{ \
			TCHAR *p2 = this->m_pstr - target_strlen, *p3 = nullptr; \
			while ((p2 = finder(p2 + target_strlen, p)) != nullptr) p3 = p2; \
			if (p3 != nullptr) return (p3 - this->m_pstr) + target_strlen; \
		} \
		return -1; \
}

//-------------------------------------------------------------------------

intptr_t String::LastIndexOf(const TCHAR *p) const
{
	return this->LastIndexOfL(p, TLEN(p));
}

//-------------------------------------------------------------------------

intptr_t String::LastIndexOfL(const TCHAR *p, intptr_t len) const
{
	_last_indexof(p, len, _tcsstr);
}

//-------------------------------------------------------------------------

intptr_t String::LastIndexOfNoCase(const TCHAR *p) const
{
	return this->LastIndexOfLNoCase(p, TLEN(p));
}

//-------------------------------------------------------------------------

intptr_t String::LastIndexOfLNoCase(const TCHAR *p, intptr_t len) const
{
	_last_indexof(p, len, Utility::_tcsistr);
}

//-------------------------------------------------------------------------

intptr_t String::IndexOfR(const TCHAR *p, intptr_t begin /* = 0 */) const
{
	return this->IndexOfRL(p, TLEN(p), begin);
}

//-------------------------------------------------------------------------

intptr_t String::IndexOfRL(const TCHAR *p, intptr_t len, intptr_t begin) const
{
    intptr_t result = this->IndexOf(p, begin);
	if (result != -1) {
		return result + len;
	} //if
	return -1;
}

//-------------------------------------------------------------------------

intptr_t String::IndexOfRNoCase(const TCHAR *p, intptr_t begin /* = 0 */) const
{
	return this->IndexOfRLNoCase(p, TLEN(p), begin);
}

//-------------------------------------------------------------------------

intptr_t String::IndexOfRLNoCase(const TCHAR *p, intptr_t len, intptr_t begin) const
{
	intptr_t result = this->IndexOfNoCase(p, begin);
	if (result != -1) {
		return result + len;
	} //if
	return -1;
}

//-------------------------------------------------------------------------

intptr_t String::LastIndexOfR(const TCHAR *p) const
{
	return this->LastIndexOfRL(p, TLEN(p));
}

//-------------------------------------------------------------------------

intptr_t String::LastIndexOfRL(const TCHAR *p, intptr_t len) const
{
	_last_indexof_R(p, len, _tcsstr);
}

//-------------------------------------------------------------------------

intptr_t String::LastIndexOfRNoCase(const TCHAR *p) const
{
	return this->LastIndexOfRLNoCase(p, TLEN(p));
}

//-------------------------------------------------------------------------

intptr_t String::LastIndexOfRLNoCase(const TCHAR *p, intptr_t len) const
{
	_last_indexof_R(p, len, Utility::_tcsistr);
}

//-------------------------------------------------------------------------

#ifndef _UNICODE
RLIB_FORCE_INLINE void string_reverse(char *buffer, intptr_t length)
{
	for (intptr_t i = 0, j = length - 1 ; i < length; ++i, --j)
	{
		// process chinese characters
		if (buffer[i] & static_cast<char>(0x80U)) {
			buffer[j - 1] = buffer[i];
			buffer[j--]   = buffer[++i];
		} else {
			buffer[j] = buffer[i];
		} //if
	}
	assert(buffer[length] == '\0');
}
#endif // _UNICODE

//-------------------------------------------------------------------------

String String::Concat(const TCHAR *pstr, intptr_t len /* = -1 */) const
{
	if (len < 0) {
		len = TLEN(pstr);
	} //if
	if (len == 0) return *this;

	return String(this->Length + len).copy(this->GetConstData(), this->m_len).append(pstr, len);
}

//-------------------------------------------------------------------------

String String::Concat(const String &src, intptr_t len /* = -1 */) const
{
	return this->Concat(src.GetConstData(), len >= 0 ? len : src.Length);
}

//-------------------------------------------------------------------------

String String::Reverse() const
{
	return String(*this).reverse();
}

//-------------------------------------------------------------------------

String &String::reverse()
{
	if (!this->IsNull()) {
		this->pre_allocate(this->Length);

#ifdef UNICODE
		intptr_t n = this->Length;
		intptr_t i = (n / 2) - 1; 
		for (--n; i >= 0; --i) {
			this->m_pstr[i]     = static_cast<TCHAR>(this->m_pstr[i] ^ this->m_pstr[n - i]);
			this->m_pstr[n - i] = static_cast<TCHAR>(this->m_pstr[i] ^ this->m_pstr[n - i]);
			this->m_pstr[i]     = static_cast<TCHAR>(this->m_pstr[i] ^ this->m_pstr[n - i]);
			//Utility::swap(this->m_pstr[i], this->m_pstr[n - i]);
		}
#else
		string_reverse(this->m_pstr, this->Length);
#endif // UNICODE
	}

	return *this;
}

//-------------------------------------------------------------------------

String String::ToLower() const
{
	return String(*this).toLower();
}

//-------------------------------------------------------------------------

String &String::toLower()
{
	intptr_t length = this->Length;
	_tcslwr_s(*this, static_cast<size_t>(length) + 1);
	this->m_len = length;
	return *this;
}

//-------------------------------------------------------------------------

String String::ToUpper() const
{
	return String(*this).toUpper();
}

//-------------------------------------------------------------------------

String &String::toUpper()
{
	intptr_t length = this->Length;
	_tcsupr_s(*this, static_cast<size_t>(length) + 1);
	this->m_len = length;
	return *this;
}

//-------------------------------------------------------------------------

String String::Trim(TCHAR c /* = 0 */) const
{
	return String(*this).trim(c);
}

//-------------------------------------------------------------------------

String &String::trim(TCHAR c /* = 0 */)
{
	intptr_t firstIndex = 0;
	for (intptr_t i = 0; i < this->Length; ++i) {
		if ((c == 0 && !_istspace(static_cast<unsigned int>(this->m_pstr[i]))) ||
			(c != 0 && this->m_pstr[i] != c)) {
			break;
		} //if
		++firstIndex;
	}
	if (firstIndex >= this->Length) {
		this->Empty();
		return *this;
	} //if

	intptr_t lastIndex = this->Length;
	for (intptr_t k = this->Length - 1; k >= 0; --k) {
		if ((c == 0 && !_istspace(static_cast<unsigned int>(this->m_pstr[k]))) ||
			(c != 0 && this->m_pstr[k] != c)) {
			break;
		} //if
		--lastIndex;
	}
	assert(lastIndex > 0);
	
	// not found any
	if (firstIndex == 0 && lastIndex == this->Length) {
		return *this;
	} //if

	if (this->is_writable()) {
		this->m_len     = lastIndex - firstIndex;
		if (firstIndex != 0) {
			memmove(this->m_pstr, this->m_pstr + firstIndex, static_cast<size_t>(TSIZE(this->m_len - 1)));
		} //if	
		this->m_pstr[this->m_len] = _T('\0');
	} else {
		this->copy(this->m_pstr + firstIndex, lastIndex - firstIndex);
	} // if
	
	return *this;
}

//-------------------------------------------------------------------------

String String::TrimStart(TCHAR c /* = 0 */) const
{
	return String(*this).trimStart(c);
}

//-------------------------------------------------------------------------

String &String::trimStart(TCHAR c /* = 0 */)
{
	intptr_t firstIndex = 0;
	for (intptr_t i = 0; i < this->Length; ++i)
	{
		if ((c == 0 && !_istspace(static_cast<unsigned int>(this->m_pstr[i]))) ||
			(c != 0 && this->m_pstr[i] != c)) {
			break;
		} //if
		++firstIndex;
	}
	if (firstIndex == 0) {
		return *this;
	} //if
	if (firstIndex >= this->Length) {
		this->Empty();
		return *this;
	} //if

	if (this->is_writable()) {
		this->m_len -= firstIndex;
		memmove(this->m_pstr,
				this->m_pstr + firstIndex,
				static_cast<size_t>(TSIZE(this->m_len - 1)));
		this->m_pstr[this->m_len] = _T('\0');
	} else {
		this->copy(this->m_pstr + firstIndex, this->m_len - firstIndex);
	} // if

	return *this;
}

//-------------------------------------------------------------------------

String String::TrimEnd(TCHAR c /* = 0 */) const
{
	return String(*this).trimEnd(c);
}

//-------------------------------------------------------------------------

String &String::trimEnd(TCHAR c /* = 0 */)
{
	intptr_t lastIndex = this->Length;
	for (intptr_t i = this->Length - 1; i >= 0; --i)
	{
		if ((c == 0 && !_istspace(static_cast<unsigned int>(this->m_pstr[i]))) ||
			(c != 0 && this->m_pstr[i] != c)) {
			break;
		} //if
		--lastIndex;
	}
	if (lastIndex == this->Length) {
		return *this;
	} //if
	if (lastIndex <= 0) {
		this->Empty();
		return *this;
	} //if
	
	if (this->is_writable()) {
		this->m_len               = lastIndex - 0;
		this->m_pstr[this->m_len] = _T('\0');
	} else {
		this->copy(this->m_pstr, lastIndex - 0);
	} // if

	return *this;
}

//-------------------------------------------------------------------------

String String::PadRight(intptr_t totalWidth, TCHAR paddingChar /* = T(' ') */) const
{
	intptr_t totalSpace = totalWidth - this->Length;
	if (totalSpace <= 0) return *this;

	String pad_str(totalWidth);
	pad_str.append(*this, totalWidth - totalSpace);
	while (--totalSpace >= 0) {
		pad_str.append(&paddingChar, 1);
	}
	return pad_str;
}

//-------------------------------------------------------------------------

String &String::padRight(intptr_t totalWidth, TCHAR paddingChar /* = T(' ') */)
{
	intptr_t totalSpace = totalWidth - this->Length;
	while (--totalSpace >= 0) {
		this->append(&paddingChar, 1);
	}
	return *this;
}

//-------------------------------------------------------------------------

String String::PadLeft(intptr_t totalWidth, TCHAR paddingChar /* = T(' ') */) const
{
	intptr_t totalSpace = totalWidth - this->Length;
	if (totalSpace <= 0) return *this;

	String pad_str(totalWidth);
	while (--totalSpace >= 0) {
		pad_str.append(&paddingChar, 1);
	}
	pad_str.append(*this, totalWidth - totalSpace);
	return pad_str;
}

//-------------------------------------------------------------------------

String &String::padLeft(intptr_t totalWidth, TCHAR paddingChar /* = T(' ') */)
{
	intptr_t totalSpace = totalWidth - this->Length;
	if (totalSpace > 0) {
		if (this->is_writable() &&
			this->m_size >= TSIZE(totalWidth)) {
			// move blocks
			memmove(this->m_pstr + totalSpace, this->m_pstr,
					static_cast<size_t>(this->m_len));
			// fill chars
			while (--totalSpace >= 0) {
				this->m_pstr[totalSpace] = paddingChar;
			}
			this->m_len               = totalWidth;
			this->m_pstr[this->m_len] = _T('\0');
		} else {
			String pad_str(totalWidth);
			while (--totalSpace >= 0) {
				pad_str.append(&paddingChar, 1);
			}
			*this = pad_str.append(*this);;
		} //if
	}
	return *this;
}

//-------------------------------------------------------------------------

String String::Replace(const TCHAR *pstrFrom,
					   const TCHAR *pstrTo,
					   intptr_t begin /* = 0 */,
					   intptr_t n /* = 0 */,
					   intptr_t *replace_count /* = nullptr */) const
{
	if (pstrFrom[0] == _T('\0') ||
		begin >= this->Length ||
		(begin = this->IndexOf(pstrFrom, begin)) == -1) {
		return *this;
	} //if

	// 是否进行次数检查
	bool diff_check = (n == 0) ? false : true;
	// 计算每次需要增加的大小
	auto sF = TLEN(pstrFrom);
	auto sT = TLEN(pstrTo);
	// 输出位置
	String tstr;
	if (sF == sT) {
		// 完全复制
		tstr.copy(*this);

		TCHAR *sL = tstr.m_pstr + begin;
		TCHAR *sP = sL - sF;
		while ((sP = _tcsstr(sP + sF, pstrFrom)) != nullptr) {
			String::FastStringCopy(sP, pstrTo, sT);

			if (replace_count != nullptr) {
				++(*replace_count);
			} //if

			if (diff_check) {
				if (n == 1) break;
				--n;
			} //if
		}

		// 无须考虑 begin 偏移
		return tstr;
	} //if

	if (sF < sT) {
		intptr_t affected_length   = this->Length - begin;
		intptr_t max_replace_count = static_cast<intptr_t>(affected_length / sF) + 1;
		tstr.pre_allocate(affected_length + max_replace_count * (sT - sF) + begin, false);
	} else {
		tstr.pre_allocate(this->Length, false);
	} //if

	if (begin != 0) {
		tstr.append(this->GetConstData(), begin - 0);
	} //if

	TCHAR *sL   = this->m_pstr + begin;
	TCHAR *sP   = sL - sF;
	intptr_t sO = begin;
	while ((sP  = _tcsstr(sP + sF, pstrFrom)) != nullptr) {
		if ((sP - sL) > 0) {
			tstr.append(&this->m_pstr[sO], sP - sL).append(pstrTo, sT);
		} else {
			tstr.append(pstrTo, sT);
		} //if
		sO = (sP - this->m_pstr) + sF;
		sL = sP + sF;

		if (replace_count != nullptr) {
			++(*replace_count);
		} //if

		if (diff_check) {
			if (n == 1) break;
			--n;
		} //if
	}

	if (sO < this->m_len) {
		tstr.append(this->GetConstData() + sO, this->Length - sO);
	} //if

	return tstr;
}

//-------------------------------------------------------------------------

String String::ReplaceNoCase(const TCHAR *pstrFrom, 
							 const TCHAR *pstrTo, 
							 intptr_t begin /* = 0 */, 
							 intptr_t n /* = 0 */, 
							 intptr_t *replace_count /* = nullptr */) const
{
	if (pstrFrom[0] == _T('\0') ||
		begin >= this->Length ||
		(begin = this->IndexOfNoCase(pstrFrom, begin)) == -1) {
		return *this;
	} //if

	// 是否进行次数检查
	bool diff_check = (n == 0) ? false : true;
	// 计算每次需要增加的大小
	auto sF = TLEN(pstrFrom);
	auto sT = TLEN(pstrTo);
	// 输出位置
	String tstr;
	if (sF == sT) {
		// 完全复制
		tstr.copy(*this);

		TCHAR *sL = tstr.m_pstr + begin;
		TCHAR *sP = sL - sF;
		while ((sP = Utility::_tcsistr(sP + sF, pstrFrom)) != nullptr) {
			String::FastStringCopy(sP, pstrTo, sT);

			if (replace_count != nullptr) {
				++(*replace_count);
			} //if

			if (diff_check) {
				if (n == 1) break;
				--n;
			} //if
		}

		// 无须考虑 begin 偏移
		return tstr;
	} //if

	if (sF < sT) {
		intptr_t affected_length   = this->Length - begin;
		intptr_t max_replace_count = static_cast<intptr_t>(affected_length / sF) + 1;
		tstr.pre_allocate(affected_length + max_replace_count * (sT - sF) + begin, false);
	} else {
		tstr.pre_allocate(this->Length + begin, false);
	} //if

	if (begin != 0) {
		tstr.append(this->GetConstData(), begin - 0);
	} //if

	TCHAR *sL = this->m_pstr + begin;
	TCHAR *sP = sL - sF;
	intptr_t sO = begin;
	while ((sP = Utility::_tcsistr(sP + sF, pstrFrom)) != nullptr) {
		if ((sP - sL) > 0) {
			tstr.append(&this->m_pstr[sO], sP - sL).append(pstrTo, sT);
		} else {
			tstr.append(pstrTo, sT);
		} //if
		sO = (sP - this->m_pstr) + sF;
		sL = sP + sF;

		if (replace_count != nullptr) {
			++(*replace_count);
		} //if

		if (diff_check) {
			if (n == 1) break;
			--n;
		} //if
	}

	if (sO < this->m_len) {
		tstr.append(this->GetConstData() + sO, this->Length - sO);
	} //if

	return tstr;
}

//-------------------------------------------------------------------------

String &String::replace(const TCHAR *pstrFrom,
						const TCHAR *pstrTo,
						intptr_t begin /* = 0 */,
						intptr_t n /* = 0 */,
						intptr_t *replace_count /* = nullptr */)
{
	if (pstrFrom[0] == _T('\0') ||
		begin >= this->Length ||
		(begin = this->IndexOf(pstrFrom, begin)) == -1) {
		return *this;
	} //if

	// 是否进行次数检查
	bool diff_check = (n == 0) ? false : true;
	// 计算每次需要增加的大小
	auto sF = TLEN(pstrFrom);
	auto sT = TLEN(pstrTo);
	if (sF == sT) {
		this->pre_allocate(this->Length);

		TCHAR *sL  = this->m_pstr + begin;
		TCHAR *sP  = sL - sF;
		while ((sP = _tcsstr(sP + sF, pstrFrom)) != nullptr) {
			String::FastStringCopy(sP, pstrTo, sT);

			if (replace_count != nullptr) {
				++(*replace_count);
			} //if

			if (diff_check) {
				if (n == 1) break;
				--n;
			} //if
		}

		// 无须考虑 begin 偏移
		return *this;
	} //if

	RLIB_DELAY(String, tstr);
	if (sF < sT) {
		intptr_t affected_length   = this->Length - begin;
		intptr_t max_replace_count = static_cast<intptr_t>(affected_length / sF) + 1;
		RLIB_InitClass(&tstr, String(affected_length + max_replace_count * (sT - sF) + begin));
	} else {
		RLIB_InitClass(&tstr, String(this->Length));
	} //if

	if (begin != 0) {
		tstr.append(this->GetConstData(), begin - 0);
	} //if

	TCHAR *sL = this->m_pstr + begin;
	TCHAR *sP = sL - sF;
	intptr_t sO = begin;
	while ((sP = _tcsstr(sP + sF, pstrFrom)) != nullptr) {
		if ((sP - sL) > 0) {
			tstr.append(&this->m_pstr[sO], sP - sL).append(pstrTo, sT);
		} else {
			tstr.append(pstrTo, sT);
		}
		sO = (sP - this->m_pstr) + sF;
		sL = sP + sF;

		if (replace_count != nullptr) {
			++(*replace_count);
		} //if

		if (diff_check) {
			if (n == 1) break;
			--n;
		} //if
	}

	if (sO < this->m_len) {
		tstr.append(this->GetConstData() + sO, this->Length - sO);
	} //if

	*this = tstr;
	RLIB_DELAY_DESTROY(String, &tstr);

	return *this;
}

//-------------------------------------------------------------------------

String &String::replaceNoCase(const TCHAR *pstrFrom,
							  const TCHAR *pstrTo,
							  intptr_t begin /* = 0 */,
							  intptr_t n /* = 0 */,
							  intptr_t *replace_count /* = nullptr */)
{
	if (pstrFrom[0] == _T('\0') ||
		begin >= this->Length ||
		(begin = this->IndexOfNoCase(pstrFrom, begin)) == -1) {
		return *this;
	} //if

	// 是否进行次数检查
	bool diff_check = (n == 0) ? false : true;
	// 计算每次需要增加的大小
	auto sF = TLEN(pstrFrom);
	auto sT = TLEN(pstrTo);
	if (sF == sT) {
		this->pre_allocate(this->Length);

		TCHAR *sL = this->m_pstr + begin;
		TCHAR *sP = sL - sF;
		while ((sP = Utility::_tcsistr(sP + sF, pstrFrom)) != nullptr) {
			String::FastStringCopy(sP, pstrTo, sT);

			if (replace_count != nullptr) {
				++(*replace_count);
			} //if

			if (diff_check) {
				if (n == 1) break;
				--n;
			} //if
		}

		// 无须考虑 begin 偏移
		return *this;
	} //if

	RLIB_DELAY(String, tstr);
	if (sF < sT) {
		intptr_t affected_length   = this->Length - begin;
		intptr_t max_replace_count = static_cast<intptr_t>(affected_length / sF) + 1;
		RLIB_InitClass(&tstr, String(affected_length + max_replace_count * (sT - sF) + begin));
	} else {
		RLIB_InitClass(&tstr, String(this->Length));
	} //if

	if (begin != 0) {
		tstr.append(this->GetConstData(), begin - 0);
	} //if

	TCHAR *sL = this->m_pstr + begin;
	TCHAR *sP = sL - sF;
	intptr_t sO = begin;
	while ((sP = Utility::_tcsistr(sP + sF, pstrFrom)) != nullptr) {
		if ((sP - sL) > 0) {
			tstr.append(&this->m_pstr[sO], sP - sL).append(pstrTo, sT);
		} else {
			tstr.append(pstrTo, sT);
		}
		sO = (sP - this->m_pstr) + sF;
		sL = sP + sF;

		if (replace_count != nullptr) {
			++(*replace_count);
		} //if

		if (diff_check) {
			if (n == 1) break;
			--n;
		} //if
	}

	if (sO < this->m_len) {
		tstr.append(this->GetConstData() + sO, this->Length - sO);
	} //if

	*this = tstr;
	RLIB_DELAY_DESTROY(String, &tstr);

	return *this;
}

//-------------------------------------------------------------------------

String String::Substring(intptr_t nIndex, intptr_t nLen /* = -1*/) const
{
	if (nIndex >= this->Length || nLen == 0) {
		return Nothing;
	} //if

	if (nLen > (this->m_len - nIndex) || nLen < 0) {
		nLen = this->m_len - nIndex;
	} //if

	return String(nLen).copy(&this->m_pstr[nIndex], nLen);
}

//-------------------------------------------------------------------------

String &String::substring(intptr_t nIndex, intptr_t nLen /* = -1*/)
{
	if (nIndex > this->Length || nLen == 0) {
		return *this;
	} //if

	this->pre_allocate(this->Length, true);
	
	if (nIndex != this->m_len) {
		if (nLen > (this->m_len - nIndex) || nLen < 0) {
			nLen = this->m_len - nIndex;
		} //if

		if (nIndex > 0) {
			memmove(this->m_pstr, &this->m_pstr[nIndex], nLen * sizeof(TCHAR));
		} //if
	} else {
		assert(nLen <= 0);
		nLen = 0;
	} //if

	this->m_pstr[nLen] = _T('\0');
	this->m_len        = nLen;
	return *this;
}

//-------------------------------------------------------------------------

intptr_t String::CountOf(const TCHAR *p1, intptr_t begin /* = 0*/) const
{
	intptr_t count = 0, len = TLEN(p1);
	while((begin = this->IndexOf(p1, begin)) != -1)
	{
		begin += len;
		++count;
	}
	return count;
}

//-------------------------------------------------------------------------

typedef intptr_t(String::*__IndexOf)(const TCHAR *p, intptr_t begin) const;
typedef const TCHAR *__FindOf(const TCHAR *p1, const TCHAR *p2);

//-------------------------------------------------------------------------

static String __match(const TCHAR *p1, const TCHAR *p2, intptr_t begin_offset,
					  const String *__this, __IndexOf fr, __IndexOf f)
{
	intptr_t begin = (__this->*fr)(p1, begin_offset);
	if (begin != -1) {
		intptr_t end = (__this->*f)(p2, begin);
		if (end != -1) return __this->Substring(begin, end - begin);
	} //if
	return Nothing;
}

//-------------------------------------------------------------------------

String String::Match(const TCHAR *p1, const TCHAR *p2, intptr_t begin_offset /* = 0 */) const
{
	return __match(p1, p2, begin_offset, this, &String::IndexOfR, &String::IndexOf);
}

//-------------------------------------------------------------------------

String String::MatchNoCase(const TCHAR *p1, const TCHAR *p2, intptr_t begin_offset /* = 0 */) const
{
	return __match(p1, p2, begin_offset, this, &String::IndexOfRNoCase, &String::IndexOfNoCase);
}

//-------------------------------------------------------------------------

static String __match_replace(const TCHAR *p1, const TCHAR *p2, const TCHAR *replaceTo, intptr_t begin,
							  const String *__this, __IndexOf f, __IndexOf fr)
{
	begin = (__this->*f)(p1, begin);
	if (begin == -1) return *__this;

	intptr_t end = (__this->*fr)(p2, begin + TLEN(p1));
	if (end == -1) return *__this;

	if (replaceTo[0] != _T('\0')) {
		intptr_t len = TLEN(replaceTo);
		return String(begin + len + (__this->Length - end))
			.append(__this->GetConstData(), begin - 0)
			.append(replaceTo, len)
			.append(__this->GetConstData() + end, (__this->Length - end));
	} else {
		return String(begin + (__this->Length - end))
			.append(__this->GetConstData(), begin - 0)
			.append(__this->GetConstData() + end, (__this->Length - end));
	} //if	
}

//-------------------------------------------------------------------------

String String::MatchReplace(const TCHAR *p1, const TCHAR *p2,
							const TCHAR *replaceTo, intptr_t begin /* = 0 */) const
{
	return __match_replace(p1, p2, replaceTo, begin, this, &String::IndexOf, &String::IndexOfR);
}

//-------------------------------------------------------------------------

String String::MatchReplaceNoCase(const TCHAR *p1, const TCHAR *p2,
								  const TCHAR *replaceTo, intptr_t begin /* = 0 */) const
{
	return __match_replace(p1, p2, replaceTo, begin, this, &String::IndexOfNoCase, &String::IndexOfRNoCase);
}

//-------------------------------------------------------------------------

static String __match_replace_callback(const TCHAR *p1, const TCHAR *p2, String::MatchCallback callback, intptr_t begin,
									   const String *__this, __FindOf f)
{
	if (begin > __this->Length || begin < 0) return *__this;

	bool matchany = false;
	auto lpdata   = __this->GetConstData() + begin;
	intptr_t len1 = TLEN(p1);
	intptr_t len2 = TLEN(p2);
	RLIB_DELAY(String, szBuffer);
	String r;
	do 
	{
		auto lpbegin = f(lpdata, p1);
		if (lpbegin == nullptr) break;

		auto lpend = f(lpbegin + len1, p2);
		if (lpend == nullptr) break;

		r = callback(lpbegin + len1, lpend);

		if (!matchany) {
			matchany = true;
			RLIB_InitClass(&szBuffer, String(__this->Length));
			szBuffer.copy(lpdata, lpbegin - lpdata);		
		} else {
			szBuffer.append(lpdata, lpbegin - lpdata);
		} //if	

		szBuffer.append(r);
		lpdata = lpend + len2;
	} while (*lpdata != _T('\0'));
	
	if (matchany) {
		szBuffer.append(lpdata, TLEN(lpdata));
		r = szBuffer;
		szBuffer.~String();
		return r;
	} //if

	return *__this;
}

//-------------------------------------------------------------------------

String String::MatchReplace(const TCHAR *p1, const TCHAR *p2, 
							MatchCallback callback, intptr_t begin /* = 0 */) const
{
	return __match_replace_callback(p1, p2, callback, begin, this, _tcsstr);
}

//-------------------------------------------------------------------------

String String::MatchReplaceNoCase(const TCHAR *p1, const TCHAR *p2, 
								  MatchCallback callback, intptr_t begin /* = 0 */) const
{
	return __match_replace_callback(p1, p2, callback, begin, this, Utility::_tcsistr);
}

//-------------------------------------------------------------------------

Array<LPTSTR> *String::FastSplit(const TCHAR *strSeparator, intptr_t lengthOfSeparator,
								 intptr_t averageItemLength)
{
	auto index_list = new Array<LPTSTR>(RLIB_ROUNDUP(this->Length / averageItemLength));
	if (index_list != nullptr) {
		if (!this->IsConst()) this->pre_allocate(this->Length, true);
//		auto length   = this->Length;
		auto lpsz     = const_cast<LPTSTR>(this->GetConstData());
		auto lpstr    = lpsz;
		auto last_ptr = lpsz;
		while ((lpstr = _tcsstr(last_ptr, const_cast<LPTSTR>(strSeparator))) != nullptr) {
			index_list->Add(last_ptr);
			lpstr[0] = _T('\0');
			last_ptr = (lpstr + lengthOfSeparator);
			if (index_list->Length == index_list->MaxLength) {
				index_list->InitStorage(index_list->Length + static_cast<intptr_t>(index_list->Length / 2));
			} //if
		}
		index_list->Add(last_ptr);
	} //if
	return index_list;
}

//-------------------------------------------------------------------------

void String::FastStringCopy(TCHAR *RLIB_RESTRICT dmem, const TCHAR *RLIB_RESTRICT smem, intptr_t charCount)
{
	assert(charCount >= 0);
	memcpy(dmem, smem, static_cast<size_t>(TSIZE(charCount - 1)));
}