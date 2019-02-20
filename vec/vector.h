/*
https://github.com/eteran/c-vector
A dynamic array impl in C99
just use a Type * (or vector(Type) to make it clear it's a vec) and set it to NULL initially

Define LINEAR_GROWTH to add to capacity instead of multiplying it, and define GROWTH_FACTOR to change
the amount the capacity is added/multiplied by (defaults are 1 if LINEAR_GROWTH, 2 otherwise)

If LINEAR_GROWTH is not set, you can set MIN_CAPACITY to change the initial capacity of the vector.
Default 8.

interface:
    vector_set_capacity(vec, new_capacity)
	    set the capacity (allocated size) of the vector. Does not realloc, just changes recorded size
	vector_set_size(vec, new_size)
	    change recorded number of items in vector
	vector_size(vec)
	    get number of items in vector
	vector_capacity(vec)
	    get capacity of vector
	vector_empty(vec)
	    see if vector is empty
	vector_grow(vec, new_capacity)
	    change capacity of the vector. Realloc's
	vector_pop_back(vec)
	    deletes the last item of the vector. Does not return it
	vector_push_back(vec)
	    add an item to the vector. Handles realloc
	vector_back(vec)
	    get the last item in the vector (as lvalue)
    vector_free(vec)
	    free the vector
	vector_erase(vec, index)
	    delete an item from the vector
    vector_begin(vec)
	    get a pointer to the start of the data
	vector_end(vec)
	    get a pointer to the end of the data
*/

#ifndef VECTOR_H_
#define VECTOR_H_

#ifndef GROWTH_FACTOR
  #ifdef LINEAR_GROWTH
    #define GROWTH_FACTOR 1
  #else
    #define GROWTH_FACTOR 2
	#ifndef MIN_CAPACITY
	  #define MIN_CAPACITY 8
	#endif
  #endif
#endif

#include <stddef.h> /* for size_t */
#include <stdlib.h> /* for malloc/realloc/free */
#include <assert.h> /* for assert */

#define vector(t) t*

/**
 * @brief vector_set_capacity - For internal use, sets the capacity variable of the vector
 * @param vec - the vector
 * @param size - the new capacity to set
 * @return void
 */
#define vector_set_capacity(vec, size)   \
do {                                     \
	if(vec) {                            \
		((size_t *)(vec))[-1] = (size);  \
	}                                    \
} while(0)

/**
 * @brief vector_set_size - For internal use, sets the size variable of the vector
 * @param vec - the vector
 * @param size - the new capacity to set
 * @return void
 */
#define vector_set_size(vec, size)      \
do {                                    \
	if(vec) {                           \
		((size_t *)(vec))[-2] = (size); \
	}                                   \
} while(0)

/**
 * @brief vector_capacity - gets the current capacity of the vector
 * @param vec - the vector
 * @return the capacity as a size_t
 */
#define vector_capacity(vec) \
	((vec) ? ((size_t *)(vec))[-1] : (size_t)0)

/**
 * @brief vector_size - gets the current size of the vector
 * @param vec - the vector
 * @return the size as a size_t
 */
#define vector_size(vec) \
	((vec) ? ((size_t *)(vec))[-2] : (size_t)0)

/**
 * @brief vector_empty - returns non-zero if the vector is empty
 * @param vec - the vector
 * @return non-zero if empty, zero if non-empty
 */
#define vector_empty(vec) \
	((vec) ? vector_size(vec) == 0 : 0)

/**
 * @brief vector_grow - For internal use, ensures that the vector is at least <count> elements big
 * @param vec - the vector
 * @param size - the new capacity to set
 * @return void
 */
#define vector_grow(vec, count) \
do {                                                                                    \
	if(!(vec)) {                                                                        \
		size_t *__p = malloc((count) * sizeof(*(vec)) + (sizeof(size_t) * 2));          \
		assert(__p);                                                                    \
		(vec) = (void *)(&__p[2]);                                                      \
		vector_set_capacity((vec), (count));                                            \
		vector_set_size((vec), 0);                                                      \
	} else {                                                                            \
		size_t *__p1 = &((size_t *)(vec))[-2];                                          \
		size_t *__p2 = realloc(__p1, ((count) * sizeof(*(vec))+ (sizeof(size_t) * 2))); \
		assert(__p2);                                                                   \
		(vec) = (void *)(&__p2[2]);                                                     \
		vector_set_capacity((vec), (count));                                            \
	}                                                                                   \
} while(0)

/**
 * @brief vector_pop_back - removes the last element from the vector
 * @param vec - the vector
 * @return void
 */
#define vector_pop_back(vec) \
do {                                              \
	vector_set_size((vec), vector_size(vec) - 1); \
} while(0)

/**
 * @brief vector_back - get the last element of the vector
 * @param vec - the vector
 * @return the array type
 */
#define vector_back(vec) \
	vec[vector_size(vec)-1]

/**
 * @brief vector_erase - removes the element at index i from the vector
 * @param vec - the vector
 * @param i - index of element to remove
 * @return void
 */
#define vector_erase(vec, i) \
do {                                                   \
	if (vec) {                                         \
		const size_t __sz = vector_size(vec);          \
		if ((i) < __sz) {                              \
			vector_set_size((vec), __sz - 1);          \
			size_t __x;                                \
			for (__x = (i); __x < (__sz - 1); ++__x) { \
				(vec)[__x] = (vec)[__x + 1];           \
			}                                          \
		}                                              \
   }                                                   \
} while(0)

/**
 * @brief vector_free - frees all memory associated with the vector
 * @param vec - the vector
 * @return void
 */
#define vector_free(vec) \
do { \
	if(vec) {                                \
		size_t *p1 = &((size_t *)(vec))[-2]; \
		free(p1);                            \
	}                                        \
} while(0)

/**
 * @brief vector_begin - returns an iterator to first element of the vector
 * @param vec - the vector
 * @return a pointer to the first element (or NULL)
 */
#define vector_begin(vec) \
	(vec)

/**
 * @brief vector_end - returns an iterator to one past the last element of the vector
 * @param vec - the vector
 * @return a pointer to one past the last element (or NULL)
 */
#define vector_end(vec) \
	((vec) ? &((vec)[vector_size(vec)]) : NULL)


/**
 * @brief vector_push_back - adds an element to the end of the vector
 * @param vec - the vector
 * @param value - the value to add 
 * @return void
 */
#ifdef LINEAR_GROWTH

#define vector_push_back(vec, value) \
do {                                                         			\
	size_t __cap = vector_capacity(vec);                    			\
	if(__cap <= vector_size(vec)) {                       		        \
		vector_grow((vec), !__cap ? __cap + 1 : __cap + GROWTH_FACTOR); \
	}                                                       			\
	vec[vector_size(vec)] = (value);                        			\
	vector_set_size((vec), vector_size(vec) + 1);           			\
} while(0)

#else

#define vector_push_back(vec, value) \
do {                                              		   \
	size_t __cap = vector_capacity(vec);             	   \
	if(__cap <= vector_size(vec)) {        			       \
		vector_grow((vec), __cap ? __cap * GROWTH_FACTOR : \
		 						   MIN_CAPACITY);          \
	}                                            		   \
	vec[vector_size(vec)] = (value);          			   \
	vector_set_size((vec), vector_size(vec) + 1);    	   \
} while(0)

#endif

#endif

