#ifndef CLIENT_HANDLER_H
#define CLIENT_HANDLER_H
#include <stdlib.h>

struct buffer
{
    char *buff;
    size_t size;
    size_t count;
};

struct client
{
    int fd;
    struct buffer in;
    struct buffer out;
    struct buffer out_data;
    struct request *req;
};

int handle_client_in(struct client *client);
int handle_client_out(struct client *client);
int response_to_client(struct client *client, const char *response);
void free_client(struct client *client);

#endif //CLIENT_HANDLER_H
