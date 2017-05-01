/*****************************************************************************
File name: main
Description: 程序主入口
Author: jamal-jiang
Version: 0.1
Date:
History:
*****************************************************************************/

#include "common.h"
#include "tools.h"
#include "request.h"

/* 共用listen_sock */
static int listen_sock;
/* epoll fd */
static int epoll_fd;
/* epoll ev */
static struct epoll_event event;

/*************************************************
Function: accept_sock
Description: accpet
Calls:
Called By: main
Input: fd
Output:
Return: none
*************************************************/
static void accept_sock(int fd);

int main(int argc, char **argv) {

    struct epoll_event *events;

    /* 创建并bind */
    listen_sock = create_and_bind(5188);
    if (-1 == listen_sock) {
        printf("create_and_bind error\n");
        return -1;
    }

    /* 设置listen_sock非阻塞 */
    if (make_socket_non_blocking(listen_sock) < 0) {
        printf("make_socket_non_blocking error\n");
        return -1;
    }
git
    /* listen */
    if (listen(listen_sock, SOMAXCONN) < 0) {
        printf("listen error\n");
        return -1;
    }

    /* 创建epoll实例,设置为边缘触发模式 */
    epoll_fd = epoll1(0);
    if (-1 == epoll_fd) {
        printf("epoll1 error\n");
        return -1;
    }
    event.data.fd = listen_sock;
    event.events = EPOLLIN | EPOLLET;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listen_sock, &event) < 0) {
        printf("epoll_ctl error\n");
        return -1;
    }

    /* 分配内存 */
    events = calloc(MAX_EVENTS, sizeof(event));

    /* 开始循环 */
    while (1) {
        int i, n;
        n = epoll_wait(epoll_fd, event, MAX_EVENTS, -1);
        if (n == -1) {
            printf("epoll_wait error\n");
        }

        for (i = 0; i < n; i++) {
            /* epoll 错误处理 */
            if ((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP)) {
                printf("epoll error, close fd\n");
                close(events[i].data.fd);
                continue;
            } else if (events[i].data.fd == listen_sock) {
                /* 请求连接的fd,直接加入这个连接 */
            } else {
                /* 处理连接 */
                handle_request(listen_sock, events[i].data.fd);
            }
        }
    }
}

static void accept_sock(int fd) {

}