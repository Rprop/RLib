/********************************************************************
	Created:	2012/01/17  21:54
	Filename: 	RLib_Fundamental.h
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#ifndef _USE_VAR
# define _USE_VAR
# include "RLib_Arithmetic.h"
# include "RLib_String.h"
# include <stdlib.h>
// https://msdn.microsoft.com/en-us/library/s3f49ktz.aspx
# define RLIB_MAX_LONG_LENGTH       RLIB_COUNTOF_STR("-2147483648")
# define RLIB_MAX_ULONG_LENGTH      RLIB_COUNTOF_STR("4294967295")
# define RLIB_MAX_LONGLONG_LENGTH   RLIB_COUNTOF_STR("-9223372036854775808")
# define RLIB_MAX_ULONGLONG_LENGTH  RLIB_COUNTOF_STR("18446744073709551615")
# define RLIB_MAX_INT_LENGTH		RLIB_MAX_LONG_LENGTH
# define RLIB_MAX_UINT_LENGTH	    RLIB_MAX_ULONG_LENGTH
# define RLIB_MAX_VAR_BIT_LENGTH    (sizeof(this->m_var) * 8)
# define RLIB_TO_VAR_PTR(t, a)      (assert(sizeof(a) == sizeof(t)), reinterpret_cast<t *>(&a)) // i cannot constexpr
# define RLIB_TO_VAR(t, a)          (*RLIB_TO_VAR_PTR(t,a))   // i cannot be constexpr
# pragma warning(push)
# pragma warning(disable:4738) // storing 32-bit float result in memory, possible loss of performance

//-------------------------------------------------------------------------

namespace System
{
	/// <summary>
	/// 表示64位有符号长整数
	/// </summary>
	class RLIB_API LongLong : public Generic::Integral<signed long long>
	{
	public:
		LongLong(signed long long i = 0) : Integral(i) {}
		LongLong(const LongLong&c) : Integral(c) {}
		RLIB_DECLARE_DYNCREATE;
	public:
		/// <summary>
		/// 将数字的字符串表示形式转换为它的等效64位有符号长整数
		/// </summary>
		static signed long long TryParse(const wchar_t *s) { return _wtoll(s); }
		/// <summary>
		/// 将数字的指定进制字符串表示形式转换为它的等效64位有符号长整数
		/// </summary>
		static signed long long TryParse(const wchar_t *s, int r, wchar_t **_end = nullptr) {
			return wcstoll(s, _end, r);
		}
		/// <summary>
		/// 将数字的字符串表示形式转换为它的等效64位有符号长整数
		/// </summary>
		static signed long long TryParse(const char *s) { return atoll(s); }
		/// <summary>
		/// 将数字的指定进制字符串表示形式转换为它的等效64位有符号长整数
		/// </summary>
		static signed long long TryParse(const char *s, int r, char **_end = nullptr) {
			return strtoll(s, _end, r);
		}
		/// <summary>
		/// 将64位有符号长整数转换为字符串形式
		/// </summary>
		String ToString() const {
			String dst(RLIB_MAX_LONGLONG_LENGTH);
			_i64tot_s(this->m_var, dst, RLIB_MAX_LONGLONG_LENGTH + 1, 10);
			return dst;
		}
		/// <summary>
		/// 将64位有符号长整数转换为指定进制字符串形式
		/// </summary>
		String ToString(int _radix) const {
			String dst(RLIB_MAX_VAR_BIT_LENGTH);
			_i64tot_s(this->m_var, dst, RLIB_MAX_VAR_BIT_LENGTH + 1, _radix);
			return dst;
		}
	};
	/// <summary>
	/// 表示64位无符号长整数
	/// </summary>
	class RLIB_API ULongLong : public Generic::Integral<unsigned long long>
	{
	public:
		ULongLong(unsigned long long i = 0) : Integral(i) {}
		ULongLong(const ULongLong&c) : Integral(c) {}
		RLIB_DECLARE_DYNCREATE;
	public:
		/// <summary>
		/// 将数字的指定进制字符串表示形式转换为它的等效64位无符号长整数
		/// </summary>
		static unsigned long long TryParse(const wchar_t *s, int r = 10, wchar_t **_end = nullptr) {
			return wcstoull(s, _end, r);
		}
		/// <summary>
		/// 将数字的指定进制字符串表示形式转换为它的等效64位无符号长整数
		/// </summary>
		static unsigned long long TryParse(const char *s, int r = 10, char **_end = nullptr) {
			return strtoull(s, _end, r);
		}
		/// <summary>
		/// 将64位无符号长整数转换为字符串形式
		/// </summary>
		String ToString() const {
			String dst(RLIB_MAX_ULONGLONG_LENGTH);
			_ui64tot_s(this->m_var, dst, RLIB_MAX_ULONGLONG_LENGTH + 1, 10);
			return dst;
		}
		/// <summary>
		/// 将64位无符号长整数转换为指定进制字符串形式
		/// </summary>
		String ToString(int _radix) const {
			String dst(RLIB_MAX_VAR_BIT_LENGTH);
			_ui64tot_s(this->m_var, dst, RLIB_MAX_VAR_BIT_LENGTH + 1, _radix);
			return dst;
		}
	};
	/// <summary>
	/// 表示64位有符号长整数
	/// </summary>
	class RLIB_API Int64 : public Generic::Integral<signed __int64>
	{
	public:
		Int64(signed __int64 i = 0) : Integral(i) {}
		Int64(const Int64&c) : Integral(c) {}
		RLIB_DECLARE_DYNCREATE;
	public:
		/// <summary>
		/// 将数字的字符串表示形式转换为它的等效64位有符号长整数
		/// </summary>
		static signed __int64 TryParse(const wchar_t *s) { return _wtoi64(s); }
		/// <summary>
		/// 将数字的指定进制字符串表示形式转换为它的等效64位有符号长整数
		/// </summary>
		static signed __int64 TryParse(const wchar_t *s, int r, wchar_t **_end = nullptr) {
			return _wcstoi64(s, _end, r);
		}
		/// <summary>
		/// 将数字的字符串表示形式转换为它的等效64位有符号长整数
		/// </summary>
		static signed __int64 TryParse(const char *s) { return _atoi64(s); }
		/// <summary>
		/// 将数字的指定进制字符串表示形式转换为它的等效64位有符号长整数
		/// </summary>
		static signed __int64 TryParse(const char *s, int r, char **_end = nullptr) {
			return _strtoi64(s, _end, r);
		}
		/// <summary>
		/// 将64位有符号长整数转换为字符串形式
		/// </summary>
		String ToString() const {
			String dst(RLIB_MAX_LONGLONG_LENGTH);
			_i64tot_s(this->m_var, dst, RLIB_MAX_LONGLONG_LENGTH + 1, 10);
			return dst;
		}
		/// <summary>
		/// 将64位有符号长整数转换为指定进制字符串形式
		/// </summary>
		String ToString(int _radix) const {
			String dst(RLIB_MAX_VAR_BIT_LENGTH);
			_i64tot_s(this->m_var, dst, RLIB_MAX_VAR_BIT_LENGTH + 1, _radix);
			return dst;
		}
	};
	/// <summary>
	/// 表示64位无符号长整数
	/// </summary>
	class RLIB_API UInt64 : public Generic::Integral<unsigned __int64>
	{
	public:
		UInt64(unsigned __int64 i = 0) : Integral(i) {}
		UInt64(const UInt64&c) : Integral(c) {}
		RLIB_DECLARE_DYNCREATE;
	public:
		/// <summary>
		/// 将数字的指定进制字符串表示形式转换为它的等效64位无符号长整数
		/// </summary>
		static unsigned __int64 TryParse(const wchar_t *s, int r = 10, wchar_t **_end = nullptr) {
			return _wcstoui64(s, _end, r);
		}
		/// <summary>
		/// 将数字的指定进制字符串表示形式转换为它的等效64位无符号长整数
		/// </summary>
		static unsigned __int64 TryParse(const char *s, int r = 10, char **_end = nullptr) {
			return _strtoui64(s, _end, r);
		}
		/// <summary>
		/// 将64位无符号长整数转换为字符串形式
		/// </summary>
		String ToString() const {
			String dst(RLIB_MAX_ULONGLONG_LENGTH);
			_ui64tot_s(this->m_var, dst, RLIB_MAX_ULONGLONG_LENGTH + 1, 10);
			return dst;
		}
		/// <summary>
		/// 将64位无符号长整数转换为指定进制字符串形式
		/// </summary>
		String ToString(int _radix) const {
			String dst(RLIB_MAX_VAR_BIT_LENGTH);
			_ui64tot_s(this->m_var, dst, RLIB_MAX_VAR_BIT_LENGTH + 1, _radix);
			return dst;
		}
	};
	/// <summary>
	/// 表示有符号长整数
	/// </summary>
	class RLIB_API Long : public Generic::Integral<signed long>
	{
	public:
		Long(signed long i = 0) : Integral(i) {}
		Long(const Long&c) : Integral(c) {}
		RLIB_DECLARE_DYNCREATE;
	public:
		/// <summary>
		/// 将数字的字符串表示形式转换为它的等效有符号整数
		/// </summary>
		static signed long TryParse(const wchar_t *s) { return _wtol(s); }
		/// <summary>
		/// 将数字的指定进制字符串表示形式转换为它的等效有符号整数
		/// </summary>
		static signed long TryParse(const wchar_t *s, int r, wchar_t **_end = nullptr) {
			return wcstol(s, _end, r);
		}
		/// <summary>
		/// 将数字的字符串表示形式转换为它的等效有符号整数
		/// </summary>
		static signed long TryParse(const char *s) { return atol(s); }
		/// <summary>
		/// 将数字的指定进制字符串表示形式转换为它的等效有符号整数
		/// </summary>
		static signed long TryParse(const char *s, int r, char **_end = nullptr) {
			return strtol(s, _end, r);
		}
		/// <summary>
		/// 将有符号数字转换为字符串形式
		/// </summary>
		String ToString() const {
			String dst(RLIB_MAX_LONG_LENGTH);
			_ltot_s(this->m_var, dst, RLIB_MAX_LONG_LENGTH + 1, 10);
			return dst;
		}
		/// <summary>
		/// 将有符号数字转换为指定进制字符串形式
		/// </summary>
		String ToString(int _radix) const {
			String dst(RLIB_MAX_VAR_BIT_LENGTH);
			_ltot_s(this->m_var, dst, RLIB_MAX_VAR_BIT_LENGTH + 1, _radix);
			return dst;
		}
	};
	/// <summary>
	/// 表示无符号长整数
	/// </summary>
	class RLIB_API ULong : public Generic::Integral<unsigned long>
	{
	public:
		ULong(unsigned long i = 0) : Integral(i) {}
		ULong(const ULong&c) : Integral(c) {}
		RLIB_DECLARE_DYNCREATE;
	public:
		/// <summary>
		/// 将数字的指定进制字符串表示形式转换为它的等效无符号整数
		/// </summary>
		static unsigned long TryParse(const wchar_t *s, int r = 10, wchar_t **_end = nullptr) {
			return wcstoul(s, _end, r);
		}
		/// <summary>
		/// 将数字的指定进制字符串表示形式转换为它的等效无符号整数
		/// </summary>
		static unsigned long TryParse(const char *s, int r = 10, char **_end = nullptr) {
			return strtoul(s, _end, r);
		}
		/// <summary>
		/// 将无符号数字转换为字符串形式
		/// </summary>
		String ToString() const {
			String dst(RLIB_MAX_ULONG_LENGTH);
			_ultot_s(this->m_var, dst, RLIB_MAX_ULONG_LENGTH + 1, 10);
			return dst;
		}
		/// <summary>
		/// 将无符号数字转换为指定进制字符串形式
		/// </summary>
		String ToString(int _radix) const {
			String dst(RLIB_MAX_VAR_BIT_LENGTH);
			_ultot_s(this->m_var, dst, RLIB_MAX_VAR_BIT_LENGTH + 1, _radix);
			return dst;
		}
	};
	/// <summary>
	/// 表示 32 位有符号整数
	/// </summary>
	class RLIB_API Int32 : public Generic::Integral<signed int>
	{
	public:
		Int32(signed int i = 0) : Integral(i) {}
		Int32(const Int32&c) : Integral(c) {}
		RLIB_DECLARE_DYNCREATE;
	public:
		/// <summary>
		/// 将数字的字符串表示形式转换为它的等效 32 位有符号整数
		/// </summary>
		static signed int TryParse(const wchar_t *s) { return _wtoi(s); }
		/// <summary>
		/// 将数字的指定进制字符串表示形式转换为它的等效 32 位有符号整数
		/// </summary>
		static signed int TryParse(const wchar_t *s, int r, wchar_t **_end = nullptr) {
			return static_cast<signed int>(wcstol(s, _end, r));
		}
		/// <summary>
		/// 将数字的字符串表示形式转换为它的等效 32 位有符号整数
		/// </summary>
		static signed int TryParse(const char *s) { return atoi(s); }
		/// <summary>
		/// 将数字的指定进制字符串表示形式转换为它的等效 32 位有符号整数
		/// </summary>
		static signed int TryParse(const char *s, int r, char **_end = nullptr) {
			return static_cast<signed int>(strtol(s, _end, r));
		}
		/// <summary>
		/// 将 32 位有符号数字转换为字符串形式
		/// </summary>
		String ToString() const {
			String dst(RLIB_MAX_INT_LENGTH);
			_itot_s(this->m_var, dst, RLIB_MAX_INT_LENGTH + 1, 10);
			return dst;
		}
		/// <summary>
		/// 将 32 位有符号数字转换为指定进制字符串形式
		/// </summary>
		String ToString(int _radix) const {
			String dst(RLIB_MAX_VAR_BIT_LENGTH);
			_itot_s(this->m_var, dst, RLIB_MAX_VAR_BIT_LENGTH + 1, _radix);
			return dst;
		}
	};
	/// <summary>
	/// 表示 32 位无符号整数
	/// </summary>
	class RLIB_API UInt32 : public ULong
	{
	public:
		UInt32(unsigned int i = 0) :ULong(i) {}
		UInt32(const UInt32&c) :ULong(c.m_var) {}
		RLIB_DECLARE_DYNCREATE;
	public:
		/// <summary>
		/// 将数字的字符串表示形式转换为它的等效 32 位无符号整数
		/// </summary>
// 		static unsigned int TryParse(const char *str)
// 		{
// 			unsigned int ret = 0;
// 			while ( isdigit( *reinterpret_cast<const unsigned char *>(str) ) ) {
// 				ret *= 10;
// 				ret += (*str++ - '0');
// 			}
// 			return ret;
// 		}
	};
	/// <summary>
	/// 表示单精度浮点值
	/// </summary>
	class RLIB_API Float : public Generic::Arithmetic<float>
	{
	public:
		Float(float i = 0) : Arithmetic(i) {}
		Float(const Float&c) : Arithmetic(c) {}
		RLIB_DECLARE_DYNCREATE;
	public:
		/// <summary>
		/// 将单精度浮点值的字符串表示形式转换为它的等效单精度浮点值
		/// </summary>
		static float TryParse(const wchar_t *s) { return static_cast<float>(_wtof(s)); }
		/// <summary>
		/// 将单精度浮点值的字符串表示形式转换为它的等效单精度浮点值
		/// </summary>
		static float TryParse(const char *s) { return static_cast<float>(atof(s)); }
		/// <summary>
		/// 将单精度浮点值的字符串表示形式转换为它的等效单精度浮点值
		/// </summary>
		static float TryParse(const wchar_t *s, wchar_t **_end) {
			return wcstof(s, _end);
		}
		/// <summary>
		/// 将单精度浮点值的字符串表示形式转换为它的等效单精度浮点值
		/// </summary>
		static float TryParse(const char *s, char **_end) {
			return strtof(s, _end);
		}
		/// <summary>
		/// 将单精度浮点值转换为字符串形式
		/// </summary>
		String ToString(LPCTSTR _Format = _T("%f"), intptr_t _Length = 16) const {
			return String(_Length).copyf(_Format, this->m_var);
		}
	};
	/// <summary>
	/// 表示双精度浮点值
	/// </summary>
	class RLIB_API Double :public Generic::Arithmetic<double>
	{
	public:
		Double(double i = 0) : Arithmetic(i) {}
		Double(const Double&c) : Arithmetic(c) {}
		RLIB_DECLARE_DYNCREATE;
	public:
		/// <summary>
		/// 将双精度浮点值的字符串表示形式转换为它的等效双精度浮点值
		/// </summary>
		static double TryParse(const wchar_t *s) { return _wtof(s); }
		/// <summary>
		/// 将双精度浮点值的字符串表示形式转换为它的等效双精度浮点值
		/// </summary>
		static double TryParse(const char *s) { return atof(s); }
		/// <summary>
		/// 将双精度浮点值的字符串表示形式转换为它的等效双精度浮点值
		/// </summary>
		static double TryParse(const wchar_t *s, wchar_t **_end) {
			return wcstod(s, _end);
		}
		/// <summary>
		/// 将双精度浮点值的字符串表示形式转换为它的等效双精度浮点值
		/// </summary>
		static double TryParse(const char *s, char **_end) {
			return strtod(s, _end);
		}
		/// <summary>
		/// 将双精度浮点值转换为字符串形式
		/// </summary>
		String ToString(LPCTSTR _Format = _T("%lf"), intptr_t _Length = 16) const {
			return String(_Length).copyf(_Format, this->m_var);
		}
	};

	typedef void Void;
	typedef bool Boolean;
	typedef unsigned char Byte;
}

#ifdef _WIN64
# define ToIntPtr(i)  ToInt64(i)
# define ToUIntPtr(i) ToUInt64(i)
#else
# define ToIntPtr(i)  ToInt32(i)
# define ToUIntPtr(i) ToUInt32(i)
#endif // _WIN64
#define ToInt32(i)    RLIB_TO_VAR(System::Int32, i)
#define ToUInt32(i)   RLIB_TO_VAR(System::UInt32, i)
#define ToInt64(i)    RLIB_TO_VAR(System::Int64, i)
#define ToUInt64(i)   RLIB_TO_VAR(System::UInt64, i)
#define ToLong(i)     RLIB_TO_VAR(System::Long, i)
#define ToULong(i)    RLIB_TO_VAR(System::ULong, i)
#define ToFloat(i)    RLIB_TO_VAR(System::Float, i)
#define ToDouble(i)   RLIB_TO_VAR(System::Double, i)

#pragma warning(pop)

#endif // _USE_VAR