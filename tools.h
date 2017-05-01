/**
* auther: jamal-jiang
* date: 17/5/1
*/

#ifndef TINYHTTPD_TOOLS_H
#define TINYHTTPD_TOOLS_H

/*************************************************
Function: create_and_bind
Description: 创建并监听套接字
Calls: fcntl
Called By: main
Input: port
Output: listen_sock
Return: 设置结果(>0或-1)
*************************************************/
static int create_and_bind(int port);

/*************************************************
Function: make_socket_non_blocking
Description: 设置文件描述符为非阻塞模式
Calls: fcntl
Called By: main
Input: fd(文件描述符)
Output: 无
Return: 设置结果(0或-1)
*************************************************/
static int make_socket_non_blocking(int fd);

#endif //TINYHTTPD_TOOLS_H
