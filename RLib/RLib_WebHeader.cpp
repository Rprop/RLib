/********************************************************************
	Created:	2016/07/28  9:47
	Filename: 	RLib_WebHeader.cpp
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#include "RLib_WebHeader.h"
#include "RLib_StringHelper.h"

using namespace System::IO;
using namespace System::Net;

//-------------------------------------------------------------------------

void WebHeaderCollection::Add(LPCSTR name, LPCSTR val)
{
	this->Add(name, static_cast<intptr_t>(strlen(name)), val, static_cast<intptr_t>(strlen(val)));
}

//-------------------------------------------------------------------------

void WebHeaderCollection::Add(LPCSTR name, intptr_t len_name, LPCSTR val, intptr_t len_val)
{
	if (this->m_headers.Length != 0) {
		this->m_headers.Write(RLIB_STR_LEN("\r\n"));
	} //if
	this->m_headers.Write(name, len_name);
	this->m_headers.Write(RLIB_STR_LEN(": "));
	this->m_headers.Write(val, len_val);
}

//-------------------------------------------------------------------------

String WebHeaderCollection::Get(LPCSTR name)
{
	return this->Get(name, static_cast<intptr_t>(strlen(name)));
}

//-------------------------------------------------------------------------

String WebHeaderCollection::Get(LPCSTR name, intptr_t len_name)
{
	if (this->m_headers.Length != 0)
	{
		char *lpheaders = this->ToByteArray();
		assert(lpheaders != nullptr);
		char *lplimiter = lpheaders + this->GetByteArraySize();

		char *lpstr = lpheaders;
		while ((lpstr = Utility::stristr(lpstr, name)) != nullptr) {
			// back check
			if ((lpstr == lpheaders || *(lpstr - 1) == '\n') &&
				StringStartWith_2_A(lpstr + len_name, ": ")) 
			{
				lpstr += (len_name + RLIB_COUNTOF_STR(": "));
				auto lpend = strchr(lpstr, '\n');
				if (lpend != nullptr){
					if (*(lpend - 1) == '\r') --lpend;
				} else {
					lpend = lplimiter;
				} //if

				intptr_t vlen = lpend - lpstr;
				return String(vlen).copy(lpstr, vlen);
			} //if

			lpstr += len_name;
		}
	}
    return Nothing;
}

//-------------------------------------------------------------------------

void WebHeaderCollection::Clear()
{
    this->m_headers.Length = 0;
}

//-------------------------------------------------------------------------

char *WebHeaderCollection::ToByteArray()
{
	assert(this->m_headers.Position == this->m_headers.Length);
	this->m_headers.Write("\0", sizeof(char));
	--this->m_headers.Length;

    return reinterpret_cast<char *>(this->m_headers.ObjectData);
}

//-------------------------------------------------------------------------

void WebHeaderCollection::WriteByteArray(char *pByteArray, intptr_t size)
{
	if (this->m_headers.Length != 0) {
		this->m_headers.Write(RLIB_STR_LEN("\r\n"));
	} //if
	while (size > 0 && 
		   (pByteArray[size - 1] == '\n' || pByteArray[size - 1] == '\r')) {
		--size;
	}
    this->m_headers.Write(pByteArray, size);
}

//-------------------------------------------------------------------------

intptr_t WebHeaderCollection::GetCount()
{
	if (this->m_headers.Length == 0){
		return 0;
	} //if

	intptr_t count = 1;
	auto lpstr = this->ToByteArray();
	while ((lpstr = strstr(lpstr, "\r\n")) != nullptr) {
		++count;
		lpstr += RLIB_COUNTOF_STR("\r\n");
	}
	return count;
}