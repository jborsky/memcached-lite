#ifndef LLIST_H
#define LLIST_H
#include "memcached.h"
#include <stdbool.h>

struct node
{
    char *key;
    int key_size;
    void *data;
    size_t data_size;
    struct node *next;
};

struct llist
{
    struct node *head;
};

void llist_init(struct llist *list);
bool llist_insert(struct llist *list, int key_size, const char *key, size_t data_size, void *data);
void llist_move(struct llist *list, struct node *node);
struct node *llist_search(struct llist *list, int key_size, const char *key);
bool llist_delete(struct llist *list, int key_size, const char *key);
void llist_destroy(struct llist *list);


#endif //LLIST_H
