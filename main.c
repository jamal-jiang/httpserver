//#include "common.h"

#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <netdb.h>
#include <errno.h>
#include <pthread.h>
#include <sys/sendfile.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>

#define BUF_SIZE 2048
#define MAX_PROCESS 10240
#define MAX_EVENTS 10240
#define NO_SOCK -1
#define NO_FILE -1
#define STATUS_READ_REQUEST 0
#define STATUS_SEND_RESPONSE_HEADER 1
#define STATUS_SEND_RESPONSE 2

struct process {
    int sock;
    int fd;
    int read_pos;
    int write_pos;
    int total_length;
    int status;
    int response_code;
    char buf[BUF_SIZE];
};


static int listenfd;
static int epfd;
static struct epoll_event ev;
static struct process processes[MAX_PROCESS];
static int current_processes = 0;

static int make_socket_non_blocking(int fd) {
    int flags, s;
    // 获取当前flag
    flags = fcntl(fd, F_GETFL, 0);
    if (-1 == flags) {
        perror("Get fd status");
        return -1;
    }

    flags |= O_NONBLOCK;

    // 设置flag
    s = fcntl(fd, F_SETFL, flags);
    if (-1 == s) {
        perror("Set fd status");
        return -1;
    }
    return 0;
}

void reset_process(struct process *process) {
    process->read_pos = 0;
    process->write_pos = 0;
}

struct process *find_free_fd_slow() {
    int i = 0;
    for (i = 0; i < MAX_PROCESS; i++) {
        if (processes[i].sock == NO_SOCK) {
            return &processes[i];
        }
    }
    return NULL;
}

struct process *find_free_fd(int fd) {
    if (fd >= 0 && fd < MAX_PROCESS && processes[fd].sock == NO_SOCK) {
        printf("quick find_free_fd\n");
        return &processes[fd];
    } else {
        return find_free_fd_slow();
    }
}

struct process *find_process_by_fd(int fd) {
    int i = 0;
    printf("fd: %d\n", fd);
    printf("fd1: %d\n", processes[fd].sock);
    if (fd >= 0 && fd < MAX_PROCESS && processes[fd].sock == fd) {
        printf("quick return\n");
        return &processes[fd];
    } else {
        for (i = 0; i < MAX_PROCESS; i++) {
            if (fd == processes[i].sock) {
                return &processes[i];
            }
        }
    }
}

void cleanup(struct process *process) {
    int s;
    if (process->sock != NO_SOCK) {
        s = close(process->sock);
        current_processes--;
        if (s == NO_SOCK) {
            printf("close sock\n");
        }
    }
    if (process->fd != -1) {
        s = close(process->fd);
        if (s == NO_FILE) {
            printf("fd: %d\n", process->fd);
            printf("\n");
            printf("close file\n");
        }
    }
    process->sock = NO_SOCK;
    reset_process(process);
}

void init_process() {
    int i = 0;
    for (i = 0; i < MAX_PROCESS; i++) {
        processes[i].sock = NO_SOCK;
    }
}

void accept_fd(int fd) {
    while (1) { // 由于采用了边缘触发模式，这里需要使用循环
        struct sockaddr in_addr = {0};
        socklen_t in_addr_len = sizeof(in_addr);
        int __result;
        int accp_fd = accept(listenfd, &in_addr, &in_addr_len);
        if (-1 == accp_fd) {
            perror("Accept");
            break;
        }

        __result = make_socket_non_blocking(accp_fd);
        if (-1 == __result) {
            abort();
        }

        ev.data.fd = accp_fd;
        ev.events = EPOLLIN | EPOLLET;
        // 为新accept的 file describe 设置epoll事件
        __result = epoll_ctl(epfd, EPOLL_CTL_ADD, accp_fd, &ev);
        if (-1 == __result) {
            perror("epoll_ctl");
            abort();
        }

        printf("accept process\n");
        struct process *process = find_free_fd(accp_fd);
        current_processes++;
        reset_process(process);
        process->sock = accp_fd;
        process->fd = NO_FILE;
        process->status = STATUS_READ_REQUEST;
    }
}

void read_request(struct process *process) {
    int sock = process->sock;
    char *buf = process->buf;
    ssize_t count;

    while (1) {
        count = read(sock, buf + process->read_pos, BUF_SIZE - process->read_pos);
        printf("count: %d\n", count);
        if (count == -1) {
            if (errno != EAGAIN) {
                printf("read request error\n");
                return;
            }
            else {
                /* errno == EAGAIN表示读取完毕 */
                printf("read finish\n");
                break;
            }
        } else if (count == 0) {
            // 被客户端关闭连接
            cleanup(process);
            return;
        } else if (count > 0) {
            process->read_pos += count;
        }
    }

    int length = process->read_pos;
    buf[length] = '\0';
    printf("buf: %d %d %s\n", process->read_pos, sizeof(buf) * sizeof(char), buf);
    write(process->sock,
          "HTTP/1.1 200 OK\r\nDate: Mon, 1 Apr 2013 01:01:01 GMT\r\nContent-Type: text/plain\r\nContent-Length: 25\r\n\r\n Hello from Epoll Server",
          strlen("HTTP/1.1 200 OK\r\nDate: Mon, 1 Apr 2013 01:01:01 GMT\r\nContent-Type: text/plain\r\nContent-Length: 25\r\n\r\n Hello from Epoll Server!"));
    close(process->sock);
}

void handle_request(int fd) {
    // printf("get fd start: %d\n", fd);
    struct process *process;
    process = &processes[fd];
    // printf("get fd: %d %d\n", process->sock, process->status);
    switch (process->status) {
        case STATUS_READ_REQUEST:
            read_request(process);
            break;
        case STATUS_SEND_RESPONSE_HEADER:
            break;
        case STATUS_SEND_RESPONSE:
            break;
        default:
            break;
    }
}

int main(int argc, char **argv) {

    struct epoll_event event[MAX_EVENTS];
    const char *const local_addr = "0.0.0.0";
    struct sockaddr_in server_addr = {0};
    int result;

    //memset(server_addr, 0, sizeof(server_addr));

    /* 初始化process */
    init_process();

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == listenfd) {
        perror("socket error\n");
        return -1;
    }

    int on = 1;
    // 打开 socket 端口复用, 防止测试的时候出现 Address already in use
    result = setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    if (-1 == result) {
        perror("Set socket");
        return 0;
    }

    server_addr.sin_family = AF_INET;
    inet_aton(local_addr, &(server_addr.sin_addr));
    server_addr.sin_port = htons(5188);
    result = bind(listenfd, (const struct sockaddr *) &server_addr, sizeof(server_addr));
    if (-1 == result) {
        perror("Bind port");
        return 0;
    }
    result = make_socket_non_blocking(listenfd);
    if (-1 == result) {
        return 0;
    }
    result = listen(listenfd, 200);
    if (-1 == result) {
        perror("Start listen");
        return 0;
    }

    // 创建epoll实例
    epfd = epoll_create1(10);
    if (1 == epfd) {
        perror("Create epoll instance");
        return 0;
    }

    ev.data.fd = listenfd;
    ev.events = EPOLLIN | EPOLLET /* 边缘触发选项。 */;
    // 设置epoll的事件
    result = epoll_ctl(epfd, EPOLL_CTL_ADD, listenfd, &ev);
    if (-1 == result) {
        perror("Set epoll_ctl");
        return 0;
    }

    while (1) {
        int wait_count, i = 0;
        // 等待事件
        wait_count = epoll_wait(epfd, event, MAX_EVENTS, -1);
        printf("wait_count: %d\n", wait_count);

        for (i = 0; i < wait_count; i++) {
            uint32_t events = event[i].events;
            // IP地址缓存
            char host_buf[NI_MAXHOST];
            // PORT缓存
            char port_buf[NI_MAXSERV];

            int __result;
            // 判断epoll是否发生错误
            if (events & EPOLLERR || events & EPOLLHUP || (!events & EPOLLIN)) {
                printf("Epoll has error\n");
                close(event[i].data.fd);
                continue;
            } else if (listenfd == event[i].data.fd) {
                accept_fd(listenfd);
            } else {
                handle_request(event[i].data.fd);
            }
        }
    }
    close(epfd);
    return 0;
}