/********************************************************************
	Created:	2014/06/30  19:34
	Filename: 	RLib_StringArray.h
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#ifndef _USE_STRINGARRAY
#define _USE_STRINGARRAY

#include "RLib_String.h"
#include "RLib_Array.h"

//-------------------------------------------------------------------------
namespace System
{
	/// <summary>
	/// Represents a collection of strings (specialized Array&lt;String&gt;)
	/// </summary>
	class RLIB_API StringArray : public Collections::Generic::Array<String>
	{
	public:
		StringArray(intptr_t length = RLIB_DEFAULT_CAPACITY) : Array(length) {}
		StringArray(const String items[], intptr_t length) : Array(items, length) {}
		StringArray(const StringArray &) = default;
		~StringArray() = default;
		RLIB_DECLARE_DYNCREATE;

	public:
		/// <summary>
		/// Concatenates all the elements of the string array, using the specified separator between each element
		/// </summary>
		String Join(const String &separator);
	};
}
#endif // _USE_STRINGARRAY