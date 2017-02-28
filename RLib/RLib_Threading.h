/********************************************************************
	Created:	2012/08/09  17:31
	Filename: 	RLib_Threading.h
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#ifndef _USE_THREAD_SYNC
#define _USE_THREAD_SYNC
#include "RLib_AppBase.h"
#include "RLib_Fundamental.h"

#define Synchronized(s, c) s.Lock();{ c };s.UnLock()
#define Unsynchronized(s)  s.UnLock()
#ifndef INFINITE
# define INFINITE 0xFFFFFFFF
#endif // !INFINITE

//-------------------------------------------------------------------------

namespace System
{
	namespace Threading
	{
		enum WaitStatus
		{
			/// <summary>
			/// The caller did not have the required privileges to the event specified by the Handle parameter
			/// </summary>
			WAIT_ACCESS_DENIED,
			/// <summary>
			/// The supplied Handle parameter was invalid
			/// </summary>
			WAIT_INVALID_HANDLE,
			/// <summary>
			/// The specified object satisfied the wait
			/// </summary>
			WAIT_SUCCESS,
			/// <summary>
			/// A time out occurred before the object was set to a signaled state.
			/// This value can be returned when the specified set of wait conditions cannot be immediately met 
			/// and the Timeout parameter is set to zero
			/// </summary>
			WAIT_TIMEOUTED,
			/// <summary>
			/// The wait was aborted to deliver a user APC to the current thread
			/// </summary>
			WAIT_USER_APC,	
			/// <summary>
			/// The wait was aborted to deliver an alert to the current thread
			/// </summary>
			WAIT_ALERTED = WAIT_USER_APC
		};
		/// <summary>
		/// 封装等待对共享资源的独占访问的操作系统特定的对象
		/// </summary>
		class RLIB_API RLIB_THREAD_SAFE WaitHandle
		{
		protected:
			RLIB_OBJECT_ATTRIBUTES m_obj;
			RLIB_UNICODE_STRING    m_name;

		public:
			/// <summary>
			/// Gets the native operating system handle
			/// </summary>
			HANDLE Handle;

		public:
			/// <summary>
			/// If a timeout is specified, and the object has not attained a state of signaled when the timeout expires,
			/// then the wait is automatically satisfied. An INFINITE timeout will never expire.
			/// If an explicit timeout value of zero is specified, then no wait occurs if the wait cannot be satisfied immediately. 
			/// </summary>
			WaitStatus WaitOne(DWORD millisecondsTimeout = INFINITE);
			~WaitHandle();

		public:
			/// <summary>
			/// 等待指定数组中的任一或者全部元素收到信号，使用 32 位带符号整数指定时间间隔
			/// </summary>
			static WaitStatus Wait(ULONG nCount, HANDLE *pHandles, bool bWaitAll, bool bAlertable, DWORD millisecondsTimeout);
			/// <summary>
			/// 阻止当前线程，直到当前收到信号，同时使用 32 位带符号整数指定时间间隔
			/// </summary>
			static WaitStatus WaitOne(HANDLE handle, bool bAlertable, DWORD millisecondsTimeout);
		};
		/// <summary>
		/// 表示一个同步基元, 也可用于进程间同步
		/// </summary>
		class RLIB_API RLIB_THREAD_SAFE Mutex : public WaitHandle
		{
		private:
			/// <summary>
			/// this constructor is a bogus constructor that does nothing
			/// and is for use only with OpenExisting.
			/// </summary>
			Mutex(LPVOID) {}

		public:
			~Mutex() = default;
			RLIB_DECLARE_DYNCREATE;
			/// <summary>
			/// 用一个指示调用线程是否应拥有互斥体的初始所属权的布尔值和一个作为互斥体名称的字符串来初始化 Mutex 类的新实例
			/// </summary>
			Mutex(bool bOwner = false, const String &szName = Nothing);
			/// <summary>
			/// 释放 Mutex 一次
			/// </summary>
			/// <returns>Internal mutant counter state before call ReleaseMutex</returns>
			LONG ReleaseMutex();

		public:
			/// <summary>
			/// 如果它已经存在，打开指定的命名 Mutex
			/// </summary>
			static Mutex *OpenExisting(const String &);
		};
		/// <summary>
		/// 表示事件类型
		/// </summary>
		enum EventType 
		{
			Notification_Event,
			Synchronization_Event
		};
		/// <summary>
		/// 表示线程同步事件
		/// </summary>
		class RLIB_API RLIB_THREAD_SAFE Event : public WaitHandle
		{
		public:
			/// <summary>
			/// 初始化 Event 类的新实例
			/// </summary>
			/// <param name="InitialState">The initial state of the event object</param>
			Event(EventType eType, bool bInitialState, const String &szName = Nothing);
			Event() = default;
			~Event() = default;
			RLIB_DECLARE_DYNCREATE;

		public:
			/// <summary>
			/// 设置事件信号
			/// </summary>
			/// <returns>State of Event Object before function call</returns>
			LONG SetSignal();
			/// <summary>
			/// 清除事件信号
			/// </summary>
			void Clear();
			/// <summary>
			/// 将指定的事件设为发出信号状态
			/// 如果是一个人工重设事件, 正在等候事件的、被挂起的所有线程都会进入活动状态,
			/// 函数随后将事件设回未发信号状态, 并返回.
			/// 如果是一个自动重设事件, 则正在等候事件的、被挂起的单个线程会进入活动状态,
			/// 事件随后设回未发信号状态, 并且函数返回
			/// </summary>
			/// <returns>State of Event Object before function call</returns>
			LONG Pulse();

		public:
			/// <summary>
			/// 如果它已经存在，打开指定的命名 Event
			/// </summary>
			static Event *OpenExisting(const String &);
		};
		/// <summary>
		/// 限制可同时访问某一资源或资源池的线程数
		/// </summary>
		class RLIB_API RLIB_THREAD_SAFE Semaphore : public WaitHandle
		{
		private:
			/// <summary>
			/// this constructor is a bogus constructor that does nothing
			/// and is for use only with OpenExisting.
			/// </summary>
			Semaphore() = default;

		public:
			RLIB_DECLARE_DYNCREATE;
			/// <summary>
			/// 初始化 Semaphore 类的新实例，并指定最大并发入口数，还可以选择为调用线程保留某些入口，以及选择指定系统信号量对象的名称
			/// </summary>
			Semaphore(ULONG nInitialCount, ULONG nMaximumCount, const String &szName = Nothing);
			/// <summary>
			/// 以指定的次数退出信号量并返回前一个计数
			/// </summary>
			ULONG Release(ULONG releaseCount = 1);

		public:
			/// <summary>
			/// 如果它已经存在，打开指定的命名 Mutex
			/// </summary>
			static Semaphore *OpenExisting(const String &);
		};
	}
}
#endif // _USE_THREAD_SYNC