//
// Created by robert on 13.01.2020.
//

#ifndef TINY_PACKAGELOSINGAPI_H
#define TINY_PACKAGELOSINGAPI_H

#include <stdio.h>
#include <sys/socket.h>
#include <random>

ssize_t sendto(int sockfd, const void *buf, size_t len, int flags,
               const struct sockaddr *dest_addr, socklen_t addrlen, float lossRatio);

#endif //TINY_PACKAGELOSINGAPI_H
