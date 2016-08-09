#ifndef __HTTPED_H__
#define __HTTPED_H__

#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<sys/socket.h>
#include<pthread.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<errno.h>
#include<string.h>
#include<unistd.h>
#include<fcntl.h>

#define _BACK_LOG_ 5
#define MAX 2048
#define MAIN_PAGE "index.html"
#define HTTP_VERSION "HTTP/1.0"
#endif
