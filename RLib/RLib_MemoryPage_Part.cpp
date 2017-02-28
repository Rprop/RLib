/********************************************************************
	Created:	2014/07/01  14:34
	Filename: 	RLib_MemoryPage_Part.cpp
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#include "RLib_File.h"
#include "RLib_MemoryPage.h"
using namespace System::IO;

//-------------------------------------------------------------------------

void MemoryPage::DumpToFile(const TCHAR *lpszFile)
{
	auto lpfile = File::Create(lpszFile, FileMode::CreateNew);
	if (lpfile != nullptr) {
		lpfile->Write(this->m_base_ptr, this->m_now);
		delete lpfile;
	} //if
}