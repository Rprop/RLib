/********************************************************************
	Created:	2015/01/20  14:28
	Filename: 	RLib_HashTable.h
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#ifndef _USE_HASHTABLE
#define _USE_HASHTABLE
#include "RLib_HashMap.h"
#include "RLib_SafeObject.h"

//-------------------------------------------------------------------------

namespace System
{
	namespace Collections
	{
		namespace Generic
		{
			template<typename K, typename V, class hasher = IHash<K, __int64>, class allocator = IO::IAllocator, typename kdisposer = IDisposable<K>, typename vdisposer = IDisposable<V>>
			using HashTable = Threading::Synchronizable<HashMap<K, V, hasher, allocator, kdisposer, vdisposer>>;
		}
	}
}

#define _USE_HASHTABLE
#endif // _USE_HASHTABLE