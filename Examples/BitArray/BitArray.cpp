/********************************************************************
	Created:	2016/07/11  14:37
	Filename: 	BitArray.cpp
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#include <RLib_Import.h>
using namespace System::Collections;

//-------------------------------------------------------------------------

int main()
{
	BitArray bits(1024);

	bool a = !bits[79];
	bool b = (bits[66] = true);
	assert(a == b);

	return 0;
}