/********************************************************************
	Created:	2014/07/14  19:46
	Filename: 	RLib_CollectionGeneric.h
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#ifndef _USE_GENERIC
#define _USE_GENERIC
#include "RLib_IAllocator.h"
#include "RLib_Standard.h"

#define foreachEx(a, b, i)  RLIB_TYPE(b.GetType()) a = (b.Length > 0 ? &b[0] : nullptr); for (intptr_t i = 0; i < b.Length; ++i, a = (i != b.Length) ? &b[i] : nullptr)
#define foreach(a, b)       foreachEx(a, b, i)
#define foreachpEx(a, b, i) RLIB_TYPE(b->GetType()) a = (b->Length > 0 ? &((*b)[0]) : nullptr); for (intptr_t i = 0; i < b->Length; ++i, a = (i != b->Length) ? &((*b)[i]) : nullptr)
#define foreachp(a, b)      foreachpEx(a, b, i)

//-------------------------------------------------------------------------

namespace System
{
	/// <summary>
	/// The namespace contains interfaces and classes that define various collections of objects,
	/// such as lists, queues, arrays, hash tables and dictionaries.
	/// </summary>
	namespace Collections
	{
		/// <summary>
		/// The namespace contains interfaces and classes that define generic collections, 
		/// which allow users to create strongly typed collections that provide better type safety 
		/// and performance than non-generic strongly typed collections.
		/// </summary>
		namespace Generic
		{
			/// <summary>
			/// Defines a method that a type implements to compare two objects
			/// </summary>
			template <class R> class IComparer
			{
			public:
				using Delegate = int(*)(const R *, const R *);
				/// <summary>
				/// Compares two objects and returns a value indicating whether one is less than,
				/// equal to, or greater than the other
				/// </summary>
				static RLIB_INLINE int Compare(const R *p1, const R *p2)
				{
					if (*p1 > *p2) return 1;
					if (*p1 < *p2) return -1;
					return 0;
				}
				using EqualsDelegate = bool(*)(const R *, const R *);
				/// <summary>
				/// Compares two objects and returns a value indicating whether one is equal to the other
				/// </summary>
				static RLIB_INLINE bool Equals(const R *p1, const R *p2)
				{
					return *p1 == *p2;
				}
			};
			/// <summary>
			/// Defines a method that a type implements to compare two objects
			/// </summary>
			template <class R> class IComparerByDescending : public IComparer<R>
			{
			public:
				/// <summary>
				/// Compares two objects and returns a value indicating whether one is greater than,
				/// equal to, or less than the other
				/// </summary>
				static RLIB_INLINE int Compare(const R *p1, const R *p2)
				{
					if (*p1 > *p2) return -1;
					if (*p1 < *p2) return 1;
					return 0;
				}
			};
			/// <summary>
			/// Defines a method that a type implements to dispose
			/// </summary>
			template <class R> class IDisposable
			{
			public:	
				using Delegate = void(*)(R *);
				/// <summary>
				/// default constructor caller
				/// </summary>
				static RLIB_INLINE void Dispose(R *obj) {
					UNREFERENCED_PARAMETER(obj);
					obj->~R();
				}
			};
		}
	}
}
#endif // _USE_GENERIC