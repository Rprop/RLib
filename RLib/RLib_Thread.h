/********************************************************************
	Created:	2012/02/05  22:43
	Filename: 	RLib_Thread.h
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#ifndef _USE_THREAD
#define _USE_THREAD
#include "RLib_Threading.h"
#include "RLib_Exception.h"

//-------------------------------------------------------------------------

namespace System
{
	/// <summary>
	/// 包含启用多线程编程的类型
	/// </summary>
	namespace Threading
	{
		/// <summary>
		/// 表示线程开始要执行的一系列方法
		/// </summary>
		class Runnable
		{
		public:
			using StdParameterizedThreadStart = DWORD(_stdcall *)(LPVOID lpThreadParameter);
			using StdThreadStart              = DWORD(_stdcall *)();
			using CppParameterizedThreadStart = void(_cdecl *)(LPVOID lpThreadParameter);
			using CppThreadStart              = void(_cdecl *)();

		public:
			CppParameterizedThreadStart callback;
			LPVOID                      state;
			template<typename T> static Runnable Any(void(_cdecl *callBack)(T), T state)
			{
				return Runnable { static_cast<CppParameterizedThreadStart>(static_cast<LPVOID>(callBack)), 
					reinterpret_cast<LPVOID>(*reinterpret_cast<intptr_t *>(&state)) };
			}

		public:
			enum RunnableType
			{
				/// <summary>
				/// 标准, 该线程方法有参数
				/// </summary>
				StandardParameterized = 0,
				/// <summary>
				/// 拓展, 但是该线程方法无参数
				/// </summary>
				Standard,
				/// <summary>
				/// C++拓展, 该线程方法遵循C++调用约定并带参数
				/// </summary>
				CppParameterized,
				/// <summary>
				/// C++拓展, 该线程方法遵循C++调用约定
				/// </summary>
				Cpp,
			};
		};
		/// <summary>
		/// 指定 Thread 的执行状态
		/// </summary>
		enum class ThreadState : int
		{
			/// <summary>
			/// 尚未对线程调用 Thread.Start 方法
			/// </summary>
			Unstarted = 0,
			/// <summary>
			/// 线程已启动，它未被阻塞
			/// </summary>
			Running = 1,
			/// <summary>
			/// 线程已终止
			/// </summary>
			Stopped = 2,
			/// <summary>
			/// 线程已挂起
			/// </summary>
			Suspended = 3,
			/// <summary>
			/// 调用 Thread.Exit 方法中止了线程
			/// </summary>
			Aborted = 4,
		};
		/// <summary>
		/// 指定 Thread 的调度优先级
		/// </summary>
		enum class ThreadPriority
		{
			/// <summary>
			/// 获取调度优先级失败
			/// </summary>
			ErrorPriority = -1,
			/// <summary>
			/// Lowest thread priority level
			/// </summary>
			LowestPriority = 0/*LOW_PRIORITY*/,    
			/// <summary>
			/// Highest thread priority level
			/// </summary>
			HighestPriority = 31/*HIGH_PRIORITY*/    
		};
		/// <summary>
		/// 表示线程异常
		/// </summary>
		RLIB_INTERNAL_EXCEPTION(ThreadException, Exception);
		/// <summary>
		/// Creates and controls a thread, sets its priority, and gets its status
		/// How to: Set a Thread Name in Native Code
		/// https://msdn.microsoft.com/en-us//library/xcb2z8hs.aspx
		/// </summary>
		class RLIB_API Thread
		{
		private:
			LPVOID                   m_startAddress; 
			LPVOID                   m_parameter; 
			HANDLE                   m_hThread;			
			Runnable::RunnableType   m_startType;
			mutable ThreadException  m_error;
			volatile ThreadState     m_state;
			DWORD                    m_ThreadId;

		private:
			void thread_create();
			void setException(NTSTATUS) const;

		public:
			/// <summary>
			/// 使用指定的线程句柄及线程ID(允许为0)初始化新的Thread类 
			/// 自动接管该句柄的所有工作, 默认该线程正在运行
			/// </summary>
			Thread(HANDLE, DWORD, BOOL IsRunning);
			/// <summary>
			/// 初始化 Thread 类的新实例
			/// </summary>
			Thread(const Runnable &);
			/// <summary>
			/// 初始化 Thread 类的新实例，并允许传参数给线程
			/// </summary>
			Thread(Runnable::StdParameterizedThreadStart, LPVOID lpParameter);
			/// <summary>
			/// 初始化 Thread 类的新实例
			/// </summary>
			Thread(Runnable::StdThreadStart);
			/// <summary>
			/// 初始化 Thread 类的新实例，并允许传参数给线程
			/// </summary>
			Thread(Runnable::CppParameterizedThreadStart, LPVOID lpParameter);
			/// <summary>
			/// 初始化 Thread 类的新实例
			/// </summary>
			Thread(Runnable::CppThreadStart);
			/// <summary>
			/// 释放内存并关闭线程对象
			/// 如果在线程过程中销毁线程对象本身, 则必须调用ExitThread以退出线程
			/// </summary>
			~Thread();
			RLIB_DECLARE_DYNCREATE;

		public:
			/// <summary>
			/// 设置一个值，该值表示线程执行时使用的参数
			/// 方法必须在 Start 方法执行前被调用
			/// </summary>
			void SetParameter(LPVOID lpParameter);
			/// <summary>
			/// 获取线程对象句柄
			/// </summary>
			operator HANDLE();
			/// <summary>
			/// 获取线程ID, 失败返回0
			/// </summary>
			const DWORD GetThreadId() const;
			/// <summary>
			/// 获取线程Id
			/// </summary>
			RLIB_PROPERTY_GET(DWORD ThreadId, GetThreadId);
			/// <summary>
			/// 设置线程名称, 仅在调试器中有效
			/// @warning the caller must handle EXCEPTION_MS_VC(0x406D1388)
			/// </summary>
			void SetThreadName(LPCSTR);
			/// <summary>
			/// 设置线程名称, 仅在调试器中有效
			/// @warning the caller must handle EXCEPTION_MS_VC(0x406D1388)
			/// </summary>
			RLIB_PROPERTY_SET(LPCSTR ThreadName, SetThreadName);
			/// <summary>
			/// 获取一个值，该值指示线程的调度优先级
			/// 失败返回 ErrorPriority
			/// </summary>
			ThreadPriority GetPriority() const;
			/// <summary>
			/// 设置一个值，该值指示线程的调度优先级
			/// LowestPriority < newPriority <= HighestPriority
			/// </summary>
			bool SetPriority(ThreadPriority newPriority);
			/// <summary>
			/// 获取或设置一个值，该值指示线程的调度优先级
			/// </summary>
			RLIB_PROPERTY_GET_SET(ThreadPriority Priority, GetPriority, SetPriority);
			/// <summary>
			/// 获取或设置一个值，该值指示某个线程是否为后台线程
			/// 如果该属性为true, 则线程结束时将自动销毁 Thread 实例, 这意味着无须
			/// 手动释放 Thread 实例.
			/// </summary>
			bool IsBackground; 	
			/// <summary>
			/// 设置一个值，该值指示某个线程是否为后台线程
			/// @see IsBackground
			/// </summary>
			RLIB_INLINE Thread *SetBackground(bool bIsBackground = true) {
				this->IsBackground = bIsBackground;
				return this;
			}
			/// <summary>
			/// 获取或设置一个值，该值指示是否禁止在线程结束时更新为 ThreadState::Stoped 状态
			/// 如果该值被设置为 true, 则必须调用 ExitThread 方法结束线程
			/// 默认值为 false
			/// </summary>
			bool IsSuppressChangeState;
			/// <summary>
			/// 获取一个值，该值指示当前线程的执行状态
			/// 如果此线程已启动并且尚未正常终止或中止，则为 true；否则为 false
			/// </summary>
			bool GetIsAlive() const;
			/// <summary>
			/// 如果此线程已启动并且尚未正常终止或中止，则为 true；否则为 false
			/// </summary>
			RLIB_PROPERTY_GET(bool IsAlive, GetIsAlive);
			/// <summary>
			/// 获取一个值，该值指示当前线程的状态
			/// </summary>
			ThreadState GetState() const;
			/// <summary>
			/// 获取一个值，该值指示当前线程的状态
			/// </summary>
			RLIB_PROPERTY_GET(ThreadState State, GetState);

		public:
			/// <summary>
			/// 导致操作系统将当前实例的状态更改为 ThreadState::Running
			/// </summary>
			bool Start();
			/// <summary>
			/// 唤醒线程
			/// </summary>
			bool Alert();
			/// <summary>
			/// 已过时.继续已挂起的线程
			/// </summary>
			bool Resume();
			/// <summary>
			/// 已过时.挂起线程，或者如果线程已挂起，则不起作用
			/// </summary>
			bool Suspend();
			/// <summary>
			/// 已过时.在调用此方法以开始中止此线程, 调用此方法通常会中止线程
			/// </summary>
			bool Abort(long exitstatus = -1);
			/// <summary>
			/// 在当前线程上执行等待操作, 调用 Start() 后立即调用 Wait() 可能不起作用(线程状态未更新)
			/// </summary>
			WaitStatus Wait(DWORD millisecondsTimeout = INFINITE);
			/// <summary>
			/// 关闭线程对象
			/// </summary>
			bool Close();
			/// <summary>
			/// 获取Thread发生的异常信息
			/// </summary>
			ThreadException *GetLastException();

		public:
			/// <summary>
			/// 获取当前正在运行的线程
			/// </summary>
			static Thread *GetCurrentThread();
			/// <summary>
			/// 退出当前线程并释放线程内存
			/// </summary>
			static void ExitThread(DWORD code = 0);
			/// <summary>
			/// 将当前线程挂起指定的时间并指定是否允许唤醒线程
			/// </summary>
			/// <returns>
			/// WAIT_TIMEOUTED if the specified time interval expired.
			/// WAIT_ALERTED if the function returned due to one or more I/O 
			/// completion callback functions. This can happen only if bAlertable is true, 
			/// and if the thread that called the WAIT_ALERTED function is the same thread that 
			/// called the extended I/O function.
			/// </returns>
			static WaitStatus Sleep(DWORD dwMilliseconds, bool bAlertable = true);
			/// <summary>
			/// 唤醒指定线程
			/// </summary>
			static bool AlertThread(HANDLE hThread);
			/// <summary>
			/// Difference between AlertResumeThread and Thread.Resume it's the first one 
			/// sets Thread LPVOID to alerted state (so before thread will continue execution, 
			/// all APC will be executed).
			/// </summary>
			/// <returns>Returns number of suspend request for thread ThreadHandle before call AlertResumeThread.
			/// If this number is 0, thread will continue execution.</returns>
			static ULONG AlertResumeThread(HANDLE hThread);
			/// <summary>
			/// 设置线程名称, 仅在调试器中有效
			/// @warning the caller must handle EXCEPTION_MS_VC(0x406D1388)
			/// </summary>
			static void SetThreadName(DWORD threadId, LPCSTR name);

		private:
			/// <summary>
			/// 线程的转发函数
			/// </summary>
			static DWORD _stdcall __threadProcWrapper(Thread *lpMySelf);
		};
	};
};
#endif