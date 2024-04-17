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

void llist_init(struct llist *list);
bool llist_insert(struct llist *list, int nbytes, const char key[nbytes],  void *data);
void llist_move(struct llist *list, struct node *node);
struct node *llist_search(struct llist *list, int nbytes, const char key[nbytes]);
bool llist_delete(struct llist *list, int nbytes, const char key[nbytes]);
void llist_destroy(struct llist *list);


#endif //LLIST_H
