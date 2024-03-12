#include "llist.h"

#include <stdlib.h>
#include <string.h>


struct llist *llist_init(void)
{
    struct llist *new_list = malloc(sizeof(*new_list));
    if (new_list == NULL)
        return NULL;

    new_list->head = NULL;

    return new_list;
}

bool llist_insert(struct llist *list, const char *key, int nbytes, void *data)
{
    struct node *new_node = malloc(sizeof(*new_node));
    if (new_node == NULL)
        return false;

    new_node->key = malloc(nbytes);
    if (new_node->key == NULL) {
        free(new_node);
        return false;
    }
    memcpy(new_node->key, key, nbytes);

    new_node->nbytes = nbytes;
    new_node->data = data;
    new_node->next = list->head;
    list->head = new_node;

    return true;
}

struct node *llist_search(struct llist *list, const char *key, int nbytes)
{
    for (struct node *node = list->head; node != NULL; node = node->next) {
        if ((nbytes == node->nbytes) && memcmp(node->key, key, nbytes) == 0)
            return node;
    }

    return NULL;
}

struct node *llist_delete(struct llist *list, const char *key, int nbytes)
{
    struct node **node;
    for (node = &list->head; *node != NULL; node = &(*node)->next) {
        if ((nbytes == (*node)->nbytes) && memcmp((*node)->key, key, nbytes) == 0)
            break;
    }

    if (*node == NULL)
        return NULL;

    struct node *deleted_node = *node;

    *node = (*node)->next;

    return deleted_node;
}

void llist_destroy(struct llist *list)
{
    for (struct node *node = list->head; node != NULL;) {
        struct node *next_node = node->next;
        free(node->key);
        free(node);
        node = next_node;
    }

    free(list);
}