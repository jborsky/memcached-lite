#ifndef CLIENT_HANDLER_H
#define CLIENT_HANDLER_H
#include <stdlib.h>
#include "llist.h"

struct buffer
{
    char *buff;
    size_t size;
    size_t count;
};

struct client
{
    int fd;
    struct request *req;
    struct buffer in;
    struct buffer out;

    struct node *out_node;
    size_t out_data_count;
};

int handle_client_in(struct client *client);
int handle_client_out(struct client *client);
int response_to_client(struct client *client, int code);
void free_client(struct client *client);

#endif //CLIENT_HANDLER_H
