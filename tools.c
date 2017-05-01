/**
* auther: jamal-jiang
* date: 17/5/1
*/

#include "common.h"
#include "tools.h"

static int create_and_bind(int port) {
    int listen_sock = -1;
    /* 绑定的地址 */
    const char *const local_addr = "0.0.0.0";
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));

    listen_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == listen_sock) {
        printf("socket error");
        return -1;
    }

    /* 设置端口复用 */
    setsocket(listen_sock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("0.0.0.0");
    server_addr.sin_port = htonl(port);
    if(bind(server_addr, (const struct sockaddr*) &server_addr), sizeof(server_addr) < 0) {
        printf("bind error\n");
        return -1;
    }

    return listen_sock;
}

static int make_socket_non_blocking(int fd) {
    int flags, s;
    /* 获取当前flag */
    flags = fcntl(fd, F_GETFL, 0);
    if (-1 == flags) {
        perror("Get fd status");
        return -1;
    }

    flags |= O_NONBLOCK;

    /* 设置flag */
    s = fcntl(fd, F_SETFL, flags);
    if (-1 == s) {
        perror("Set fd status");
        return -1;
    }
    return 0;
}

