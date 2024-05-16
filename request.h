#ifndef REQUEST_H
#define REQUEST_H
#include "client_handler.h"
#include <stdlib.h>

enum req_type {
    UNDEFINED,
    REQ_STORE,
    REQ_LOAD,
    REQ_ERASE};

struct request
{
    enum req_type req;
    char *key;
    int key_size;
    size_t data_size;
    char *data;
    size_t data_rec;
};

int parse_request(struct request *req, char *start, int size);
int handle_request(struct client *client);
void clear_request(struct request *req);
void free_request(struct request *req);

#endif //REQUEST_H
