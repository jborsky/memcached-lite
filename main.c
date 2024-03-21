#include "hashtable.h"
#include <assert.h>

int main(void)
{
    struct hash_table table;
    if (!hash_table_init(&table, 1, 0))
        return -1;

    uint16_t int_key1 = 1;
    uint16_t int_key2 = 2;
    const char byte_key[] = {0x00, 0x12, 0x16};
    const char *keys[] = {"1", "2", "3", "4", "5",
            (const char *) &int_key1, byte_key, (const char *) &int_key2};
    int keys_length[] = {1, 1, 1, 1, 1, 2, 3, 2};

    for (int i = 0; i < 8; ++i)
        assert(hash_table_insert(&table, keys[i], keys_length[i], NULL));

    for (int i = 0; i < 8; ++i)
        assert(hash_table_search(&table, keys[i], keys_length[i]) != NULL);

    for (int i = 0; i < 8; ++i)
        assert(hash_table_delete(&table, keys[i], keys_length[i]));

    assert(!hash_table_search(&table, "1", 1));
    assert(!hash_table_delete(&table, "1", 1));

    hash_table_destroy(&table);

    return 0;

}