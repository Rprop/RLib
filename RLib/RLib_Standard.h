/********************************************************************
	Created:	2016/08/13  13:42
	Filename: 	RLib_Standard.h
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#pragma once

#pragma warning(push)
#pragma warning(disable:4577) // 'noexcept' used with no exception handling mode specified; termination on exception is not guaranteed. Specify /EHsc
#pragma warning(disable:4548) // expression before comma has no effect
#pragma warning(disable:4514) // unreferenced inline function has been removed
#include <initializer_list>
#include <type_traits>
#include <algorithm>
#pragma warning(pop)

namespace System
{
	/// <summary>
	/// C++ standard library
	/// </summary>
	namespace Standard
	{
		template<class T>
		using initializer_list = _STD initializer_list<T>;

		using nullptr_t = _STD nullptr_t;

		template<class T> 
		inline constexpr T &&forward(typename _STD remove_reference<T>::type &_Arg) {
			// forward an lvalue as either an lvalue or an rvalue
			return static_cast<T &&>(_Arg);
		}

		template<class T> 
		inline constexpr T &&forward(typename _STD remove_reference<T>::type &&_Arg) {
			// forward an rvalue as an rvalue
			static_assert(!_STD is_lvalue_reference<T>::value, "bad forward call");
			return static_cast<T &&>(_Arg);
		}
	};
}
