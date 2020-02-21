#include <stdio.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>
#include <errno.h>

#ifndef debuglog
    #define debuglog(...) __debuglog(__FILE__, __LINE__, __VA_ARGS__)
#endif

extern char g_stratum_algo[256];

extern FILE *g_debuglog;
extern FILE *g_stratumlog;
extern FILE *g_clientlog;
extern FILE *g_rejectlog;


void initlog(const char *algo);
void closelogs();

void __debuglog(const char* filename, int fileline, const char *format, ...);
void stratumlog(const char *format, ...);
void stratumlogdate(const char *format, ...);
void rejectlog(const char *format, ...);
FILE *open_log(const char *log_filename);