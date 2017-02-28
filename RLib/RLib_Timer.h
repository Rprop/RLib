/********************************************************************
	Created:	2012/03/10  07:06
	Filename: 	RLib_Timer.h
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#ifndef _USE_TIMESPAN
#define _USE_TIMESPAN
#include "RLib_AppBase.h"
#if (WINVER > _WIN32_WINNT_WIN7)
# include <profileapi.h>
#endif

//-------------------------------------------------------------------------

namespace System
{
	/// <summary>
	/// 提供一种计算代码执行时间的类
	/// </summary>
	class Timer
	{
	public:
		LARGE_INTEGER m_frequency, m_start_counter, m_end_counter;
	public:
		RLIB_DECLARE_DYNCREATE;
		Timer(){
			QueryPerformanceFrequency(&this->m_frequency);
		}
		/// <summary>
		/// 开始计时
		/// </summary>
		RLIB_INLINE void BeginTimer(){
			QueryPerformanceCounter(&this->m_start_counter);
		}
		/// <summary>
		/// 停止计时
		/// </summary>
		RLIB_INLINE void EndTimer(){
			QueryPerformanceCounter(&this->m_end_counter);
		}
		/// <summary>
		/// 获取计时差值
		/// </summary>
		RLIB_INLINE LONGLONG GetTimeSpan(){
			return this->m_end_counter.QuadPart - this->m_start_counter.QuadPart;
		}
		/// <summary>
		/// 获取双精度计时差值
		/// </summary>
		RLIB_INLINE double GetDoubleTimeSpan(){
			return static_cast<double>(this->GetTimeSpan()) / this->m_frequency.QuadPart;
		}
		/// <summary>
		/// 将当前 Timer 对象的值(double)转换为其等效的字符串表示形式
		/// </summary>
		RLIB_INLINE String ToString(){
			return Double(this->GetDoubleTimeSpan()).ToString();
		}
	};
}
#endif // _USE_TIMESPAN