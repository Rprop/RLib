/********************************************************************
	Created:	2015/02/01  20:05
	Filename: 	RLib_Interlocked.h
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#pragma once

#define __RLIB_INTERLOCKED_API_UNARY(a, t)  template<typename T> RLIB_FORCE_INLINE static T __stdcall a(volatile T *location){ assert((reinterpret_cast<intptr_t>(location) % sizeof(*location)) == 0); return static_cast<T>(Interlocked##a(reinterpret_cast<volatile t *>(location))); }
#define __RLIB_INTERLOCKED_API_BINARY(a)    template<typename T> RLIB_FORCE_INLINE static T __stdcall a(volatile T *location, const T value){ assert((reinterpret_cast<intptr_t>(location) % sizeof(*location)) == 0); return static_cast<T>(Interlocked##a(location, value)); }
#define __RLIB_INTERLOCKED_API_TEMARY(a, t) template<typename T> RLIB_FORCE_INLINE static T __stdcall a(volatile T *location, const T value, const T comparand){ assert((reinterpret_cast<intptr_t>(location) % sizeof(*location)) == 0); return static_cast<T>(Interlocked##a(reinterpret_cast<volatile t *>(location), value, comparand)); }

//-------------------------------------------------------------------------

namespace System
{
	namespace Threading
	{
		/// <summary>
		/// 为多个线程共享的变量提供原子操作
		/// </summary>
		class Interlocked
		{
		public:
			/// <summary>
			/// 以原子操作的形式递增指定变量的值并存储结果, 并且不设置内存屏障
			/// </summary>
			/// <param name="location">其值要递增的变量, 必须 32-bit 对齐</param>
			/// <returns>递增后的值</returns>
			__RLIB_INTERLOCKED_API_UNARY(IncrementNoFence, long);
			/// <summary>
			/// 以原子操作的形式递增指定变量的值并存储结果
			/// </summary>
			/// <param name="location">其值要递增的变量, 必须 32-bit 对齐</param>
			/// <returns>递增后的值</returns>
			__RLIB_INTERLOCKED_API_UNARY(Increment, long);
			/// <summary>
			/// 以原子操作的形式递增指定变量的值并存储结果
			/// </summary>
			/// <param name="location">其值要递增的变量, 必须 16-bit 对齐</param>
			/// <returns>递增后的值</returns>
			__RLIB_INTERLOCKED_API_UNARY(Increment16, short);
			/// <summary>
			/// 以原子操作的形式递增指定变量的值并存储结果
			/// </summary>
			/// <param name="location">其值要递增的变量, 必须 64-bit 对齐</param>
			/// <returns>递增后的值</returns>
			__RLIB_INTERLOCKED_API_UNARY(Increment64, long long);
			/// <summary>
			/// 以原子操作的形式递减指定变量的值并存储结果, 并且不设置内存屏障
			/// </summary>
			/// <param name="location">其值要递减的变量, 必须 32-bit 对齐</param>
			/// <returns>递减后的值</returns>
			__RLIB_INTERLOCKED_API_UNARY(DecrementNoFence, long);
			/// <summary>
			/// 以原子操作的形式递减指定变量的值并存储结果
			/// </summary>
			/// <param name="location">其值要递减的变量, 必须 32-bit 对齐</param>
			/// <returns>递减后的值</returns>
			__RLIB_INTERLOCKED_API_UNARY(Decrement, long);
			/// <summary>
			/// 以原子操作的形式递减指定变量的值并存储结果
			/// </summary>
			/// <param name="location">其值要递减的变量, 必须 16-bit 对齐</param>
			/// <returns>递减后的值</returns>
			__RLIB_INTERLOCKED_API_UNARY(Decrement16, short);
			/// <summary>
			/// 以原子操作的形式递减指定变量的值并存储结果
			/// </summary>
			/// <param name="location">其值要递减的变量, 必须 64-bit 对齐</param>
			/// <returns>递减后的值</returns>
			__RLIB_INTERLOCKED_API_UNARY(Decrement64, long long);
			/// <summary>
			/// 以原子操作的形式，将变量设置为指定的值并返回原始值, 并且不设置内存屏障
			/// </summary>
			/// <param name="location1">要设置为指定值的变量, 必须 32-bit 对齐</param>
			/// <param name="value">location1 参数被设置为的值</param>
			/// <returns>location1 的原始值</returns>
			__RLIB_INTERLOCKED_API_BINARY(ExchangeNoFence);
			/// <summary>
			/// 以原子操作的形式，将变量设置为指定的值并返回原始值
			/// </summary>
			/// <param name="location1">要设置为指定值的变量, 必须 32-bit 对齐</param>
			/// <param name="value">location1 参数被设置为的值</param>
			/// <returns>location1 的原始值</returns>
			__RLIB_INTERLOCKED_API_BINARY(Exchange);
			/// <summary>
			/// 以原子操作的形式，将变量设置为指定的值并返回原始值
			/// </summary>
			/// <param name="location1">要设置为指定值的变量, 必须 16-bit 对齐</param>
			/// <param name="value">location1 参数被设置为的值</param>
			/// <returns>location1 的原始值</returns>
			__RLIB_INTERLOCKED_API_BINARY(Exchange16);
			/// <summary>
			/// 以原子操作的形式，将变量设置为指定的值并返回原始值
			/// </summary>
			/// <param name="location1">要设置为指定值的变量, 必须 64-bit 对齐</param>
			/// <param name="value">location1 参数被设置为的值</param>
			/// <returns>location1 的原始值</returns>
			__RLIB_INTERLOCKED_API_BINARY(Exchange64);
			/// <summary>
			/// 以原子操作的形式，将指针变量设置为指定的值并返回原始值
			/// </summary>
			/// <param name="location1">要设置为指定值的指针变量, 必须对齐</param>
			/// <param name="value">location1 参数被设置为的值</param>
			/// <returns>location1 的原始值</returns>
			__RLIB_INTERLOCKED_API_BINARY(ExchangePointer);
			/// <summary>
			/// 对两个 32 位整数进行求和并用和替换第一个整数，上述操作作为一个原子操作完成, 并且不设置内存屏障
			/// </summary>
			/// <param name="location1">一个变量，必须 32-bit 对齐，包含要添加的第一个值. 两个值的和存储在 location1 中</param>
			/// <param name="value">要添加到整数中的 location1 位置的值</param>
			/// <returns>存储在 location1 处的新值</returns>
			__RLIB_INTERLOCKED_API_BINARY(AddNoFence);
			/// <summary>
			/// 对两个 32 位整数进行求和并用和替换第一个整数，上述操作作为一个原子操作完成
			/// </summary>
			/// <param name="location1">一个变量，必须 32-bit 对齐，包含要添加的第一个值. 两个值的和存储在 location1 中</param>
			/// <param name="value">要添加到整数中的 location1 位置的值</param>
			/// <returns>存储在 location1 处的新值</returns>
			__RLIB_INTERLOCKED_API_BINARY(Add);
			/// <summary>
			/// 对两个 32 位整数进行求和并用和替换第一个整数，上述操作作为一个原子操作完成
			/// </summary>
			/// <param name="location1">一个变量，必须 32-bit 对齐，包含要添加的第一个值. 两个值的和存储在 location1 中</param>
			/// <param name="value">要添加到整数中的 location1 位置的值</param>
			/// <returns>location1 处的原值</returns>
			__RLIB_INTERLOCKED_API_BINARY(ExchangeAdd);
			/// <summary>
			/// 对两个 16 位整数进行求和并用和替换第一个整数，上述操作作为一个原子操作完成
			/// </summary>
			/// <param name="location1">一个变量，必须 16-bit 对齐，包含要添加的第一个值. 两个值的和存储在 location1 中</param>
			/// <param name="value">要添加到整数中的 location1 位置的值</param>
			/// <returns>存储在 location1 处的新值</returns>
			__RLIB_INTERLOCKED_API_BINARY(Add16);
			/// <summary>
			/// 对两个 64 位整数进行求和并用和替换第一个整数，上述操作作为一个原子操作完成
			/// </summary>
			/// <param name="location1">一个变量，必须 64-bit 对齐，包含要添加的第一个值. 两个值的和存储在 location1 中</param>
			/// <param name="value">要添加到整数中的 location1 位置的值</param>
			/// <returns>存储在 location1 处的新值</returns>
			__RLIB_INTERLOCKED_API_BINARY(Add64);
			/// <summary>
			/// 比较两个 32 位有符号整数是否相等，如果相等则替换第一个值，上述操作作为一个原子操作完成
			/// </summary>
			/// <param name="location1">其值将与 comparand 进行比较并且可能被替换的目标</param>
			/// <param name="value">比较结果相等时替换目标值的值</param>
			/// <param name="comparand">与位于 location1 处的值进行比较的值</param>
			/// <returns>location1 中的原值</returns>
			__RLIB_INTERLOCKED_API_TEMARY(CompareExchange, long);
			/// <summary>
			/// 比较两个 64 位有符号整数是否相等，如果相等则替换第一个值，上述操作作为一个原子操作完成
			/// </summary>
			/// <param name="location1">其值将与 comparand 进行比较并且可能被替换的目标</param>
			/// <param name="value">比较结果相等时替换目标值的值</param>
			/// <param name="comparand">与位于 location1 处的值进行比较的值</param>
			/// <returns>location1 中的原值</returns>
			__RLIB_INTERLOCKED_API_TEMARY(CompareExchange64, long long);
			/// <summary>
			/// 比较两个指针是否相等，如果相等则替换第一个值，上述操作作为一个原子操作完成
			/// </summary>
			/// <param name="location1">其值将与 comparand 进行比较并且可能被替换的目标</param>
			/// <param name="value">比较结果相等时替换目标值的值</param>
			/// <param name="comparand">与位于 location1 处的值进行比较的值</param>
			/// <returns>location1 中的原值</returns>
			__RLIB_INTERLOCKED_API_TEMARY(CompareExchangePointer, PVOID);
		};
	}
}