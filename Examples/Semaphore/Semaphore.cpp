/********************************************************************
	Created:	2016/07/09  20:19
	Filename: 	Semaphore.cpp
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#include <RLib_Import.h>
#include <stdio.h>

//-------------------------------------------------------------------------
// A semaphore that simulates a limited resource pool.
//
static Semaphore *_pool;

// A padding interval to make the output more orderly.
volatile static long _padding;

//-------------------------------------------------------------------------

class Example
{
public:
	static void Main()
	{
		// Create a semaphore that can satisfy up to three
		// concurrent requests. Use an initial count of zero,
		// so that the entire semaphore count is initially
		// owned by the main program thread.
		//
		_pool = new Semaphore( 0,3 );

		// Create and start five numbered threads.
		//
		for ( intptr_t i = 1; i <= 5; i++ )
		{
			Thread *t = new Thread(Worker, (LPVOID)i);

			t->IsBackground = true;

			// Start the thread
			//
			t->Start();
		}

		// Wait for half a second, to allow all the
		// threads to start and to block on the semaphore.
		//
		Thread::Sleep( 500 );

		// The main thread starts out holding the entire
		// semaphore count. Calling Release(3) brings the
		// semaphore count back to its maximum value, and
		// allows the waiting threads to enter the semaphore,
		// up to three at a time.
		//
		printf( "Main thread calls Release(3).\n" );
		_pool->Release( 3 );

		printf( "Main thread exits.\n" );
	}

private:
	static void Worker( LPVOID num )
	{
		// Each worker thread begins by requesting the
		// semaphore.
		printf( "Thread {%d} begins and waits for the semaphore.\n", int(intptr_t(num)) );
		_pool->WaitOne();

		// A padding interval to make the output more orderly.
		int padding = Interlocked::Add( &_padding, 100L );

		printf( "Thread {%d} enters the semaphore.\n", int(intptr_t(num)));

		// The thread's "work" consists of sleeping for
		// about a second. Each thread "works" a little
		// longer, just to make the output more orderly.
		//
		Thread::Sleep( 1000 + padding );

		printf( "Thread {%d} releases the semaphore.\n", int(intptr_t(num)));
		printf( "Thread {%d} previous semaphore count: {%ld}\n",
			   int(intptr_t(num)), _pool->Release() );
	}
};
//-------------------------------------------------------------------------
int main()
{
	Example::Main();
	Thread::Sleep(8888);
	RLIB_Delete(_pool);
	return 0;
}