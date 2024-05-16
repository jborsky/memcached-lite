#include "llist.h"

#include <stdlib.h>
#include <string.h>


void llist_init(struct llist *list)
{
    list->head = NULL;
}

bool llist_insert(struct llist *list, int key_size, const char *key, size_t data_size, void *data)
{
    struct node *new_node = malloc(sizeof(*new_node));
    if (new_node == NULL)
        return false;

    new_node->key = malloc(key_size);
    if (new_node->key == NULL) {
        free(new_node);
        return false;
    }
    memcpy(new_node->key, key, key_size);

    new_node->key_size = key_size;
    new_node->data_size = data_size;
    new_node->data = data;
    new_node->next = list->head;
    new_node->ref_count = 0;
    list->head = new_node;

    return true;
}

void llist_move(struct llist *list, struct node *node)
{
    node->next = list->head;
    list->head = node;
}

struct node *llist_search(struct llist *list, int key_size, const char *key)
{
    for (struct node *node = list->head; node != NULL; node = node->next) {
        if ((key_size == node->key_size) && memcmp(node->key, key, key_size) == 0)
            return node;
    }

    return NULL;
}

void node_destroy(struct node *node)
{
    free(node->key);
    free(node->data);
    free(node);
}

bool llist_delete(struct llist *list, int nbytes, const char *key)
{
    struct node **node;
    for (node = &list->head; *node != NULL; node = &(*node)->next) {
        if ((nbytes == (*node)->key_size) && memcmp((*node)->key, key, nbytes) == 0)
            break;
    }

    if (*node == NULL)
        return false;

    struct node *tmp = *node;
    *node = (*node)->next;

    node_destroy(tmp);

    return true;
}

struct node *llist_pop(struct llist *list, int nbytes, const char *key)
{
    struct node **node;
    for (node = &list->head; *node != NULL; node = &(*node)->next) {
        if ((nbytes == (*node)->key_size) && memcmp((*node)->key, key, nbytes) == 0)
            break;
    }

    if (*node == NULL)
        return NULL;

    struct node *tmp = *node;
    *node = (*node)->next;

    return tmp;
}

void llist_cleanup(struct llist *list)
{
    struct node **node;
    for (node = &list->head; *node != NULL; node = &(*node)->next) {
        if ((*node)->ref_count != 0)
            continue;

        struct node *tmp = *node;
        *node = (*node)->next;

        node_destroy(tmp);

    }
}

void llist_destroy(struct llist *list)
{
    for (struct node *node = list->head; node != NULL;) {
        struct node *next_node = node->next;
        node_destroy(node);
        node = next_node;
    }
}
