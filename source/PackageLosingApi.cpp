//
// Created by robert on 13.01.2020.
//
#include "PackageLosingApi.h"

ssize_t sendto(int sockfd, const void *buf, size_t len, int flags,
               const struct sockaddr *dest_addr, socklen_t addrlen, float lossRatio){

    if(lossRatio < 0) {
        lossRatio = 0;
    } else if(lossRatio > 1) {
        lossRatio = 1;
    }

    // sprawdza czy wyslac pakiet
    std::random_device rd{};
    std::mt19937 gen{rd()};

    // number of workers
    std::uniform_int_distribution<> randomPercent{0,1};
    float randomNumber = randomPercent(gen);
    if(randomNumber < lossRatio){
        return -1;
    }

    return sendto(sockfd, buf, len, flags, dest_addr, addrlen);
}

