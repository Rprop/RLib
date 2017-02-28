/********************************************************************
	Created:	2014/12/28  12:57
	Filename: 	RLib_Utility.h
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#ifndef _USE_UTILITY
#define _USE_UTILITY
#include "RLib_AppBase.h"

#define RLIB_ENABLE_SHELLAPI 0
#if RLIB_ENABLE_SHELLAPI
# include <shlwapi.h>
# pragma comment(lib, "shlwapi.lib")
#endif // RLIB_ENABLE_SHELLAPI

namespace System
{
	namespace Utility
	{
		/// <summary>
		/// Copies values from a to b
		/// </summary>
		template <class T> void copy(const T &a, T &b)
		{
			memcpy(&b, &a, sizeof(T));
		}
		/// <summary>
		/// Exchanges values of two objects
		/// </summary>
		template <class T> void swap(T &a, T &b) 
		{
			unsigned char data[sizeof(T)];
			memcpy(data, &a, sizeof(T));
			memcpy(&a, &b, sizeof(T));
			memcpy(&b, data, sizeof(T));
		}
		/// <summary>
		/// Exchanges values of two objects arrays
		/// </summary>
		template <class t, size_t N> void swap(t(&a)[N], t(&b)[N])
		{
			for (size_t i = 0; i < N; ++i) swap(a[i], b[i]);
		}
		/// <summary>
		/// Returns a numeric value, rounded to the specified length or precision
		/// </summary>
		template <typename _INTEGER> constexpr _INTEGER round_up(_INTEGER _val, const _INTEGER _align)
		{
			return _val < _align ? _align : (_val % _align != 0 ? static_cast<_INTEGER>((_val / _align) + 1) * _align : _val);
		}
		
		//-------------------------------------------------------------------------
		
		template <typename _INTEGER> constexpr _INTEGER round_up_4(_INTEGER _val)
		{
			return _val <= 4 ? 4 : ((_val & 3) != 0 ? static_cast<_INTEGER>(_val & 0xFFFFFFFC) + 4 : _val);
		}
		
		//-------------------------------------------------------------------------

		template <typename _INTEGER> constexpr _INTEGER round_up_8(_INTEGER _val)
		{
			return _val <= 8 ? 8 : ((_val & 7) != 0 ? static_cast<_INTEGER>(_val & 0xFFFFFFF8) + 8 : _val);
		}
		
		//-------------------------------------------------------------------------
		
		template <typename _INTEGER> /* constexpr */ _INTEGER round_up_pow_2(_INTEGER i) 
		{
			// If input is a power of two, shift its high-order bit right.
			// "Smear" the high-order bit all the way to the right.
			return 1 + (i | ((i |= (i |= (i |= (i |= (--i) >> 1) >> 2) >> 4) >> 8) >> 16));
		}
		
		//-------------------------------------------------------------------------
		
		template <typename _INTEGER> /* constexpr */ _INTEGER floor_to_pow_2(_INTEGER i)
		{
			i = i | (i >> 1);
			i = i | (i >> 2);
			i = i | (i >> 4);
			i = i | (i >> 8);
			i = i | (i >> 16);
			return i - (i >> 1);
		}

		//-------------------------------------------------------------------------

		template<typename string_type> string_type stristr(string_type p1, const char *p2)
		{
#if RLIB_ENABLE_SHELLAPI
			return StrStrIA(p1, p2);
#else
			if (!*p2) return p1;

			string_type s1;
			const char *s2;

			while (*p1) {
				s1 = p1;
				s2 = p2;

				while (*s1 && *s2 && !(toupper(*s1) - toupper(*s2))) {
					++s1;
					++s2;
				}

				if (!*s2) return p1;

				++p1;
			}

			return nullptr;
#endif // RLIB_ENABLE_SHELLAPI		
		}

		//-------------------------------------------------------------------------

		template<typename string_type> string_type wcsistr(string_type p1, const wchar_t *p2)
		{
#if RLIB_ENABLE_SHELLAPI
			return StrStrIW(p1, p2);
#else
			if (!*p2) return p1;

			string_type    s1;
			const wchar_t *s2;

			while (*p1) {
				s1 = p1;
				s2 = p2;

				while (*s1 && *s2 && !(toupper(*s1) - toupper(*s2))) {
					++s1;
					++s2;
				}

				if (!*s2) return p1;

				++p1;
			}

			return nullptr;
#endif // RLIB_ENABLE_SHELLAPI
		}
		
		//-------------------------------------------------------------------------

		template<typename string_type> string_type _tcsistr(string_type p1, const TCHAR *p2)
		{
#ifdef _UNICODE
			return wcsistr(p1, p2);
#else
			return stristr(p1, p2);
#endif // _UNICODE
		}
		
		//-------------------------------------------------------------------------
		
		template<typename data_type, typename size_type = intptr_t>
		data_type memmem(data_type haystack, size_type haystack_len,
						 const void *needle, size_type needle_len)
		{
			/*
			 * The first occurrence of the empty string is deemed to occur at
			 * the beginning of the string.
			*/
			if (needle_len == 0)
				return haystack;

			/*
			 * Sanity check, otherwise the loop might search through the whole
			 * memory.
			*/
			if (haystack_len >= needle_len) {
				const char *begin         = reinterpret_cast<const char *>(haystack);
				const char *last_possible = begin + haystack_len - needle_len;
				const char *tail          = reinterpret_cast<const char *>(needle);
				char point                = *tail++;
				for (; begin <= last_possible; ++begin) {
					if (*begin == point && !memcmp(begin + 1, tail, static_cast<size_t>(needle_len - 1)))
						return static_cast<data_type>(const_cast<void *>(reinterpret_cast<const void *>(begin)));
				}
			}

			return nullptr;
		}
		
		//-------------------------------------------------------------------------
		
		template<typename string_type> string_type memstr(string_type p1, intptr_t s1, const void *p2, intptr_t s2)
		{
			return Utility::memmem<string_type, intptr_t>(p1, s1, p2, s2);
		}
		
		//-------------------------------------------------------------------------

		static constexpr LPCTSTR compiler_tcschr(_In_z_ const TCHAR *_String, _In_ const TCHAR _Ch) 
		{
			return _String[0] != _Ch && _String[0] != _T('\0') ?
				compiler_tcschr(_String + 1, _Ch) : (_String[0] != _T('\0') ? _String : nullptr);
		}
		
		//-------------------------------------------------------------------------
		
		static constexpr LPCTSTR compiler_tcsrchr(_In_z_ const TCHAR *_String, _In_ const TCHAR _Ch)
		{
			return _String[0] != _Ch && _String[0] != _T('\0') ?
				compiler_tcsrchr(_String + 1, _Ch) : (_String[0] != _T('\0') ? (compiler_tcsrchr(_String + 1, _Ch) == nullptr ? _String : compiler_tcsrchr(_String + 1, _Ch)) : nullptr);
		}
		
		//-------------------------------------------------------------------------
		
		template <typename T = intptr_t> constexpr T select_max(const T &a, const T &b)
		{
			return RLIB_MAX(a, b);
		}
		
		//-------------------------------------------------------------------------
		
		template <typename T = intptr_t, typename ... Args> constexpr T select_max(const T &a, const T &b, const Args& ... args)
		{
			return select_max(RLIB_MAX(a, b), args...);
		}
		
		//-------------------------------------------------------------------------
		
		template <typename T = intptr_t> constexpr T select_min(const T &a, const T &b)
		{
			return RLIB_MIN(a, b);
		}

		//-------------------------------------------------------------------------

		template <typename T = intptr_t, typename ... Args> constexpr T select_min(const T &a, const T &b, const Args& ... args)
		{
			return select_min(RLIB_MIN(a, b), args...);
		}
	}
}

#endif // _USE_UTILITY