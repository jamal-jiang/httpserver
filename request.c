/*****************************************************************************
File name: request
Description: 
Author: jamal-jiang
Version: 0.1
Date: 17/5/1
History: 
*****************************************************************************/

#include "common.h"
#include "request.h"

void handle_request(int listen_sock, int fd) {
    if (listen_sock == fd) {
        accept_sock();
    }
}