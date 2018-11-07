#ifndef __INC_DEBUG_C_H__
#define __INC_DEBUG_C_H__

#ifdef OS_LINUX
#include <stdio.h>
#endif

#ifdef FN_DEBUG
#include <sys/time.h>
#include <time.h>

#define MAX_LOG_DEV_NUM 10

enum
{
    LOCAL_FILE = 1 << 0,
    ETHER_NTCP = 1 << 1,
    ETHER_NUDP = 1 << 2
};

typedef struct _log_dev_
{
    FILE* log_steam;
    int   log_fd;
    char  log_type;
    void* netClient;
} log_dev;

int log_buf(const char *format, ...);
void hexdump(char* data, int len) ;
void log_to_local_file(char* name);
void log_to_ether_ntcp(char* host, int port);

#define log_time() \
	{ \
		struct timeval time_val; \
		struct tm * time_info; \
		gettimeofday (&time_val, NULL); \
		time_info = localtime (&time_val.tv_sec); \
		log_buf("%02d-%02d %02d:%02d:%02d.%06ld ",time_info->tm_mon + 1, time_info->tm_mday, \
		time_info->tm_hour, time_info->tm_min, time_info->tm_sec, time_val.tv_usec); \
	}

#define log_fun(...) { log_buf("%s ", __func__);}
#define log_fli(...) { log_buf("%s %d ", __FILE__, __LINE__);}
#define log_err(...) { log_fli(); log_buf(__VA_ARGS__); log_buf("\n"); }
#define log_ver(...) { log_buf(__VA_ARGS__); log_buf("\n"); }
#define log_dbg(...) { log_time(); log_fun(); log_buf(__VA_ARGS__); log_buf("\n"); }
#define log_hex(...) { hexdump(__VA_ARGS__); }
#else
#define log_err(...)
#define log_ver(...)
#define log_dbg(...)
#define log_hex(...)
#endif

#endif
