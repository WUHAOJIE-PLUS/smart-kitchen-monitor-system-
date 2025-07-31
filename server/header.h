#ifndef MY_HEADER_H
#define MY_HEADER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/select.h>
#include <time.h>
#include <sys/wait.h>
#include <dirent.h>
#include <syslog.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <signal.h>
#include <sys/time.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/epoll.h>
#include <sys/sendfile.h>
#include <stdbool.h>

// 检查 main 函数参数个数
#define ARGS_CHECK(argc, num) \
    if ((argc) != (num)) { \
        fprintf(stderr, "Usage: %s <arg1> ... <arg%d>\n", argv[0], (num)-1); \
        return -1; \
    }

// 检查返回 int 类型的函数是否出错，如 socket(), open()
#define ERROR_CHECK(ret, num, msg) \
    if ((ret) == (num)) { \
        perror(msg); \
        return -1; \
    }

// 检查指针是否为 NULL
#define PTR_CHECK(ptr, msg) \
    if ((ptr) == NULL) { \
        fprintf(stderr, "%s\n", msg); \
        return -1; \
    }


// 线程相关函数错误检查
#define THREAD_ERROR_CHECK(ret, msg) \
    if ((ret) != 0) { \
        fprintf(stderr, "%s: %s\n", msg, strerror(ret)); \
    }

#endif
