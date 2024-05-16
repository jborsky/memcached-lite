#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <err.h>
#include <poll.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include "server.h"
#include <stdio.h>

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

    return sock_fd;

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

void remove_client(struct server *server, int index)
{
    if (index >= server->size || index < 0)
        return;

    if (server->pfds[index].fd != -1)
        close(server->pfds[index].fd);

    server->pfds[index].fd = -1;

    free_client(&server->clients[index]);
}

void accept_client(struct server *server)
{
    int client_fd = accept(server->fd, NULL, NULL);
    if (client_fd == -1)
        perror("Accept");

    fcntl(client_fd, O_NONBLOCK);

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
            goto error;
    }

    server->pfds[index].fd = client_fd;
    server->clients[index].fd = client_fd;
    server->pfds[index].events = POLLIN | POLLOUT;
    server->pfds[index].revents = 0;

    if (response_to_client(&server->clients[index], "Connected\n") == -1)
        goto error;

    return;

error:
    remove_client(server, index);
}

int memcached_server(int sock_fd)
{
    struct server server;
    if (init_server(&server, sock_fd,256))
        return -1;

    while (1) {
        if (poll(server.pfds, server.size, -1) == -1)
            continue;

        for (int i = 0; i < server.size; ++i) {
            if (server.pfds[i].revents & (POLLIN | POLLHUP)) {
                if (server.pfds[i].fd == server.fd) {
                    accept_client(&server);
                    continue;
                }

                if (handle_client_in(&server.clients[i]) == -1) {
                    remove_client(&server, i);
                    continue;
                }
            }

            if (server.pfds[i].revents & POLLOUT)
                if (handle_client_out(&server.clients[i]) == -1)
                    remove_client(&server, i);
        }
    }
}

void destroy_server(struct server *server)
{
    close(server->fd);
    free(server->clients);
    free(server->pfds);
}




