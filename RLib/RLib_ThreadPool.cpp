/********************************************************************
	Created:	2012/07/26  19:32
	Filename: 	RLib_ThreadPool.cpp
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#include "RLib_ThreadPool.h"
#include "RLib_Interlocked.h"
#include "RLib_Helper.h"
#if (WINVER > _WIN32_WINNT_WIN7)
# include <processthreadsapi.h>
#endif
using namespace System::Threading;
using namespace System::Collections::Generic;
#ifdef _WIN64
# define __safe_exchange(a,b) Interlocked::Exchange64(a,b)
#else 
# define __safe_exchange(a,b) Interlocked::Exchange<LONG>(reinterpret_cast<volatile LONG *>(a), b)
#endif // _WIN64

//-------------------------------------------------------------------------

ThreadPoolWorker::~ThreadPoolWorker()
{
	if (this->pThread != nullptr) {
		if (this->pThread->ThreadId != AppBase::GetCurrentThreadId()) {
			this->pThread->Wait();
		} //if
		delete this->pThread;
	} //if
}

//-------------------------------------------------------------------------

void ThreadPool::__threadPoolExecutorWrapper(Runnable::CppThreadStart _start)
{
	return _start();
}

//-------------------------------------------------------------------------

void ThreadPoolWorker::Abort()
{
	Interlocked::Increment(&this->nExit);
}

//-------------------------------------------------------------------------

ThreadPool::ThreadPool(intptr_t defaultMinThreads /* = 4 */, intptr_t defaultMaxThreads /* = 1024 */, 
					   intptr_t defaultMaxIdleTime /* = 6000 */) : m_tasks(static_cast<intptr_t>(RLIB_DEFAULT_BUFFER_SIZE))
{
// 	this->m_tasks         = new TaskQueue::Type;
// 	this->m_workers       = new WorkerList::Type;
// 	this->m_avail_workers = new TempWorkerList::Type;
	this->m_nDestroying   = 0;
	this->m_nPending      = 0;

	__safe_exchange(&this->m_nMaxThreads, defaultMaxThreads);
	__safe_exchange(&this->m_nMinThreads, defaultMinThreads); 
	__safe_exchange(&this->m_nMaxIdleTime, defaultMaxIdleTime);
}

//-------------------------------------------------------------------------

void ThreadPool::Abort()
{
	Interlocked::Increment(&this->m_nDestroying);
}

//-------------------------------------------------------------------------

void ThreadPool::Reset()
{
	Interlocked::Exchange(&this->m_nDestroying, 0L);
}

//-------------------------------------------------------------------------

ThreadPool::~ThreadPool()
{
	this->Abort();

	bool _isWaitTaskCompleted = false;
	while (!_isWaitTaskCompleted)
	{
		if (this->m_workers.TryLock()) {
			_isWaitTaskCompleted = this->m_workers.Length == 0;
			if (!_isWaitTaskCompleted) {
				for (auto &lpworker : this->m_workers) {
					if (!lpworker->pThread->Alert()) {
						alert(lpworker->pThread->GetLastException()->Message);
						break;
					} //if
				}
			} //if
			this->m_workers.UnLock();
		} //if

		// alerts thread pool workers
		if (this->m_avail_workers.TryLock()) {
			for (auto &lpworker : this->m_avail_workers) {
				if (!lpworker->pThread->Alert()) {
					alert(lpworker->pThread->GetLastException()->Message);
					break;
				} //if
			}
			this->m_avail_workers.UnLock();
		} //if

		Thread::Sleep(300); // avoids lock competition
	}

	assert(this->m_avail_workers.Length == 0);
	assert(this->m_workers.Length == 0);
}

//-------------------------------------------------------------------------

void ThreadPool::Alert()
{
	Synchronized(this->m_workers, {
		for (auto &lpworker : this->m_workers) {
			if (!lpworker->pThread->Alert()) {
				alert(lpworker->pThread->GetLastException()->Message);
				break;
			} //if
		}
	});
}

//-------------------------------------------------------------------------

intptr_t ThreadPool::GetAvailableThreads()
{
	AutoLock<> locker(this->m_avail_workers.SyncRoot);
	return this->m_avail_workers.Length;
}

//-------------------------------------------------------------------------

intptr_t ThreadPool::GetThreads()
{
	AutoLock<> locker(this->m_workers.SyncRoot);
	return this->m_workers.Length;
}

//-------------------------------------------------------------------------

intptr_t ThreadPool::GetMaxThreads()
{
	return this->m_nMaxThreads;
}

//-------------------------------------------------------------------------

intptr_t ThreadPool::GetMinThreads()
{
	return this->m_nMinThreads;
}

//-------------------------------------------------------------------------

intptr_t ThreadPool::GetMaxIdleTime()
{
	return this->m_nMaxIdleTime;
}

//-------------------------------------------------------------------------

intptr_t ThreadPool::GetTasks()
{
	AutoLock<> locker(this->m_tasks.SyncRoot);
	return this->m_tasks.Count;
}

//-------------------------------------------------------------------------

bool ThreadPool::AddTask(ThreadPoolExecutor callBack, LPVOID state)
{
	bool bResult = this->QueueUserWorkItem(callBack, state);
	if (bResult) this->Dispatch();
	return bResult;
}

//-------------------------------------------------------------------------

bool ThreadPool::AddTask(Runnable::CppThreadStart callBack)
{
	bool bResult = this->QueueUserWorkItem(TaskExecutor(__threadPoolExecutorWrapper),
										   callBack);
	if (bResult) this->Dispatch();
	return bResult;
}

//-------------------------------------------------------------------------

bool ThreadPool::QueueUserWorkItem(ThreadPoolExecutor callBack, LPVOID state)
{
	AutoLock<> locker(this->m_tasks.SyncRoot);
	return this->m_tasks.Enqueue(ThreadPoolTask(callBack, state));
}

//-------------------------------------------------------------------------

bool ThreadPool::QueueUserWorkItem(Runnable::CppThreadStart callBack)
{
	return this->QueueUserWorkItem(TaskExecutor(__threadPoolExecutorWrapper), 
								   callBack);
}

//-------------------------------------------------------------------------

bool ThreadPool::SetMaxThreads(intptr_t workerThreads)
{
	assert(workerThreads >= 1);
	if (workerThreads < this->m_nMinThreads) {
		return false;
	} //if
	__safe_exchange(&this->m_nMaxThreads, workerThreads);

	intptr_t exist_workers;
	Synchronized(this->m_workers, {
		exist_workers = this->m_workers.Length;
	});

	// we need release workers
	if (exist_workers > workerThreads) {
		auto idle_workers = exist_workers - workerThreads;
		Synchronized(this->m_avail_workers, {
			for (auto &lpworker : this->m_avail_workers) {
				lpworker->Abort();
				if (--idle_workers == 0) {
					break;
				} //if
			}
		});
	} else {
		this->Dispatch();
	} //if
	return true;
}

//-------------------------------------------------------------------------

bool ThreadPool::SetMinThreads(intptr_t workerThreads)
{
	assert(workerThreads >= 1);
	if (workerThreads > this->m_nMaxThreads) {
		return false;
	} //if
	__safe_exchange(&this->m_nMinThreads, workerThreads);

	AutoLock<> locker(this->m_workers.SyncRoot);

	// we need more workers
	while(this->m_workers.Length < workerThreads)
	{
		auto pworker = new ThreadPoolWorker(this);
		if (!pworker || !pworker->pThread || this->m_workers.Add(pworker) == nullptr) {
			return false;
		} //if
	}
	return true;
}

//-------------------------------------------------------------------------

bool ThreadPool::SetMaxIdleTime(intptr_t maxIdleTime)
{
	if (maxIdleTime < 0) {
		return false;
	} //if
	__safe_exchange(&this->m_nMaxIdleTime, maxIdleTime);

	return true;
}

//-------------------------------------------------------------------------

bool ThreadPool::Dispatch()
{
	intptr_t tasksCount = this->GetTasks();
	if (tasksCount < 1) {
		return false;
	} //if
	
	Synchronized(this->m_avail_workers, {
		for (auto &lpworker : this->m_avail_workers) {
			lpworker->pThread->Alert();
			if (--tasksCount < 1) {
				Unsynchronized(this->m_avail_workers);
				return true;
			} //if
		}
	});
	
	// add thread pool workers
	{
		AutoLock<Monitor> locker(this->m_workers.SyncRoot);
		
		if (this->m_workers.Length < this->m_nMaxThreads)
		{
			while(this->m_workers.Length < this->m_nMaxThreads && tasksCount >= 1)
			{
				auto pworker = new ThreadPoolWorker(this);
				if (!pworker) return false;
				if (!pworker->pThread || this->m_workers.Add(pworker) == nullptr) {
					delete pworker;
					return false;
				} //if
				--tasksCount;
			}
			return true;
		} //if
	}
	return false;
}

//-------------------------------------------------------------------------

bool ThreadPool::IsTasksComplete()
{
	if (this->GetTasks() != 0) {
		return false;
	} //if
	return this->GetAvailableThreads() == this->GetThreads();
}

//-------------------------------------------------------------------------

void ThreadPool::PausePendingTasks()
{
	Interlocked::Increment(&this->m_nPending);
}

//-------------------------------------------------------------------------

void ThreadPool::ResumePendingTasks()
{
	Interlocked::Exchange(&this->m_nPending, 0L);
}

//-------------------------------------------------------------------------

void ThreadPool::FinalizePendingTasks()
{
	AutoLock<> locker(this->m_tasks.SyncRoot);
	return this->m_tasks.Clear();
}

//-------------------------------------------------------------------------

void ThreadPool::WaitForTasksComplete(DWORD interval /* = 6000 */, 
									  bool bAlertable /* = true */)
{
	while (!this->IsTasksComplete())
	{
		if (this->IsDestroying()) {
			break;
		} //if
		Threading::Thread::Sleep(interval, bAlertable);
	}
}

//-------------------------------------------------------------------------

void ThreadPool::RemoveWorker(ThreadPoolWorker *pworker_removing, 
							  bool available)
{
	if(available) {
		Synchronized(this->m_avail_workers, {
			foreachList(start, this->m_avail_workers) {
				if (*start == pworker_removing) {
					this->m_avail_workers.Remove(start);
					available = false;
					break;
				} //if
			}
		});
		assert(available != true || !"unexpected avail state " RLIB_LINE);
	} //if


	Synchronized(this->m_workers, {
		foreachList(start, this->m_workers) {
			if (*start == pworker_removing) {
				this->m_workers.Remove(start);
				break;
			} //if
		}
	});
}

//-------------------------------------------------------------------------

void ThreadPoolWorker::worker_workshop(struct ThreadPoolWorker *pMySelf)
{
	auto pManager = pMySelf->pManager; // !must save copy
	bool bReady   = false;
	ThreadPoolTask task = { 0 };
	intptr_t       currentTasksCount;
	while(!pMySelf->IsAborting() && !pMySelf->pManager->IsDestroying() && 
		(pManager->GetThreads() <= pManager->GetMaxThreads()))
	{
		Synchronized(pManager->m_tasks, {
			currentTasksCount = pManager->m_tasks.Count;
			if (currentTasksCount > 0) task = pManager->m_tasks.Dequeue();
		});
		assert(currentTasksCount >= 0);

		if (currentTasksCount > 0) {
			if (bReady) {
				// removes avail state if set
				Synchronized(pManager->m_avail_workers, {
					foreachList(start, pManager->m_avail_workers) {
						if (*start == pMySelf) {
							pManager->m_avail_workers.Remove(start);
							bReady = false;
							break;
						} //if

						// by the way, alerts others
						if (currentTasksCount > 1 && (*start)->pThread->Alert()) --currentTasksCount;
					}
				});
				assert(bReady != true || !"unexpected avail state " RLIB_LINE);
			} //if

			// executes task
			assert(task.ExecutionTarget != nullptr);
			task.ExecutionTarget(task.ExecutionParameter);
			continue;
		} //if		

		// no tasks, check if we need exit
		if (pMySelf->IsAborting() || 
			pMySelf->pManager->IsDestroying() || 
			pManager->GetThreads() > pManager->GetMaxThreads()
			) {
			break;
		} //if
		
		// sets ready state
		if (!bReady) {
			pManager->m_avail_workers.Lock();
			if (pManager->m_avail_workers.Add(pMySelf) != nullptr) {
				bReady = true;
			} //if
			pManager->m_avail_workers.UnLock();
		} //if

		// max idle time out, sleep
		while (pMySelf->pThread->Sleep(static_cast<DWORD>(pManager->GetMaxIdleTime())) == WAIT_TIMEOUTED)
		{
			if (pMySelf->IsAborting() || pMySelf->pManager->IsDestroying()) {
				goto __remove_worker;
			} //if

			if (pManager->GetTasks() <= 0 && pManager->GetThreads() > pManager->GetMinThreads()) {
				goto __remove_worker;
			} //if
			
			if (pManager->m_nPending == 0) break; // break sleep
		}
		// WAIT_ALERTED
	}

__remove_worker:
	pManager->RemoveWorker(pMySelf, bReady);

	Thread::ExitThread();
}