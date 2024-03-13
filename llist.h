#ifndef LLIST_H
#define LLIST_H

#include <stdbool.h>

struct node
{
    char *key;
    int nbytes;
    void *data;
    struct node *next;
};

struct llist
{
    struct node *head;
};

struct llist *llist_init(void);
bool llist_insert(struct llist *list, const char *key, int nbytes, void *data);
struct node *llist_search(struct llist *list, const char *key, int nbytes);
bool llist_delete(struct llist *list, const char *key, int nbytes);
void llist_destroy(struct llist *list);


#endif //LLIST_H
