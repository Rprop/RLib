/********************************************************************
	Created:	2012/08/05  12:56
	Filename: 	ThreadPool.h
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#ifndef _USE_THREADPOOL
#define _USE_THREADPOOL
#include "RLib_Thread.h"
#include "RLib_List.h"
#include "RLib_Queue.h"
#include "RLib_Monitor.h"
#include "RLib_SafeObject.h"

#define TaskExecutor(a)  static_cast<System::Threading::ThreadPoolExecutor>(reinterpret_cast<void *>(a))
#define TaskCallback(a,b) TaskExecutor(a), const_cast<LPVOID>(reinterpret_cast<LPCVOID>(b))

//-------------------------------------------------------------------------

namespace System
{
	namespace Threading
	{
		/// <summary>
		/// 表示线程池线程要执行的回调方法
		/// </summary>
		typedef Runnable::CppParameterizedThreadStart ThreadPoolExecutor;
		/// <summary>
		/// 表示线程池工作任务对象
		/// </summary>                                     
		struct RLIB_API ThreadPoolTask
		{
		public:
			ThreadPoolExecutor ExecutionTarget;
			LPVOID             ExecutionParameter;
		public:
			ThreadPoolTask() = default;
			ThreadPoolTask(ThreadPoolExecutor callback, LPVOID param = nullptr) {
				this->ExecutionTarget    = callback;
				this->ExecutionParameter = param;
			}
		};
		/// <summary>
		/// 表示线程池工作线程
		/// </summary>                                     
		struct RLIB_API ThreadPoolWorker
		{
			friend class ThreadPool;

		private:
			static void worker_workshop(struct ThreadPoolWorker *);

		public:
			Thread           *pThread;
			class ThreadPool *pManager;
			volatile long     nExit; 

		public:			
			ThreadPoolWorker(class ThreadPool *pOwner) {
				this->pThread = new Thread(TaskExecutor(worker_workshop), this);
				if (this->pThread == nullptr) {
					trace(!"new thread failed!");
					this->pManager = nullptr;
					return;
				} //if

				this->pManager = pOwner;
				this->nExit    = 0;
				this->pThread->IsSuppressChangeState = true;
				this->pThread->Start();
			}
			~ThreadPoolWorker();
			RLIB_DECLARE_DYNCREATE;

		public:
			bool IsAborting() {
				return this->nExit != 0;
			}
			void Abort();

		public:
			static void Dispose(ThreadPoolWorker **lppobj) {
				delete *lppobj;
			}
		};
		/// <summary>
		/// 线程池任务结构
		/// </summary>
		typedef Synchronizable<Collections::Generic::Queue<ThreadPoolTask>> TaskQueue;
		typedef Synchronizable<Collections::Generic::List<ThreadPoolWorker *, ThreadPoolWorker>> WorkerList;
		typedef Synchronizable<Collections::Generic::List<ThreadPoolWorker *>> TempWorkerList;
		/// <summary>
		/// 线程池支持类
		/// </summary>
		class RLIB_API RLIB_THREAD_SAFE ThreadPool
		{
			friend ThreadPoolWorker;

		protected:
#pragma warning(push)
#pragma warning(disable:4251)
			TaskQueue      m_tasks; 
			WorkerList     m_workers;
			TempWorkerList m_avail_workers;
#pragma warning(pop)		
			volatile intptr_t m_nMinThreads, m_nMaxThreads, m_nMaxIdleTime;
			volatile long     m_nDestroying, m_nPending;

		private:
			static void __threadPoolExecutorWrapper(Runnable::CppThreadStart _start);

		public:
			ThreadPool(const ThreadPool &) = delete;
			ThreadPool &operator = (const ThreadPool &) = delete;
			ThreadPool(intptr_t defaultMinThreads = 4, intptr_t defaultMaxThreads = 1024, 
					   intptr_t defaultMaxIdleTime = 6000);
			~ThreadPool();
			RLIB_DECLARE_DYNCREATE;

		public:
			//
			// 摘要:
			//     检索线程池是否正在进行终止工作
			bool IsDestroying() {
				return this->m_nDestroying != 0;
			}
			//
			// 摘要:
			//     检索所有任务是否全部完成(如果同时有新任务被添加可能会返回错误值)
			bool IsTasksComplete();
			//
			// 摘要:
			//     执行等待直到所有任务完成(如果同时有新任务被添加可能会中途返回)
			void WaitForTasksComplete(DWORD interval = 6000, bool bAlertable = true);
			//
			// 摘要:
			//     唤醒线程池中所有可唤醒的正在工作的线程
			void Alert();
			//
			// 摘要:
			//     设置中止信号以准备进行线程池中止工作
			void Abort();
			//
			// 摘要:
			//     取消 Abort() 方法设置的信号以让线程池正常工作
			void Reset();
			//
			// 摘要:
			//     暂停调取等待被执行的任务队列, 正在执行的任务不会被暂停
			void PausePendingTasks();
			//
			// 摘要:
			//     恢复调取等待被执行的任务队列
			void ResumePendingTasks();
			//
			// 摘要:
			//     清理等待被执行的任务队列
			void FinalizePendingTasks();
			//
			// 摘要:
			//     检索由 GetMaxThreads
			//     方法返回的最大线程池线程数和当前活动线程数之间的差值。
			intptr_t GetAvailableThreads();
			//
			// 摘要:
			//     检索由 GetMaxThreads
			//     方法返回线程池当前线程数。
			intptr_t GetThreads();
			//
			// 摘要:
			//     检索可以同时处于活动状态的线程池请求的数目。所有大于此数目的请求将保持排队状态，直到线程池线程变为可用。
			intptr_t GetMaxThreads();
			//
			// 摘要:
			//     检索线程池在新请求预测中维护的空闲线程数。
			intptr_t GetMinThreads();
			//
			// 摘要:
			//     检索线程池允许的最大空闲时间(ms)。
			intptr_t GetMaxIdleTime();
			//
			// 摘要:
			//     检索线程池滞留(正在排队)的任务数。
			intptr_t GetTasks();
			//
			// 摘要:
			//     将方法排入队列并调用 Dispatch 以便执行，并指定包含该方法所用数据的对象。
			//     此方法将在 Dispatch 成功分配任务后被执行。
			//
			// 参数:
			//   callBack:
			//     表示要执行的方法。
			//   state:
			//     表示传入所执行方法的参数。
			//
			// 返回结果:
			//     如果此方法成功排队，则为 true；如果未能将该工作项排队，则
			//     返回 false。不考虑 Dispatch 方法执行情况。
			bool AddTask(ThreadPoolExecutor callBack, LPVOID state);
			//
			// 摘要:
			//     将方法排入队列并调用 Dispatch 以便执行。
			//     此方法将在 Dispatch 成功分配任务后被执行。
			//
			// 参数:
			//   callBack:
			//     表示要执行的方法。
			//
			// 返回结果:
			//     如果此方法成功排队，则为 true；如果未能将该工作项排队，则
			//     返回 false。不考虑 Dispatch 方法执行情况。
			bool AddTask(Runnable::CppThreadStart callBack);
			/// <summary>
			/// @see AddTask
			/// </summary>
			template<typename T> bool AddTask(void(_cdecl *callBack)(T), T state) {
				return this->AddTask(reinterpret_cast<ThreadPoolExecutor>(callBack),
									 reinterpret_cast<LPVOID>(*reinterpret_cast<intptr_t *>(&state)));
			}
			/// <summary>
			/// @see AddTask
			/// </summary>
			template<typename T> bool InvokeAndWait(void(_cdecl *callBack)(T), T state) {
				return this->AddTask(reinterpret_cast<ThreadPoolExecutor>(callBack),
									 reinterpret_cast<LPVOID>(*reinterpret_cast<intptr_t *>(&state)));
			}
			/// <summary>
			/// @see AddTask
			/// </summary>
			RLIB_INLINE bool InvokeAndWait(Runnable::CppThreadStart callBack) {
				return this->AddTask(callBack);
			}
			//
			// 摘要:
			//     将方法排入队列以便执行，并指定包含该方法所用数据的对象。
			//     此方法在有线程池线程变得可用时执行。
			//
			// 参数:
			//   callBack:
			//     表示要执行的方法。
			//
			//   state:
			//     包含方法所用数据的对象。
			//
			// 返回结果:
			//     如果此方法成功排队，则为 true；如果未能将该工作项排队，则
			//     返回false
			bool QueueUserWorkItem(ThreadPoolExecutor callBack, LPVOID state);
			//
			// 摘要:
			//     将方法排入队列以便执行。
			//     此方法在有线程池线程变得可用时执行。
			//
			// 参数:
			//   callBack:
			//     表示要执行的方法。
			//
			//   state:
			//     包含方法所用数据的对象。
			//
			// 返回结果:
			//     如果此方法成功排队，则为 true；如果未能将该工作项排队，则
			//     返回false
			bool QueueUserWorkItem(Runnable::CppThreadStart callBack);
			/// <summary>
			/// @see QueueUserWorkItem
			/// </summary>
			template<typename T> bool QueueUserWorkItem(void(_cdecl *callBack)(T), T state) {
				return this->QueueUserWorkItem(reinterpret_cast<ThreadPoolExecutor>(callBack),
											   reinterpret_cast<LPVOID>(*reinterpret_cast<intptr_t *>(&state)));
			}
			/// @see QueueUserWorkItem
			/// </summary>
			template<typename T> bool InvokeLater(void(_cdecl *callBack)(T), T state) {
				return this->QueueUserWorkItem(reinterpret_cast<ThreadPoolExecutor>(callBack),
											   reinterpret_cast<LPVOID>(*reinterpret_cast<intptr_t *>(&state)));
			}
			/// <summary>
			/// @see QueueUserWorkItem
			/// </summary>
			RLIB_INLINE bool InvokeLater(Runnable::CppThreadStart callBack) {
				return this->QueueUserWorkItem(callBack);
			}
			//
			// 摘要:
			//     设置可以同时处于活动状态的线程池的请求数目。
			//     所有大于此数目的请求将保持排队状态，直到线程池线程变为可用。
			//
			// 参数:
			//   workerThreads:
			//     线程池中辅助线程的最大数目。
			//
			// 返回结果:
			//     如果更改成功，则为 true；否则为 false。
			bool SetMaxThreads(intptr_t workerThreads);
			//
			// 摘要:
			//     设置线程池在新请求预测中维护的空闲线程数。
			//
			// 参数:
			//   workerThreads:
			//     要由线程池维护的新的最小空闲辅助线程数。
			//
			// 返回结果:
			//     如果更改成功，则为 true；否则为 false。
			bool SetMinThreads(intptr_t workerThreads);
			//
			// 摘要:
			//     设置线程池允许的最大空闲时间(ms), 超过该时间线程池将缩减线程拥有量。
			//     默认值为8000(8s)
			//
			// 参数:
			//   maxIdleTime:
			//     线程池允许的最大空闲时间。
			bool SetMaxIdleTime(intptr_t maxIdleTime);
			//
			// 摘要:
			//     要求线程池尝试为队列中的任务分配工人
			//
			// 返回结果:
			//     如果成功为任务分配工人，则为 true；否则为 false。
			bool Dispatch();

		private:
			//
			// 摘要:
			//     将指定线程从线程池中移除, 不再受线程池管理
			//
			// 参数:
			//   thread:
			//     要移除的线程对象指针。
			//   available:
			//     指示要移除的线程是否在就绪列表中。
			void RemoveWorker(ThreadPoolWorker *pworker, bool available);
		};
	}
}

#endif //_USE_THREADPOOL