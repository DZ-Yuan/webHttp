#ifndef PCH_H
#define PCH_H

#include <iostream>
#include <thread>
#include <mutex>
#include <functional>
#include <condition_variable>
#include <vector>
#include <deque>

#include <string.h>    // memset
#include <sys/types.h> // open
#include <sys/stat.h>
#include <sys/fcntl.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/sendfile.h>

#include <assert.h>

#include "Http.h"
#include "threadPools.h"

string URLdecode(const string &URL);


#endif // !PCH_H