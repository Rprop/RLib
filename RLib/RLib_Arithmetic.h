/********************************************************************
	Created:	2015/09/27  16:10
	Filename: 	RLib_Arithmetic.h
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#pragma once
#include "RLib_AppBase.h"

namespace System
{
	namespace Generic
	{
		/// <summary>
		/// Template for arithmetic type (either an integral or a floating point type)
		/// </summary>
		template <class Type> class Arithmetic
		{
		public:
			Type m_var;

		public:
			Arithmetic(const Type &t = 0) { this->m_var = t; }
			Arithmetic(const Arithmetic&c) { this->m_var = c.m_var; }

			operator Type &() { return static_cast<Type &>(this->m_var); }
			operator const Type() const { return static_cast<const Type>(this->m_var); }

			Arithmetic &operator = (const Type &t) { this->m_var = t; return *this; }
			Arithmetic &operator += (const Type &t) { this->m_var += t; return *this; }
			Arithmetic &operator -= (const Type &t) { this->m_var -= t; return *this; }
			Arithmetic &operator *= (const Type &t) { this->m_var *= t; return *this; }
			Arithmetic &operator /= (const Type &t) { this->m_var /= t; return *this; }
			const Arithmetic operator + (const Type &t) const { return this->m_var + t; }
			const Arithmetic operator - (const Type &t) const { return this->m_var - t; }
			const Arithmetic operator * (const Type &t) const { return this->m_var * t; }
			const Arithmetic operator / (const Type &t) const { return this->m_var / t; }

			Arithmetic &operator ++ () { this->m_var++; return *this; } // prefix increment operator 
			Arithmetic &operator -- () { this->m_var--; return *this; }
			Arithmetic operator ++ (int) { Type t = this->m_var; this->m_var++; return t; } // postfix increment operator 
			Arithmetic operator -- (int) { Type t = this->m_var; this->m_var--; return t; }
	
// 			bool operator == (const Type &t) const { return this->m_var == t; }
// 			bool operator != (const Type &t) const { return this->m_var != t; }
// 			bool operator <  (const Type &t) const { return this->m_var < t; }
// 			bool operator <= (const Type &t) const { return this->m_var <= t; }
// 			bool operator >  (const Type &t) const { return this->m_var > t; }
// 			bool operator >= (const Type &t) const { return this->m_var >= t; }
		};
		/// <summary>
		/// Template for integral type
		/// </summary>
		template <class Type> class Integral : public Arithmetic<Type>
		{
		public:
			Integral(const Type &t = 0) : Arithmetic<Type>(t) {}
			Integral(const Integral&c) : Arithmetic<Type>(c) {}

			operator Type &() { return static_cast<Type &>(this->m_var); }
			operator const Type() const { return static_cast<const Type>(this->m_var); }
		
			Integral &operator %= (const Type &t) { this->m_var %= t; return *this; }
			Integral &operator ^=( const Type &t) { this->m_var ^= t; return *this; }
			Integral &operator &= (const Type &t) { this->m_var &= t; return *this; }
			Integral &operator |= (const Type &t) { this->m_var |= t; return *this; }
			Integral &operator >>= (const Type &t) { this->m_var >>= t; return *this; }
			Integral &operator <<= (const Type &t) { this->m_var <<= t; return *this; }
			Integral operator % (const Type &t) const { return this->m_var % t; }
			Integral operator ^ (const Type &t) const { return this->m_var ^ t; }
			Integral operator & (const Type &t) const { return this->m_var & t; }
			Integral operator | (const Type &t) const { return this->m_var | t; }
			Integral operator >> (const Type &t) const { return this->m_var >> t; }
			Integral operator << (const Type &t) const { return this->m_var << t; }
		};
	}
}