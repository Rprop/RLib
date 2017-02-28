/********************************************************************
	Created:	2014/06/30  19:34
	Filename: 	RLib_StringArray.cpp
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#include "RLib_StringHelper.h"
#include "RLib_StringArray.h"

//-------------------------------------------------------------------------

String StringArray::Join(const String &separator)
{
	if (this->Length == 0) {
		return String::nullpcstr;
	} //if

	const intptr_t length_separator = separator.Length;
	if (length_separator == 0) {
		// no separtor
		intptr_t len, alignment = sizeof(__int64);
		String str(this->Length * alignment);
		for (intptr_t x = 0; x < this->Length; ++x)
		{
			len = this->m_pItems[x].Length;
			if (len > alignment) {
				str.reserve(this->Length * (alignment *= 2));
			} //if
			str.append(this->m_pItems[x], len);
		}
		return str;
	} //if

	// allocate array used for storing LPCTSTR
	auto ptrList = RLIB_GlobalAllocAny(LPCTSTR *, this->Length * sizeof(LPCTSTR));
	if (ptrList == nullptr) {
		return String::nullpcstr;
	} //if

	// assign each LPCTSTR to array and calc the total length	
	intptr_t length = length_separator * this->Length;
	for (intptr_t i = 0; i < this->Length; ++i) 
	{
		ptrList[i] = this->m_pItems[i]; 
		length    += this->m_pItems[i].Length;      
	}

	String str(length - separator.Length);
	LPTSTR pDest = str.GetData();
	intptr_t k = 0;
	for (; k < this->Length - 1; ++k)
	{
		// Note that String::GetLength() has been called,
		// so we use String::m_len directly
		length = this->m_pItems[k].m_len;
		String::FastStringCopy(pDest, ptrList[k], length);
		String::FastStringCopy(pDest + length, separator, length_separator);
		pDest += length + length_separator;
	}

	// copy the last one
	String::FastStringCopy(pDest, ptrList[k], this->m_pItems[k].m_len);
	pDest[this->m_pItems[k].m_len] = _T('\0');

	RLIB_GlobalCollect(ptrList);

	return str;
}

//-------------------------------------------------------------------------

typedef intptr_t(String::*__IndexOf)(const TCHAR *p, intptr_t begin) const;

//-------------------------------------------------------------------------

static StringArray *__match_all(const TCHAR *p1, const TCHAR *p2, intptr_t begin_offset,
								const String *__this, __IndexOf f)
{
	StringArray *p = new StringArray();
	if (p != nullptr) {
		intptr_t len  = TLEN(p1);
		intptr_t len2 = TLEN(p2);
		while ((begin_offset = (__this->*f)(p1, begin_offset)) != -1) {
			begin_offset += len;
			intptr_t end_offset = (__this->*f)(p2, begin_offset);
			if (end_offset == -1) break;

			p->Add(__this->Substring(begin_offset, end_offset - begin_offset));
			begin_offset = end_offset + len2;
		}
	} //if
	return p;
}

//-------------------------------------------------------------------------

StringArray *String::MatchAll(const TCHAR *p1, const TCHAR *p2,
							  intptr_t begin_offset /* = 0 */) const
{
	return __match_all(p1, p2, begin_offset, this, &String::IndexOf);
}

//-------------------------------------------------------------------------

StringArray *String::MatchAllNoCase(const TCHAR *p1, const TCHAR *p2,
									intptr_t begin_offset /* = 0 */) const
{
	return __match_all(p1, p2, begin_offset, this, &String::IndexOfNoCase);
}

//-------------------------------------------------------------------------

StringArray *String::Split(const String &strSeparator, bool removeEmptyEntries /* = false */) const
{
	return this->Split(strSeparator, strSeparator.Length, 16, removeEmptyEntries);
}

//-------------------------------------------------------------------------

StringArray *String::Split(const TCHAR *strSeparator, intptr_t lengthOfSeparator, 
						   intptr_t averageItemLength, bool removeEmptyEntries /* = false */) const
{
	assert(lengthOfSeparator > 0);
	if (this->IsNull()) {
		return nullptr;
	} //if

	LPTSTR findPtr      = nullptr;
	LPTSTR lastFindPtr  = this->m_pstr;
	auto resultArray    = new StringArray(RLIB_ROUNDUP(this->Length / averageItemLength));
	if (resultArray != nullptr) {
		while ((findPtr = _tcsstr(lastFindPtr, strSeparator)) != nullptr)
		{
			if ((findPtr - lastFindPtr) != 0) {
				resultArray->Add(String(findPtr - lastFindPtr).copy(lastFindPtr, findPtr - lastFindPtr));
			} else if (!removeEmptyEntries) {
				resultArray->Add(String::nullpstr);
			} //if
			lastFindPtr += (findPtr - lastFindPtr) + lengthOfSeparator;
		}
		// 没有找到分割项或者达到结尾
		if (lastFindPtr == this->m_pstr) {
			// 一项也没有
			trace(resultArray->Length == 0);
			resultArray->Add(*this);
		} else {
			// 添加最后那项
			auto lastPartLength = this->Length - (lastFindPtr - this->m_pstr);
			resultArray->Add(String(lastPartLength).copy(lastFindPtr, lastPartLength));
		} //if
	} //if
	return resultArray;
}
