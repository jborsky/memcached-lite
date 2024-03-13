#include "hashtable.h"
#include "llist.h"
#include <stdlib.h>

bool hash_table_init(struct hash_table *table, int size, hash_function func, uint32_t seed)
{
    int buckets = 1 << size;
    table->table = calloc(buckets, sizeof(*table->table));
    if (table->table == NULL)
        return false;

    table->hash_func = func;
    table->mask = (1 << size) - 1;
    table->seed = seed;

    return true;
}

static uint32_t hash(struct hash_table *table, const char *key, int key_size)
{
    return table->hash_func(key, key_size, table->seed);
}

bool hash_table_insert(struct hash_table *table, const char *key, int key_size, void *data)
{
    struct llist *bucket = table->table[hash(table, key, key_size) & table->mask];

    return llist_insert(bucket, key, key_size, data);
}

struct node *hash_table_search(struct hash_table *table, const char *key, int key_size)
{
    struct llist *bucket = table->table[hash(table, key, key_size) & table->mask];

    return llist_search(bucket, key, key_size);
}

bool hash_table_delete(struct hash_table *table, const char *key, int key_size, void *data)
{
    struct llist *bucket = table->table[hash(table, key, key_size) & table->mask];

    return llist_delete(bucket, key, key_size);
}

void hash_table_destroy(struct hash_table *table)
{
    for (uint32_t i = 0; i < table->mask; ++i) {
        if (table->table[i] != NULL)
            llist_destroy(table->table[i]);
    }

    free(table);
}