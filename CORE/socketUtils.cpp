#include "socketUtils.h"

#include <sys/socket.h>

bool setSocketTimeout(int fileDescriptor, std::chrono::microseconds microseconds)
{
    struct timeval timeout;
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(microseconds);
    timeout.tv_sec = seconds.count();
    timeout.tv_usec = (microseconds - seconds).count();

    return setsockopt(fileDescriptor, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) >= 0;
}