#include "hashtable.h"
#include "llist.h"
#include "spookyhash.h"
#include <stdlib.h>

bool hash_table_init(struct hash_table *table, uint8_t size, uint32_t seed)
{
    int buckets = 1 << size;
    table->table = calloc(buckets, sizeof(*table->table));
    if (table->table == NULL)
        return false;

    table->mask = (1 << size) - 1;
    table->count = 0;
    table->size = size;
    table->seed = seed;

    return true;
}

static uint32_t hash(struct hash_table *table, const char *key, int key_size)
{
    return spooky_hash32(key, key_size, table->seed);
}

bool hash_table_insert(struct hash_table *table, const char *key, int key_size, void *data)
{
    struct llist *bucket = &table->table[hash(table, key, key_size) & table->mask];

    if (!llist_insert(bucket, key_size, key, data))
        return false;

    if ((double) ++table->count / (1 << table->size) >= 0.75)
        if (!hash_table_resize(table, table->size + 1))
            return false;

    return true;
}

struct node *hash_table_search(struct hash_table *table, const char *key, int key_size)
{
    struct llist *bucket = &table->table[hash(table, key, key_size) & table->mask];

    return llist_search(bucket, key_size, key);
}

bool hash_table_resize(struct hash_table *table, uint8_t new_size)
{
    if (new_size >= 32)
        return false;

    struct hash_table old_table = *table;
    bool rv = false;

    if (!hash_table_init(table, new_size, old_table.seed))
        goto err;

    for (uint32_t i = 0; i <= old_table.mask; ++i) {
        struct node *head = old_table.table[i].head;
        for (struct node *node = head; node != NULL; node = node->next) {
            if (!hash_table_insert(table, node->key, node->nbytes, node->data))
                goto err;
        }
    }

    rv = true;

err:
    hash_table_destroy(&old_table);
    return rv;
}

bool hash_table_delete(struct hash_table *table, const char *key, int key_size)
{
    struct llist *bucket = &table->table[hash(table, key, key_size) & table->mask];

    if (!llist_delete(bucket, key_size, key))
        return false;

    table->count--;

    return true;
}


void hash_table_destroy(struct hash_table *table)
{
    for (uint32_t i = 0; i <= table->mask; ++i) {
        if (table->table[i].head != NULL)
            llist_destroy(&table->table[i]);
    }

    free(table->table);
}