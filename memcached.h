#ifndef MEMCACHED_H
#define MEMCACHED_H
#include"hashtable.h"

struct memcached
{
    struct hash_table table;
};

struct memcached memcached;

#endif //MEMCACHED_H
