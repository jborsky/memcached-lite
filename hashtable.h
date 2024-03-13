#ifndef HASHTABLE_H
#define HASHTABLE_H
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

typedef uint32_t (*hash_function)(const void *, size_t, uint32_t);

struct hash_table
{
    struct llist *table;
    hash_function hash_func;
    uint32_t mask;
    uint32_t seed;
};

bool hash_table_init(struct hash_table *table, int size, hash_function func, uint32_t seed);

bool hash_table_insert(struct hash_table *table, const char *key, int key_size, void *data);

struct node *hash_table_search(struct hash_table *table, const char *key, int key_size);

bool hash_table_delete(struct hash_table *table, const char *key, int key_size);

void hash_table_destroy(struct hash_table *table);

#endif
