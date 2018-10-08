#pragma once

#include <sys/socket.h>
#include <stdio.h>
#include <ctime>
#include <netinet/tcp.h>
#include <sys/time.h>
#include <netinet/in.h>

namespace NAC {
    namespace NSocketUtils {
        static inline bool SetTimeout(int fh, time_t timeoutMilliseconds) {
            timeval tv;
            tv.tv_sec = (timeoutMilliseconds / 1000);
            tv.tv_usec = (timeoutMilliseconds % 1000) * 1000;

            if(setsockopt(fh, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) == -1) {
                perror("setsockopt");
                return false;
            }

            if(setsockopt(fh, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv)) == -1) {
                perror("setsockopt");
                return false;
            }

            return true;
        }

        static inline bool SetupSocket(int fh, time_t timeoutMilliseconds) {
            int yes = 1;

            if(setsockopt(fh, SOL_SOCKET, SO_KEEPALIVE, &yes, sizeof(yes)) == -1) {
                perror("setsockopt");
                return false;
            }

            if(setsockopt(fh, IPPROTO_TCP, TCP_NODELAY, &yes, sizeof(yes)) == -1) {
                perror("setsockopt");
                return false;
            }

            return SetTimeout(fh, timeoutMilliseconds);
        }
    }
}
