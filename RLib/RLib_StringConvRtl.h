/********************************************************************
	Created:	2016/07/15  13:10
	Filename: 	RLib_StringConvHelper.h
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#pragma once
/*
 *	string conversion macros 
 */
#define RA2WCL(s,l) RA2WEX<RA2WEX<>::require(l)>(s, l) // l is constexpr
#define RA2W(s)     RA2WEX<>(s)
#define RA2W_S(s,c) RA2WEX<c>(s)
#define RW2ACL(s,l) RW2AEX<RW2AEX<>::require(l)>(s, l) // l is constexpr
#define RW2A(s)     RW2AEX<>(s)
#define RW2A_S(s,c) RW2AEX<c>(s)

//-------------------------------------------------------------------------

namespace System
{
	/// <summary>
	/// Helper class used by the string conversion macros RW2A, RW2ACL and RW2AL
	/// </summary>
	template<intptr_t BUFFER_SIZE = RLIB_DEFAULT_LENGTH>
	class RW2AEX : public RT2
	{
	protected:
		char m_buffer[BUFFER_SIZE];
	public:
		RW2AEX(_In_ LPCWSTR lpwstr) {
			assert(BUFFER_SIZE >= 1);

			// perform conversion
			this->m_sizeConverted = static_cast<size_t>(RT2::UnicodeToMultiByte(this->m_buffer,
																				sizeof(this->m_buffer) - sizeof(char),
																				lpwstr,
																				wcslen(lpwstr) * sizeof(wchar_t)));
			this->m_buffer[this->m_sizeConverted / sizeof(char)] = '\0';
		}
		RW2AEX(_In_ LPCWSTR lpwstr, _In_ intptr_t sizeInChars) {
			assert(BUFFER_SIZE >= 1);

			// perform conversion
			this->m_sizeConverted = static_cast<size_t>(RT2::UnicodeToMultiByte(this->m_buffer,
																				sizeof(this->m_buffer) - sizeof(char),
																				lpwstr,
																				sizeInChars * sizeof(wchar_t)));
			this->m_buffer[this->m_sizeConverted / sizeof(char)] = '\0';
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
		/// not counting the terminating null character.
		/// </summary>
		intptr_t sizeofGBK() const {
			assert(this->m_sizeConverted >= 0);
			return static_cast<intptr_t>(this->m_sizeConverted);
		}
		/// <summary>
		/// Gets the length of string converted, in chars,
		/// not counting the terminating null character.
		/// </summary>
		RLIB_INLINE intptr_t length() const {
			assert(this->m_sizeConverted >= 0);
			return this->m_sizeConverted / sizeof(char);
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
	template<intptr_t BUFFER_SIZE = RLIB_DEFAULT_LENGTH>
	class RA2WEX : public RT2
	{
	protected:
		wchar_t m_buffer[BUFFER_SIZE];
	public:
		RA2WEX(_In_ LPCSTR lpstr) {
			assert(BUFFER_SIZE >= 1);

			// perform conversion
			this->m_sizeConverted = static_cast<size_t>(RT2::MultiByteToUnicode(this->m_buffer,
																				sizeof(this->m_buffer) - sizeof(wchar_t),
																				lpstr,
																				strlen(lpstr) * sizeof(char)));
			this->m_buffer[this->m_sizeConverted / sizeof(wchar_t)] = L'\0';
		}
		RA2WEX(_In_ LPCSTR lpstr, _In_ intptr_t sizeInChars) {
			assert(BUFFER_SIZE >= 1);

			// perform conversion
			this->m_sizeConverted = static_cast<size_t>(RT2::MultiByteToUnicode(this->m_buffer,
																				sizeof(this->m_buffer) - sizeof(wchar_t),
																				lpstr,
																				sizeInChars * sizeof(char)));
			this->m_buffer[this->m_sizeConverted / sizeof(wchar_t)] = L'\0';
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
		/// not counting the terminating null character.
		/// </summary>
		intptr_t sizeofUnicode() const {
			assert(this->m_sizeConverted >= 0);
			return this->m_sizeConverted;
		}
		/// <summary>
		/// Gets the length of string converted, in chars,
		/// not counting the terminating null character.
		/// </summary>
		RLIB_INLINE intptr_t length() const {
			assert((this->m_sizeConverted % sizeof(wchar_t)) == 0);
			return this->m_sizeConverted / sizeof(wchar_t);
		}
	public:
		static constexpr intptr_t require(_In_ intptr_t sizeInChars) {
			// sizeInBytes may include null terminator or not
			return (sizeInChars + 1) * sizeof(wchar_t);
		}
	};
}
