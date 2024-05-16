#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include "request.h"
#include "hashtable.h"
#include "memcached.h"
#include "client_handler.h"

static int parse_request_type(struct request *req, char *request)
{
    if (strcmp(request, "store") == 0)
        req->req = REQ_STORE;

    if (strcmp(request, "load") == 0)
        req->req = REQ_LOAD;

    if (strcmp(request, "erase") == 0)
        req->req = REQ_ERASE;

    return req->req == UNDEFINED ? -2 : 0;
}

static int parse_key(struct request *req, char *key)
{
    req->key = malloc(strlen(key));
    if (key == NULL)
        return -1;

    memcpy(req->key, key, strlen(key));
    req->key_size = strlen(key);

    return 0;
}

static int parse_data_length(struct request *req, char *length)
{
    char *end = NULL;
    errno = 0;
    req->data_size = strtoul(length, &end, 10);
    if (*end != '\0' || errno == ERANGE)
        return -2;

    return 0;
}

static char *token(char *line, int ch, int size)
{
    char *found = memchr(line, ch, size);
    if (found == NULL)
        return NULL;

    *found = '\0';

    return found;
}

int parse_request(struct request *req, char *start, int size)
{
    char *whitespace = token(start, ' ', size);
    if (whitespace == NULL || parse_request_type(req, start) == -2)
        return -2;

    size -= whitespace - start;
    start = whitespace + 1;
    whitespace = token(start, ' ', size);

    if (whitespace == NULL)
        return -2;

    if (parse_key(req, start) == -1)
        return -1;

    size -= whitespace - start;
    start = whitespace + 1;
    token(start, '\r', size);

    if (req->req == REQ_STORE && parse_data_length(req, start) == -2)
        return -2;

    return 0;
}

static int store_request(struct client *client)
{
    struct request *req = client->req;

    if (hash_table_search(&memcached.table, req->key, req->key_size) != NULL)
        return response_to_client(client, "Duplicate key\r\n");

    if (!hash_table_insert(&memcached.table, req->key, req->key_size, req->data, req->data_size))
        return -1;

    req->data = NULL;

    if (response_to_client(client, "Stored\r\n") == -1)
        return -1;

    return 0;
}

static int load_request(struct client *client)
{
    struct request *req = client->req;

    struct node *node = hash_table_search(&memcached.table, req->key, req->key_size);
    if (node == NULL)
        return response_to_client(client, "Key not found\r\n");

    if (response_to_client(client, "Found ") == -1)
        return -1;

    char size[32];
    sprintf(size, "%zu\r\n", node->data_size);
    if (response_to_client(client, size) == -1)
        return -1;

    client->out_node = node;
    client->out_node->ref_count++;
    client->out_data_count = 0;

    return 0;
}

static int erase_request(struct client *client)
{
    struct request *req = client->req;

    struct node *node = hash_table_pop(&memcached.table, req->key, req->key_size);
    if (node == NULL)
        return response_to_client(client, "Key not found\r\n");

    if (node->ref_count == 0)
        node_destroy(node);
    else
        llist_move(memcached.node_garbage, node);

    return 0;
}

int handle_request(struct client *client)
{
    switch (client->req->req) {
        case REQ_STORE:
            if (store_request(client) == -1)
                return -1;
            break;
        case REQ_LOAD:
            if (load_request(client) == -1)
                return -1;
            break;
        case REQ_ERASE:
            if (erase_request(client) == -1)
                return -1;
        default:
            return -1;
    }

    free_request(client->req);
    client->req = NULL;

    return 0;
}