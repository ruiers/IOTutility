#ifdef OS_LINUX
#include <stdio.h>
#endif

#ifdef FN_DEBUG
#include <sys/time.h>
#include <time.h>

#define log_time() \
	{ \
		struct timeval time_val; \
		struct tm * time_info; \
		gettimeofday (&time_val, NULL); \
		time_info = localtime (&time_val.tv_sec); \
		printf("%02d-%02d %02d:%02d:%02d.%06ld ",time_info->tm_mon + 1, time_info->tm_mday, \
		time_info->tm_hour, time_info->tm_min, time_info->tm_sec, time_val.tv_usec); \
	}

#define log_hex(data, len) \
{ \
    int i = 0, value = 0; \
    for (i = 0; i < len; i++) \
    { \
        value = *((unsigned char *) data + i); \
        if ((i) % 2 == 0) \
        { \
            printf(" "); \
        } \
        if ((i) % 16 == 0) \
        { \
            printf("\n"); \
            printf("%p: ", data + i); \
        } \
        printf("%02x", value); \
    } \
    printf("\n"); \
}

#define log_fun(...) printf("%s ", __func__);
#define log_fli(...) printf("%s %d ", __FILE__, __LINE__);
#define log_err(...) { log_fli(); printf(__VA_ARGS__); printf("\n"); }
#define log_ver(...) { printf(__VA_ARGS__); printf("\n"); }
#define log_dbg(...) { log_time(); log_fun(); printf(__VA_ARGS__); printf("\n"); }
#else
#define log_err(...)
#define log_ver(...)
#define log_dbg(...)

#endif
