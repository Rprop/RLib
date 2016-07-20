/********************************************************************
	Created:	2012/05/26  16:08
	Filename: 	ThreadPool.cpp
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#include <RLib_Import.h>
#include <stdio.h>

//-------------------------------------------------------------------------

static ThreadPool *pool;

//-------------------------------------------------------------------------

class Example
{
public:
	// This thread procedure performs the task.
	static void ThreadProc( LPVOID stateInfo )
	{
		printf( "Hello from the thread pool.\n" );
		Sleep(100);

		Int64 i  = reinterpret_cast<intptr_t>(stateInfo);
		Int32 t  = AppBase::GetCurrentThreadId();
		String s = _R("ID: ") + i.ToString() + _R("  线程: ") + t.ToString() +
			_R("  总数: ") + Int64(pool->GetThreads()).ToString() +
			_R("  空闲: ") + Int64(pool->GetAvailableThreads()).ToString() +
			_R("  最大: ") + Int64(pool->GetMaxThreads()).ToString() +
			_R("  最小: ") + Int64(pool->GetMinThreads()).ToString() +
			_R("\n");
		printf("%s", GlobalizeString(s).toGBK());
	}
};

//-------------------------------------------------------------------------

int main()
{
	pool = new ThreadPool(1, 10);
	for (intptr_t i = 100; i < 20000; ++i) {
		// Queue the task.
		pool->AddTask(Example::ThreadProc, reinterpret_cast<int *>(i));

		if (i < 1000) pool->SetMaxThreads(i / 10);
	}

	// If you comment out the Sleep, the main thread exits before
	// the thread pool task runs. 
	Thread::Sleep(6000);
	delete pool;

	return 0;
}