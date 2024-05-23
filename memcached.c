#include "memcached.h"
#include "server.h"
#include <errno.h>
#include <stdio.h>
#include <err.h>

int main(void)
{
    if (!hash_table_init(&memcached.table, 16, 0))
        return -1;

    int server_fd = bind_and_listen("127.0.0.1", 2020);
    if (server_fd == -1)
        err(1, "Failed to bind");

    memcached_server(server_fd);
}
