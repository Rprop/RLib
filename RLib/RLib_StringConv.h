/********************************************************************
	Created:	2016/07/15  13:10
	Filename: 	RLib_StringConvHelper.h
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#pragma once
#include "RLib_Interlocked.h"
/*
 *	string conversion macros 
 */
#define RA2WCL(s,l) RA2WEX<RA2WEX<>::require(l), l + 1>(s, l) // l is constexpr
#define RA2W(s)     RA2WEX<RLIB_DEFAULT_LENGTH, 0>(s)
#define RA2W_S(s,c) RA2WEX<c, 0>(s)
#define RW2ACL(s,l) RW2AEX<RW2AEX<>::require(l), l + 1>(s, l) // l is constexpr
#define RW2A(s)     RW2AEX<RLIB_DEFAULT_LENGTH, 0>(s)
#define RW2A_S(s,c) RW2AEX<c, 0>(s)

//-------------------------------------------------------------------------

namespace System
{
	/// <summary>
	/// Helper class used by the string conversion macros RW2A, RW2ACL and RW2AL
	/// </summary>
	template<intptr_t BUFFER_SIZE = RLIB_DEFAULT_LENGTH, intptr_t TMP_BUFFER_SIZE = RLIB_DEFAULT_LENGTH>
	class RW2AEX : public RT2
	{
	protected:
		char m_buffer[BUFFER_SIZE];

	public:
		RW2AEX(_In_ LPCWSTR lpwstr) {
			errno_t ret;
			assert(BUFFER_SIZE >= 1);

			// perform conversion
			ret = _wcstombs_s_l(&this->m_charsConverted, this->m_buffer, RLIB_COUNTOF(this->m_buffer), lpwstr, _TRUNCATE, this->getlocale());
			assert(ret == 0 && "wcstombs_s failed!");
		}
		RW2AEX(_In_ LPCWSTR lpwstr, _In_ intptr_t sizeInChars) {
			errno_t ret;
			wchar_t tmp_buffer[TMP_BUFFER_SIZE];
			assert(BUFFER_SIZE >= 1);
			assert(sizeInChars >= 0);
			assert(sizeInChars <= (TMP_BUFFER_SIZE - 1) && "TRUNCATE occurred");
			assert((sizeInChars * sizeof(*lpwstr)) <= (BUFFER_SIZE - 1) && "TRUNCATE occurred");

			sizeInChars = min(sizeInChars, TMP_BUFFER_SIZE - 1);
			memcpy(tmp_buffer, lpwstr, sizeInChars * sizeof(*lpwstr));
			tmp_buffer[sizeInChars] = NULL;

			// perform conversion
			ret = _wcstombs_s_l(&this->m_charsConverted, this->m_buffer, RLIB_COUNTOF(this->m_buffer), tmp_buffer, _TRUNCATE, this->getlocale());
			assert(ret == 0 && "wcstombs_s failed!");
		}
		operator const char *() const {
			return this->m_buffer;
		}
		operator char *() {
			return this->m_buffer;
		}
		const char *toGBK() const {
			return this->m_buffer;
		}
		char *toGBK() {
			return this->m_buffer;
		}
		/// <summary>
		/// Gets the size of string converted, in bytes,
		/// not null terminator included.
		/// </summary>
		intptr_t sizeofGBK() const {
			assert(this->m_charsConverted >= 1);
			return (this->m_charsConverted - 1) * sizeof(char);
		}

	public:
		static constexpr intptr_t require(_In_ intptr_t sizeInChars) {
			// sizeInChars may include null terminator or not
			return (sizeInChars * sizeof(wchar_t) + 1) * sizeof(char);
		}
	};
	/// <summary>
	/// Helper class used by the string conversion macros RA2W, RA2WCL and RA2WL
	/// </summary>
	template<intptr_t BUFFER_SIZE = RLIB_DEFAULT_LENGTH, intptr_t TMP_BUFFER_SIZE = RLIB_DEFAULT_LENGTH>
	class RA2WEX : public RT2
	{
	protected:
		wchar_t m_buffer[BUFFER_SIZE];

	public:
		RA2WEX(_In_ LPCSTR lpstr) {
			errno_t ret;
			assert(BUFFER_SIZE >= 1);

			// perform conversion
			ret = _mbstowcs_s_l(&this->m_charsConverted, this->m_buffer, RLIB_COUNTOF(this->m_buffer), lpstr, _TRUNCATE, this->getlocale());
			assert(ret == 0 && "mbstowcs_s failed!");
		}
		RA2WEX(_In_ LPCSTR lpstr, _In_ intptr_t sizeInChars) {
			errno_t ret;
			char    tmp_buffer[TMP_BUFFER_SIZE];
			assert(BUFFER_SIZE >= 1);
			assert(sizeInChars >= 0);
			assert(sizeInChars <= (TMP_BUFFER_SIZE - 1) && "TRUNCATE occurred");
			assert((sizeInChars * sizeof(*lpstr)) <= (BUFFER_SIZE - 1) && "TRUNCATE occurred");

			sizeInChars = min(sizeInChars, TMP_BUFFER_SIZE - 1);
			memcpy(tmp_buffer, lpstr, sizeInChars * sizeof(*lpstr));
			tmp_buffer[sizeInChars] = NULL;

			// perform conversion
			ret = _mbstowcs_s_l(&this->m_charsConverted, this->m_buffer, RLIB_COUNTOF(this->m_buffer), tmp_buffer, _TRUNCATE, this->getlocale());
			assert(ret == 0 && "mbstowcs_s failed!");
		}
		operator const wchar_t *() const {
			return this->m_buffer;
		}
		operator wchar_t *() {
			return this->m_buffer;
		}
		const wchar_t *toUnicode() const {
			return this->m_buffer;
		}
		wchar_t *toUnicode() {
			return this->m_buffer;
		}
		/// <summary>
		/// Gets the size of string converted, in bytes,
		/// not null terminator included.
		/// </summary>
		intptr_t sizeofUnicode() const {
			assert(this->m_charsConverted >= 1);
			return (this->m_charsConverted - 1) * sizeof(wchar_t);
		}

	public:
		static constexpr intptr_t require(_In_ intptr_t sizeInChars) {
			// sizeInBytes may include null terminator or not
			return (sizeInChars + 1) * sizeof(wchar_t);
		}
	};
}
