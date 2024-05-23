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

    return req->req == UNDEFINED ? -1 : 0;
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
        return -1;

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
    if (whitespace == NULL || parse_request_type(req, start) == -1)
        return -1;

    size -= whitespace - start;
    start = whitespace + 1;
    whitespace = token(start, ' ', size);
    if (whitespace == NULL) {
        whitespace = token(start, '\n', size);
        if (whitespace == NULL)
            return -1;
    }

    if (parse_key(req, start) == -1)
        return -1;

    if (req->req == REQ_STORE) {
        size -= whitespace - start;
        start = whitespace + 1;
        if (token(start, '\n', size) == NULL)
            return -1;

        return parse_data_length(req, start);
    }

    return 0;
}

static int store_request(struct client *client)
{
    struct request *req = client->req;

    if (hash_table_search(&memcached.table, req->key, req->key_size) != NULL)
        return response_to_client(client, 300);

    if (!hash_table_insert(&memcached.table, req->key, req->key_size, req->data, req->data_size))
        return -1;

    req->data = NULL;

    if (response_to_client(client, 100) == -1)
        return -1;

    return 0;
}

static int load_request(struct client *client)
{
    struct request *req = client->req;

    struct node *node = hash_table_search(&memcached.table, req->key, req->key_size);
    if (node == NULL)
        return response_to_client(client, 200);

    if (response_to_client(client, 100) == -1)
        return -1;

    if (response_to_client(client, node->data_size) == -1)
        return -1;

    client->out_node = node;
    client->out_node->ref_count++;
    client->out_data_count = 0;

    return 0;
}

static int erase_request(struct client *client)
{
    struct request *req = client->req;

    if (!hash_table_delete(&memcached.table, req->key, req->key_size))
        return response_to_client(client, 200);

    return response_to_client(client, 100);
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
            break;
        default:
            return -1;
    }

    free_request(client->req);
    client->req = NULL;

    return 0;
}

void free_request(struct request *req)
{
    free(req->key);
    free(req->data);
    free(req);
}

void clear_request(struct request *req)
{
    free(req->key);
    free(req->data);
    memset(req, 0, sizeof(*req));
}