#ifndef SERVER_H
#define SERVER_H

struct server
{
    int fd;
    struct pollfd *pfds;
    struct client *clients;
    int size;
};

int bind_and_listen(const char *ip, int port);
int memcached_server(int sock_fd);
void destroy_server(struct server *server);

#endif //SERVER_H
