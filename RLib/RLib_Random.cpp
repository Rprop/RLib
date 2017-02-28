/********************************************************************
	Created:	2012/11/17  13:44
	Filename: 	RLib_Random.cpp
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#include "RLib_Random.h"
#include <stdlib.h> // for rand, srand
#if (WINVER > _WIN32_WINNT_WIN7)
# include <sysinfoapi.h>
#endif

//-------------------------------------------------------------------------

unsigned int Random::GetDefaultSeed()
{
#if (WINVER <= _WIN32_WINNT_WIN7)
	return static_cast<unsigned int>(GetTickCount());
#else
	return static_cast<unsigned int>(GetTickCount64());
#endif
}

//-------------------------------------------------------------------------

void Random::Srand()
{
	srand(Random::GetDefaultSeed());
}

//-------------------------------------------------------------------------

void Random::Srand(unsigned int seed)
{
	srand(seed);
}

//-------------------------------------------------------------------------

int Random::NextInt()
{
	return rand();
}

//-------------------------------------------------------------------------

ULONG Random::NextLong(_Inout_ PULONG Seed)
{
	ULONG rv = RtlRandomEx(Seed);
#if (WINVER > _WIN32_WINNT_WIN7)
	if (rv > MAXLONG - 1UL) rv &= (MAXLONG - 1UL);
#endif
	assert(rv <= MAXLONG - 1UL);
	return static_cast<ULONG>(rv);
}

//-------------------------------------------------------------------------

Random::Random() : m_seed(Random::GetDefaultSeed())
{
}

//-------------------------------------------------------------------------

Random::Random(ULONG seed) : m_seed(seed)
{
}

//-------------------------------------------------------------------------

ULONG Random::NextLong()
{
	return Random::NextLong(&this->m_seed);
}