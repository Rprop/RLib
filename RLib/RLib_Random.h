/********************************************************************
	Created:	2012/11/17  13:44
	Filename: 	RLib_Random.h
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#include "RLib_AppBase.h"

#ifndef ULONG_MAX
# define ULONG_MAX 0xffffffffUL 
#endif // !ULONG_MAX

//-------------------------------------------------------------------------

namespace System
{
	/// <summary>
	/// Represents a pseudo-random number generator, a device that produces a sequence of numbers that
	/// meet certain statistical requirements for randomness
	/// </summary>
	class RLIB_API Random
	{
	protected:
		ULONG m_seed;

	public:
		Random();
		Random(ULONG seed);
		~Random() = default;

	public: // RTL random algorithm, considers to be faster and produces better random numbers
		/// <summary>
		/// Returns a random number in the range [0..MAXLONG-1]
		/// </summary>
		ULONG NextLong(); 
		/// <summary>
		/// Returns values that are uniformly distributed over the range from zero to the maximum possible LONG value less 1,
		/// if it is called repeatedly with the same Seed
		/// </summary>
		/// <returns>A signed integer greater than or equal to zero and less than MAXLONG</returns>
		static ULONG NextLong(_Inout_ PULONG Seed);
		/// <summary>
		/// Returns a random number within a specified range
		/// </summary>
		/// <param name="minValue">The inclusive lower bound of the random number returned</param>
		/// <param name="maxValue">The exclusive upper bound of the random number returned</param>
		/// <returns>A signed integer greater than or equal to minValue and less than maxValue</returns>
		template<typename T = intptr_t> T NextLong(T minValue, const T maxValue)
		{
			return static_cast<T>(static_cast<T>(static_cast<long double>(this->NextLong()) / MAXLONG * (maxValue - minValue)) + minValue);
		}

	public: // CRT random algorithm
		/// <summary>
		/// Gets a time-dependent default seed value
		/// </summary>
		static unsigned int GetDefaultSeed();
		/// <summary>
		/// Uses the system clock to provide a seed value.
		/// This is the most common way of instantiating the random number generator
		/// </summary>
		static void Srand();
		/// <summary>
		/// Uses the specified seed value
		/// </summary>
		/// <param name="Seed">A number used to calculate a starting value for the pseudo-random number sequence</param>
		static void Srand(unsigned int seed);
		/// <summary>
		/// Returns a nonnegative random number
		/// </summary>
		/// <returns>A signed integer greater than or equal to zero and less than RAND_MAX(32767)</returns>
		static int NextInt();
		/// <summary>
		/// Returns a random number within a specified range using a time-dependent default seed value
		/// </summary>
		/// <param name="minValue">The inclusive lower bound of the random number returned</param>
		/// <param name="maxValue">The exclusive upper bound of the random number returned</param>
		/// <returns>A signed integer greater than or equal to minValue and less than maxValue</returns>
		template<typename T = intptr_t> static T Next(T minValue, const T maxValue)
		{
			/*
			 *	Generate random numbers in the half-closed interval
			 *  [range_min, range_max)
			 */
			return static_cast<T>(static_cast<T>(static_cast<long double>(NextInt()) / (RAND_MAX + 1) * (maxValue - minValue)) + minValue);
		}
	};
}