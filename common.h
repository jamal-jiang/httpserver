/**
* auther: jamal-jiang
* date: 17/4/26
*/

#ifndef TINYHTTPD_COMMON_H
#define TINYHTTPD_COMMON_H

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
#include<pthread.h>
#include <sys/sendfile.h>
#include <sys/types.h>
#include <sys/stat.h>

#define READ_BUF_LEN 20
/* 最大读取大小 */
#define BUF_SIZE 2048
/* 最大header大小 */
#define MAX_HEADER_SIZE 2048
/* conn超时时间 */
#define CONNECT_TIMEOUT 60
/* read超时时间 */
#define READ_TIMEOUT 60
/* 最大epoll事件 */
#define MAX_EVENTS 4096

/* 定义各个事件 */


/* connection结构体 */
struct connection {
    /* socket */
    int sock;
    /* 读取的文件fd */
    int fd;
    /* 当前的状态 */
    int status;
    /* http响应码 */
    int response_code;
    /* 读取的文件位置 */
    int read_pos;
    /* 写入的文件位置 */
    int write_pos;
    /* buf */
    char buf[BUF_SIZE];
};


#endif //TINYHTTPD_COMMON_H
