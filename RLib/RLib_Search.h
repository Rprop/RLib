/********************************************************************
	Created:	2015/09/27  16:24
	Filename: 	RLib_Search.h
	Author:		rrrfff
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#pragma once
#include "RLib_CollectionGeneric.h"

namespace System
{
	namespace Generic
	{
		/// <summary>
		/// Provides several static methods for searching
		/// </summary>
		namespace Search
		{
			/// <summary>
			/// 顺序搜索法(Sequential Search)
			/// 优点: 该方法对数据无事先排序(Sorting)要求, 容易实现
			/// 缺点: 搜索效率差(平均搜索 (N + 1) / 2 次), 不管是否排序, 每次都必须从头到尾遍历一次
			/// 时间复杂度: 
			///		如果没有重复数据，线性查找在找到任一元素时就终止，否则需要全部遍历. 在最差情況下, 需作N次比较, O(N)
			///		在平均狀況下(各元素出现与分布的概率相等)需 (N + 1) / 2次比较，所以最差时间为O(N), 最好时间为O(1) = 1次
			/// </summary>
			template<
				typename R = intptr_t,
				typename Collections::Generic::IComparer<R>::EqualsDelegate equals = Collections::Generic::IComparer<R>::Equals>
			intptr_t sequential_search(const R list[], intptr_t _lower, intptr_t _upper, const R &key)
			{
				while (_lower <= _upper) {
					if (equals(&list[_lower], &key)) return _lower;
					++_lower;
				}

				// not found
				return -1;
			}
			template<typename R = intptr_t>
			intptr_t sequential_search(const R list[], intptr_t _lower, intptr_t _upper, const R &key,
									   typename Collections::Generic::IComparer<R>::EqualsDelegate equals)
			{
				while (_lower <= _upper) {
					if (equals(&list[_lower], &key)) return _lower;
					++_lower;
				}

				// not found
				return -1;
			}
			template<
				typename R = intptr_t, 
				typename Collections::Generic::IComparer<R>::EqualsDelegate equals = Collections::Generic::IComparer<R>::Equals>
			intptr_t sequential_search_end(const R list[], intptr_t _lower, intptr_t _upper, const R &key)
			{
				while (_upper >= _lower) {
					if (equals(&list[_upper], &key)) return _upper;
					--_upper;
				}

				// not found
				return -1;
			}
			template<typename R = intptr_t>
			intptr_t sequential_search_end(const R list[], intptr_t _lower, intptr_t _upper, const R &key, 
										   typename Collections::Generic::IComparer<R>::EqualsDelegate equals)
			{
				while (_upper >= _lower) {
					if (equals(&list[_upper], &key)) return _upper;
					--_upper;
				}

				// not found
				return -1;
			}
				
			//-------------------------------------------------------------------------

			/// <summary>
			/// 二分搜索法(Binary Search)
			/// 优点: 搜索效率优(平均搜索 Log2N 次)
			/// 缺点: 数据必需事先排序且必需可直接随机存取
			/// 时间复杂度: 
			///		每次比较都会比上一次少一半数据, 2^x = N, x = Log2N, 因此平均时间 O(LogN)
			/// </summary>
			template<
				typename R = intptr_t, 
				typename Collections::Generic::IComparer<R>::Delegate compare = Collections::Generic::IComparer<R>::Compare> 
			intptr_t binary_search(const R list[], intptr_t _lower, intptr_t _upper, const R &key)
			{
				intptr_t _mean;
				while (_lower <= _upper) {
					_mean = (_lower + _upper) >> 1;
					
					auto r = compare(&list[_mean], &key);
					if (r == 0 ) {
						return _mean;
					} //if

					if (r > 0) {
						_upper = _mean - 1;
					} else {
						_lower = _mean + 1;
					} //if
				}

				// not found
				return -1;
			}
			template<typename R = intptr_t>
			intptr_t binary_search(const R list[], intptr_t _lower, intptr_t _upper, const R &key,
								   typename Collections::Generic::IComparer<R>::Delegate compare)
			{
				intptr_t _mean;
				while (_lower <= _upper) {
					_mean = (_lower + _upper) >> 1;

					auto r = compare(&list[_mean], &key);
					if (r == 0) {
						return _mean;
					} //if

					if (r > 0) {
						_upper = _mean - 1;
					} else {
						_lower = _mean + 1;
					} //if
				}

				// not found
				return -1;
			}
			template<typename R = intptr_t, typename Collections::Generic::IComparer<R>::Delegate compare = Collections::Generic::IComparer<R>::Compare>
			intptr_t binary_search_recursively(const R *list, intptr_t _lower, intptr_t _upper, const R &key)
			{
				assert(_lower <= _upper);
				intptr_t _mean = (_lower + _upper) >> 1;

				auto r = compare(&list[_mean], &key);
				if (r == 0) {
					return _mean;
				} //if

				if (_lower == _upper) {			
					return -1; // not found
				} //if

				if (r > 0) {
					return binary_search_recursively(list, _lower, _mean - 1, key);
				} //if
				return binary_search_recursively(list, _mean + 1, _upper, key);
			}
			
			//-------------------------------------------------------------------------

			/// <summary>
			/// 插值搜索法(Interpolation Search)
			/// 定义: 二分搜索法的一种改进, 依照数据位置分布, 运用插值预测数据所在位置, 再以二分法方式逼近
			///       插值是离散函数逼近的重要方法, 利用它可通过函数在有限个点处的取值状况, 估算出函数在其他点处的近似值
			/// 优点: 数据分布均匀时搜索速度极快
			/// 缺点: 数据必需事先排序且需计算预测公式
			/// 时间复杂度: 
			///		取决于数据分布情形, 平均时间 O(LogLogN) 优于 二分搜索 O(LogN)
			/// </summary>
			template<typename R = intptr_t> intptr_t interpolation_search(const R *list, intptr_t _lower, intptr_t _upper, const R &key)
			{
				intptr_t _mean;
				while (_lower <= _upper) {
					// predictor
					_mean = (_upper - _lower) * (key - list[_lower]) / (list[_upper] - list[_lower]) + _lower;
					if (_mean < _lower || _mean > _upper) {
						// mean misprediction
						break;
					} //if

					if (list[_mean] == key) {
						return _mean;
					} //if

					if (key < list[_mean]) {
						_upper = _mean - 1;
					} else {
						_lower = _mean + 1;
					} //if
				}

				// not found
				return -1;
			}

			//-------------------------------------------------------------------------

			/// <summary>
			/// 斐波那契搜索法(Fbonacci Search)
			/// 定义: 利用斐波那契数列的性质(黄金分割原理)预测数据所在位置, 再以二分法方式逼近
			/// 优点: 只涉及加法和减法运算
			/// 缺点: 数据必需事先排序且需计算或者预定义斐波那契数列
			/// 时间复杂度: 
			///		同于二分搜索 O(LogN), 平均情况下, 斐波那契查找优于二分查找, 但最坏情况下则劣于二分查找
			/// </summary>
			template<typename R = intptr_t> intptr_t fbonacci_search(const R *list, intptr_t _lower, intptr_t _upper, const R &key)
			{
				// 预定义斐波那契数列, 代码计算可参考http://blog.csdn.net/rrrfff/article/details/6848700
				static intptr_t F[] = { 0, 1, 1, 2, 3, 5, 8, 13, 21, 34, 55, 89, 144 };

				intptr_t _mean;

				// 计算 n 在斐波那契数列中位置
				intptr_t _index = 0;
				while (n > F[_index] - 1)
					++_index;

				// 斐波那契搜索要求 n == F[_index] - 1
				R *_list;
				intptr_t _count = F[_index] - 1;
				if (n != _count) {
					assert(n < _count);
					_list = new R[_count];
					memcpy_s(_list, _count * sizeof(R), list, n * sizeof(R));
					// 填补长度, _count则为填充后的长度
					for (intptr_t i = n; i < _count; ++i)
						_list[i] = _list[n - 1];
				} else {
					_list = const_cast<R *>(list);
				} //if


				while (_lower <= _upper) {
					// 利用斐波那契数列确定下一个要比较的元素位置
					_mean = _lower + F[_index - 1] - 1;

					if (key == _list[_mean]) {
						if (_list != list) delete[] _list;

						if (_mean <= n - 1)
							return _mean;
						else
							return n - 1;
					} //if

					if (key < _list[_mean]) {
						_upper = _mean - 1; // 待查找的元素在[_lower, _mean -1]范围内
						// 此时范围[_lower, _mean -1]内的元素个数为 F[_index - 1] - 1
						// 所以
						--_index;
					} else {
						_lower = _mean + 1; // 待查找的元素在[_mean + 1, _upper]范围内
						// 此时范围[_lower, _upper]内的元素个数为 _count - F[_index - 1] = 
						//  (F[_index] - 1) - F[_index - 1] = (F[_index] - F[_index - 1]) - 1 = 
						//   F(_index - 2) - 1个(因为 F(n) = F(n - 1) + F(n - 2)(n≥2, n∈N*))
						// 所以
						_index -= 2;
					} //if
				}

				if (_list != list) {
					delete[] _list;
				} //if

				// not found
				return -1;
			}

			//-------------------------------------------------------------------------
			
			/// <summary>
			/// 二叉树搜索
			/// </summary>
			template<typename R = intptr_t> intptr_t tree_search(const R *list, intptr_t _lower, intptr_t _upper, const R &key)
			{
				RLIB_NODEFAULT("not implemented!");
			}
		};
	};
};