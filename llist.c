#include "llist.h"

#include <stdlib.h>
#include <string.h>


void llist_init(struct llist *list)
{
    list->head = NULL;
}

bool llist_insert(struct llist *list, int nbytes, const char key[nbytes], void *data)
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

void llist_move(struct llist *list, struct node *node)
{
    node->next = list->head;
    list->head = node;
}

struct node *llist_search(struct llist *list, int nbytes, const char key[nbytes])
{
    for (struct node *node = list->head; node != NULL; node = node->next) {
        if ((nbytes == node->nbytes) && memcmp(node->key, key, nbytes) == 0)
            return node;
    }

    return NULL;
}

static void node_destroy(struct node *node)
{
    free(node->key);
    free(node->data);
    free(node);
}


bool llist_delete(struct llist *list, int nbytes, const char *key)
{
    struct node **node;
    for (node = &list->head; *node != NULL; node = &(*node)->next) {
        if ((nbytes == (*node)->nbytes) && memcmp((*node)->key, key, nbytes) == 0)
            break;
    }

    if (*node == NULL)
        return false;

    struct node *tmp = *node;
    *node = (*node)->next;

    node_destroy(tmp);

    return true;
}


void llist_destroy(struct llist *list)
{
    for (struct node *node = list->head; node != NULL;) {
        struct node *next_node = node->next;
        node_destroy(node);
        node = next_node;
    }
}
