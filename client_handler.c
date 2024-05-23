#include <string.h>
#include <unistd.h>
#include "client_handler.h"
#include "request.h"

static int buffer_realloc(struct buffer *buff, size_t min_space)
{
    while (buff->size - buff->count < min_space) {
        if (buff->buff == NULL)
            buff->size = 256;

        char *tmp = realloc(buff->buff, buff->size * 2 * sizeof(*tmp));
        if (tmp == NULL)
            return -1;

        buff->buff = tmp;
        buff->size *= 2;
    }

    return 0;
}

static int buffer_append(struct buffer *buff, const void *data, size_t data_size)
{
    if (data_size >= buff->size - buff->count) {
        if (buffer_realloc(buff, data_size) == -1)
            return -1;
    }

    memcpy(buff->buff + buff->count, data, data_size);
    buff->count += data_size;

    return 0;
}

static int buffer_read(int fd, struct buffer *buff)
{
    if (512 > buff->size - buff->count)
        if (buffer_realloc(buff, 512) == -1)
            return -1;

    int bytes_read = read(fd, buff->buff + buff->count, 512);
    if (bytes_read <= 0)
        return -1;

    buff->count += bytes_read;
    return 0;
}

static void buffer_clear(struct buffer *buff)
{
    free(buff->buff);
    buff->buff = NULL;
    buff->size = 0;
    buff->count = 0;
}

static int parse_request_line(struct client *client)
{
    struct buffer *buff = &client->in;

    for (int i = 0; i < buff->count; ++i) {
        if (buff->buff[i] == '\n') {
            if (parse_request(client->req, buff->buff, i + 1) == -1)
                return -1;

            buff->count -= i + 1;
            memmove(buff->buff, buff->buff + i + 1, buff->count);
        }
    }

    return 0;
}

static int init_request_data(struct client *client)
{
    struct request *req = client->req;
    struct buffer *buff = &client->in;

    if (req->data_size > 0) {
        req->data = malloc(req->data_size);
        if (req->data == NULL)
            return -1;

        int nbytes = buff->count > req->data_size ? req->data_size : buff->count;
        memcpy(req->data, buff->buff, nbytes);

        req->data_rec += nbytes;

        buff->count -= nbytes;
        memmove(buff->buff, buff->buff + nbytes, buff->count);
    }

    return 0;
}

static int read_request(struct client *client)
{
    struct buffer *buff = &client->in;

    if (buffer_read(client->fd, buff) == -1)
        return -1;

    if (parse_request_line(client) == -1) {
        buff->count = 0;
        if (response_to_client(client, 400) == -1)
            return -1;

        clear_request(client->req);

        return 0;
    }

    if (client->req != UNDEFINED && init_request_data(client) == -1)
        return -1;

    return 0;
}

static int read_request_data(struct client *client)
{
    struct request *req = client->req;

    ssize_t bytes_read = read(client->fd, req->data + req->data_rec, req->data_size - req->data_rec);
    if (bytes_read <= 0)
        return -1;

    req->data_rec += bytes_read;

    return 0;
}

static int send_data(struct client *client)
{
    if (client->out_node == NULL)
        return 0;

    struct node *node = client->out_node;

    ssize_t bytes_written = write(client->fd,  node->data + client->out_data_count,
                                  node->data_size - client->out_data_count);
    if (bytes_written == -1)
        return -1;

    client->out_data_count += bytes_written;

    if (client->out_data_count == node->data_size) {
        client->out_node->ref_count--;
        client->out_node = NULL;

    }

    return 0;
}

int response_to_client(struct client *client, int code)
{
    int response = htonl(code);
    return buffer_append(&client->out, &response, 4);
}

int handle_client_in(struct client *client)
{
    if (client->req == NULL) {
        client->req = calloc(1, sizeof(*client->req));
        if (client->req == NULL)
            return -1;
    }

    if (client->req->req == UNDEFINED) {
        if (read_request(client) == -1)
            return -1;
    }

    if (client->req->req != UNDEFINED) {
        if (client->req->data_rec != client->req->data_size && read_request_data(client) == -1)
            return -1;

        if (handle_request(client) == -1)
            return -1;
    }

    return 0;
}

int handle_client_out(struct client *client)
{
    if (client->out.count == 0)
        return send_data(client);

    ssize_t bytes_written = write(client->fd, client->out.buff, client->out.count);
    if (bytes_written == -1)
        return -1;

    client->out.count -= bytes_written;
    memmove(client->out.buff, client->out.buff + bytes_written, client->out.count);

    return 0;
}

void free_client(struct client *client)
{
    client->fd = -1;
    buffer_clear(&client->in);
    buffer_clear(&client->out);
    free_request(client->req);
    client->req = NULL;
    if (client->out_node) {
        client->out_node->ref_count--;
        client->out_node = NULL;
    }
}




