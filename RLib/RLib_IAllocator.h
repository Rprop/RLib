/********************************************************************
	Created:	2015/09/27  10:54
	Filename: 	RLib_IAllocator.h
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#pragma once
#include "RLib_AppBase.h"

namespace System
{
	namespace IO
	{
		/// <summary>
		/// IAllocator defines the memory allocator interface
		/// </summary>
		class RLIB_API RLIB_THREAD_SAFE IAllocator
		{
		private:
			IAllocator() = delete;
			~IAllocator() = delete;
		public:
			/// <summary>
			/// This method allocates a block of memory
			/// </summary>
			static void *allocateMemory(intptr_t cb RLIB_INTERNAL_DEBUG_PARAM);
			/// <summary>
			/// This method frees a previously allocated block of memory
			/// </summary>
			static void freeMemory(void *address);
			/// <summary>
			/// This method changes the size of a previously allocated memory block
			/// </summary>
			static void *reallocateMemory(void *address, intptr_t cb);
		public:
			/// <summary>
			/// return the default instance of IAllocator 
			/// </summary>
			static class IAllocator *getSharedInstance();
		};
	};
};