/**
* auther: jamal-jiang
* date: 17/5/1
*/

#ifndef TINYHTTPD_REQUEST_H
#define TINYHTTPD_REQUEST_H

/*************************************************
Function: handle_request
Description: 处理连接
Calls:
Called By: main
Input: fd
Output: void
Return: void
*************************************************/
void handle_request(int listen_sock, int fd);

#endif //TINYHTTPD_REQUEST_H
