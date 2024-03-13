#ifndef HASHTABLE_H
#define HASHTABLE_H
#include <stdint.h>

typedef uint64_t (*hash_function)(const void *, int, uint32_t);

#endif
