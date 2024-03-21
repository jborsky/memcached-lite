#ifndef HASHTABLE_H
#define HASHTABLE_H
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

struct hash_table
{
    struct llist *table;
    uint32_t mask;
    uint32_t seed;
    uint32_t count;
    uint8_t size;
};

bool hash_table_init(struct hash_table *table, uint8_t size, uint32_t seed);

bool hash_table_insert(struct hash_table *table, const char *key, int key_size, void *data);

struct node *hash_table_search(struct hash_table *table, const char *key, int key_size);

bool hash_table_resize(struct hash_table *table, uint8_t new_size);

bool hash_table_delete(struct hash_table *table, const char *key, int key_size);

void hash_table_destroy(struct hash_table *table);

#endif
