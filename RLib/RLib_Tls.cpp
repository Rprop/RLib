/********************************************************************
	Created:	2015/01/20  13:53
	Filename: 	RLib_Tls.cpp
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#include "RLib_Tls.h"
#ifdef _USE_TLS
#if (WINVER > _WIN32_WINNT_WIN7)
# include <processthreadsapi.h>
#else
# include <windows.h>
#endif
using namespace System::Threading;

//-------------------------------------------------------------------------

void *TlsItem::GetValue() const
{
	return TlsGetValue(static_cast<DWORD>(this->tlsIndex));
}

//-------------------------------------------------------------------------

void TlsItem::SetValue(const void *obj)
{
	TlsSetValue(static_cast<DWORD>(this->tlsIndex), const_cast<void *>(obj));
}

//-------------------------------------------------------------------------

TlsItem Tls::AllocateLocallyUniqueId()
{
	return TlsItem(static_cast<TlsId>(TlsAlloc()));
}

//-------------------------------------------------------------------------

void Tls::Finalize(TlsId id)
{
	if (id != TLS_OUT_OF_INDEXES){
		TlsFree(static_cast<DWORD>(id));
	}
}
#endif
