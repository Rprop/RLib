/********************************************************************
	Created:	2015/05/01  13:43
	Filename: 	RLib_PriorityQueue.h
	Author:		Volkan Yazıcı <volkan.yazici@gmail.com>
	Url:	    http://blog.csdn.net/rrrfff
*********************************************************************/
#ifndef _USE_PQUEUE
#define  _USE_PQUEUE
#include "RLib_CollectionGeneric.h"

#define __pq_left(i)   ((i) << 1)
#define __pq_right(i)  (((i) << 1) + 1)
#define __pq_parent(i) ((i) >> 1)

namespace System
{
	namespace Collections
	{
		namespace Generic
		{
			/// <summary>
			/// Priority Queue
			/// </summary>
			template <typename data_t, typename pqueue_pri_t = unsigned long long, class allocator = IO::IAllocator>
			class PriorityQueue
			{
			public:
				/** callback functions to get/set/compare the priority of an element */
				typedef pqueue_pri_t(*pqueue_get_pri_f)(const data_t *a);
				typedef void(*pqueue_set_pri_f)(data_t *a, pqueue_pri_t pri);
				typedef int(*pqueue_cmp_pri_f)(pqueue_pri_t next, pqueue_pri_t curr);

				/** callback functions to get/set the position of an element */
				typedef size_t(*pqueue_get_pos_f)(const data_t *a);
				typedef void(*pqueue_set_pos_f)(data_t *a, size_t pos);

				/** callback function to iterate a entry */
				typedef int(*pqueue_iterator_f)(data_t *a, const data_t *user_data);

			protected:
				void bubble_up(size_t i)
				{
					void *moving_node = this->d[i];
					pqueue_pri_t moving_pri = this->getpri(moving_node);

					for (auto parent_node = __pq_parent(i);
						 ((i > 1) && this->cmppri(this->getpri(this->d[parent_node]), moving_pri));
						 i = parent_node, parent_node = __pq_parent(i)) {
						this->d[i] = this->d[parent_node];
						this->setpos(this->d[i], i);
					}

					this->d[i] = moving_node;
					this->setpos(moving_node, i);
				}
				size_t maxchild(size_t i)
				{
					size_t child_node = __pq_left(i);

					if (child_node >= this->size) {
						return 0;
					} //if

					if ((child_node + 1) < this->size &&
						this->cmppri(this->getpri(this->d[child_node]), this->getpri(this->d[child_node + 1]))) {
						child_node++;    /* use right child instead of left */
					} //if

					return child_node;
				}
				void percolate_down(size_t i)
				{
					size_t child_node;
					void *moving_node = this->d[i];
					pqueue_pri_t moving_pri = this->getpri(moving_node);

					while ((child_node = maxchild(q, i)) &&
						   this->cmppri(moving_pri, this->getpri(this->d[child_node]))) {
						this->d[i] = this->d[child_node];
						this->setpos(this->d[i], i);
						i = child_node;
					}

					this->d[i] = moving_node;
					this->setpos(moving_node, i);
				}
				int subtree_is_valid(int pos)
				{
					if (__pq_left(pos) < this->size) {
						/* has a left child */
						if (this->cmppri(this->getpri(this->d[pos]), this->getpri(this->d[__pq_left(pos)]))) {
							return 0;
						} //if
						if (!subtree_is_valid(q, __pq_left(pos))) {
							return 0;
						} //if
					}
					if (__pq_right(pos) < this->size) {
						/* has a right child */
						if (this->cmppri(this->getpri(this->d[pos]), this->getpri(this->d[__pq_right(pos)]))) {
							return 0;
						} //if
						if (!subtree_is_valid(q, __pq_right(pos))) {
							return 0;
						} //if
					}
					return 1;
				}
				/** the priority queue handle */
#pragma warning(push)
#pragma warning(disable:4201)
				struct{
					size_t size;                /**< number of elements in this queue */
					size_t avail;               /**< slots available in this queue */
					size_t step;                /**< growth stepping setting */
					pqueue_cmp_pri_f cmppri;    /**< callback to compare nodes */
					pqueue_get_pri_f getpri;    /**< callback to get priority of a node */
					pqueue_set_pri_f setpri;    /**< callback to set priority of a node */
					pqueue_get_pos_f getpos;    /**< callback to get position of a node */
					pqueue_set_pos_f setpos;    /**< callback to set position of a node */
					data_t **d;                 /**< The actualy queue in binary heap form */
				};
#pragma warning(pop)

			public:
				RLIB_DECLARE_DYNCREATE;
				/**
				* initialize the queue
				*
				* @param n the initial estimate of the number of queue items for which memory
				*     should be preallocated
				* @param cmppri The callback function to run to compare two elements
				*     This callback should return 0 for 'lower' and non-zero
				*     for 'higher', or vice versa if reverse priority is desired
				* @param setpri the callback function to run to assign a score to an element
				* @param getpri the callback function to run to set a score to an element
				* @param getpos the callback function to get the current element's position
				* @param setpos the callback function to set the current element's position
				*/
				PriorityQueue(size_t n,
							  pqueue_cmp_pri_f cmppri,
							  pqueue_get_pri_f getpri,
							  pqueue_set_pri_f setpri,
							  pqueue_get_pos_f getpos,
							  pqueue_set_pos_f setpos)
				{
					/* Need to allocate n+1 elements since element 0 isn't used. */
					if ( !(this->d = allocator::allocateMemory((n + 1) * sizeof(data *))) ) {
						this->size = this->avail = this->step = 0;
						return;
					} //if

					this->size   = 1;
					this->avail  = this->step = (n + 1); /* see comment above about n+1 */
					this->cmppri = cmppri;
					this->setpri = setpri;
					this->getpri = getpri;
					this->getpos = getpos;
					this->setpos = setpos;
				}
				/**
				* free all memory used by the queue
				*/
				~PriorityQueue() {
					if (this->d != nullptr) {
						allocator::freeMemory(this->d);
						//this->size = this->avail = this->step = 0;
					} //if
				}
				/**
				* return the size of the queue.
				*/
				size_t pqueue_size() {
					/* queue element 0 exists but doesn't count since it isn't used. */
					return this->size - 1;
				}
				/**
				* insert an item into the queue.
				* @param d the item
				* @return 0 on success
				*/
				int pqueue_insert(data_t *d)
				{
					/* allocate more memory if necessary */
					if (this->size >= this->avail) {
						auto newsize = this->size + this->step;
						auto tmp = 
							reinterpret_cast<data_t **>(allocator::reallocateMemory(this->d, sizeof(data_t *) * newsize));
						if (tmp == nullptr) {
							return 1;
						} //if
						this->d     = tmp;
						this->avail = newsize;
					}

					/* insert item */
					auto i     = this->size++;
					this->d[i] = d;
					this->bubble_up(i);

					return 0;
				}
				/**
				* move an existing entry to a different priority
				* @param new_pri the new priority
				* @param d the entry
				*/
				void pqueue_change_priority(pqueue_pri_t new_pri, data_t *d) {
					pqueue_pri_t old_pri = this->getpri(d);

					this->setpri(d, new_pri);
					auto posn = this->getpos(d);
					if (this->cmppri(old_pri, new_pri)) {
						this->bubble_up(posn);
					} else {
						this->percolate_down(posn);
					} //if
				}
				/**
				* pop the highest-ranking item from the queue.
				* @return NULL on error, otherwise the entry
				*/
				void *pqueue_pop() {
					if (this->size <= 1) {
						return nullptr;
					} //if

					auto head = this->d[1];
					this->d[1] = this->d[--this->size];
					this->percolate_down(q, 1);

					return head;
				}
				/**
				* remove an item from the queue.
				* @param d the entry
				* @return 0 on success
				*/
				int pqueue_remove(data_t *d) {
					size_t posn   = this->getpos(d);
					this->d[posn] = this->d[--this->size];
					if (this->cmppri(this->getpri(d), this->getpri(this->d[posn]))) {
						this->bubble_up(q, posn);
					} else {
						this->percolate_down(q, posn);
					} //if

					return 0;
				}
				/**
				* access highest-ranking item without removing it.
				* @return NULL on error, otherwise the entry
				*/
				void *pqueue_peek() {
					if (this->size <= 1) {
						return nullptr;
					} //if
					return this->d[1];
				}
				/**
				* iterate the queue
				* @param the callback function to iterate the entry
				*/
				void *pqueue_iterate(pqueue_iterator_f iterator, const data_t *user_data) {
					for (int i = 1; i < this->size; ++i) {
						if (iterator(this->d[i], user_data) > 0) {
							return this->d[i];
						} //if
					}
					return nullptr;
				}

			private:
#ifdef _DEBUG
				/**
				* checks that the pq is in the right order, etc
				* @internal
				* debug function only
				*/
				int pqueue_is_valid() {
					return this->subtree_is_valid(1);
				}
				/**
				* dump the queue and it's internal structure
				* @internal
				* debug function only
				*/
				void pqueue_dump() {
					fprintf(stdout, "posn\tleft\tright\tparent\tmaxchild\t...\n");
					for (int i = 1; i < q->size; ++i) {
						fprintf(stdout,
								"%d\t%d\t%d\t%d\t%ul\t",
								i,
								left(i), right(i), parent(i),
								static_cast<unsigned int>(maxchild(i)));
					}
				}
#endif // _DEBUG
			};

		}
	}
}

#endif // !_USE_PQUEUE

