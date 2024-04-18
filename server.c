#include <stdbool.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <err.h>
#include <poll.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

struct buffer
{
    char *buff;
    int size;
    int count;
};

struct client
{
    int fd;
    struct buffer in;
    struct buffer out;
};

struct server
{
    int fd;
    struct pollfd *pfds;
    struct client *clients;
    int size;
};

int bind_and_listen(const char *ip, int port)
{
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd == -1)
        return -1;

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if (inet_aton(ip, &addr.sin_addr) == 0)
        goto err;

    if (bind(sock_fd, (const struct sockaddr *)&addr, sizeof(addr)) == -1)
        goto err;

    if (listen(sock_fd, 10) == -1)
        goto err;

    return 0;

err:
    close(sock_fd);
    return -1;
}

int init_server(struct server *server, int fd, int count)
{
    server->clients = calloc(count, sizeof(*server->clients));
    if (server->clients == NULL)
        return -1;


    server->pfds = malloc(count * sizeof(*server->pfds));
    if (server->pfds == NULL) {
        free(server->clients);
        return -1;
    }
    memset(server->pfds, -1, count * sizeof(*server->pfds));

    server->size = count;
    server->fd = fd;
    server->pfds[0].fd = fd;
    server->pfds[0].events = POLLIN;

    return 0;
}

int increase_capacity(struct server *server)
{
    struct client *tmp_c = realloc(server->clients, server->size * 2 * sizeof(*server->clients));
    if (tmp_c == NULL)
        return -1;

    server->clients = tmp_c;
    memset(server->clients + server->size, 0, server->size * sizeof(*server->clients));

    struct pollfd *tmp_p = realloc(server->pfds ,server->size * 2 * sizeof(*server->pfds));
    if (tmp_p == NULL)
        return -1;

    server->pfds = tmp_p;
    memset(server->pfds + server->size, -1, server->size * sizeof(*server->pfds));

    server->size *= 2;

    return 0;
}

void destroy_server(struct server *server)
{
    close(server->fd);
    free(server->clients);
    free(server->pfds);
}

int buffer_append(struct buffer *buff, const char *data, int nbytes)
{
    if (nbytes >= buff->size - buff->count) {
        if (buff->buff == NULL)
            buff->size = 256;

        char *tmp = realloc(buff->buff, buff->size * 2 * sizeof(*tmp));
        if (tmp == NULL)
            return -1;

        buff->buff = tmp;
        buff->size *= 2;
    }

    memcpy(buff->buff + buff->count, data, nbytes);
    buff->count += nbytes;

    return 0;
}

void clear_buffer(struct buffer *buff)
{
    free(buff->buff);
    buff->buff = NULL;
    buff->size = 0;
    buff->count = 0;
}

int accept_client(struct server *server)
{
    int client_fd = accept(server->fd, NULL, NULL);
    if (client_fd == -1)
        return -1;

    int index = -1;
    for (int i = 0; i < server->size; ++i) {
        if (server->pfds[i].fd == -1) {
            index = i;
            break;
        }
    }

    if (index == -1) {
        index = server->size;
        if (increase_capacity(server) == -1)
            return -1;
    }

    server->pfds[index].fd = client_fd;
    server->clients[index].fd = client_fd;
    server->pfds[index].events = POLLIN | POLLOUT;

    const char *welcome = "Connected\n\r";
    buffer_append(&server->clients[index].out, welcome, strlen(welcome));

    return 0;
}

void remove_client(struct server *server, int index)
{
    close(server->pfds[index].fd);

    server->pfds[index].fd = -1;

    struct client *client = &server->clients[index];
    client->fd = -1;
    clear_buffer(&client->in);
    clear_buffer(&client->out);
}

int read_message(struct client *client) {
    char buffer[512];

    int bytes_read = read(client->fd, buffer, 512);
    if (bytes_read == -1)
        return -1;
    if (bytes_read == 0)
        return -2;

    if (buffer_append(&client->in, &buffer, bytes_read) == -1)
        return -1;

    // parse_message

    return 0;
}

int send_message(struct client *client)
{
    if (client->out.count == 0)
        return 0;

    int bytes_written = write(client->fd, client->out.buff, client->out.count);
    if (bytes_written == -1) {
        if (errno == EPIPE)
            return -2;
        return -1;
    }

    client->out.count -= bytes_written;
    memmove(client->out.buff, client->out.buff + bytes_written, client->out.count);

    return 0;
}

int memcached_server(int sock_fd)
{
    struct server server;
    if (init_server(&server, sock_fd,256))
        return -1;

    while (1) {
        if (poll(server.pfds, server.size, 0) == -1)
            goto error;

        for (int i = 0; i < server.size; ++i) {
            if (server.pfds[i].revents & (POLLIN | POLLHUP)) {
                if (server.pfds[i].fd != server.fd) {
                    int ret = read_message(&server.clients[i]);
                    if (ret == -1)
                        goto error;
                    if (ret == -2)
                        remove_client(&server, i);
                }

                else {
                    if (accept_client(&server) == -1)
                        goto error;
                }
            }

            if (server.pfds[i].revents & POLLOUT) {
                int ret = send_message(&server.clients[i]);
                if (ret == -1)
                    goto error;
                if (ret == -2) {
                    remove_client(&server, i);
                }
            }
        }
    }

error:
    destroy_server(&server);
    return -1;
}




