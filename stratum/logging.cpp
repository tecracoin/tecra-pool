

#include "logging.h"

FILE *g_debuglog = NULL;
FILE *g_stratumlog = NULL;
FILE *g_clientlog = NULL;
FILE *g_rejectlog = NULL;


const int g_stratum_install_path_bufsize = 2048;
char g_stratum_install_path[g_stratum_install_path_bufsize];
////////////////////////////////////////////////////////////////////////////////

void get_timestamp(char *buf, const int bufsize, const char *msg){
    time_t rawtime;
    struct tm * timeinfo;

    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(buf, bufsize, "[%Y-%m-%d %H:%M:%S]", timeinfo);
    if(msg && strlen(msg)) {
        sprintf(buf, "%s %s", buf, msg);
    }
}
void log_with_timestamp(FILE *log_file,const char*msg){
    char buf[g_stratum_install_path_bufsize] = {'\0'};
    get_timestamp(buf,g_stratum_install_path_bufsize,msg);
    printf("%s", g_stratum_algo);
    fprintf(log_file, "%s", buf);
}

/**
 * Returns path for a `filename` in stratum bin installation path. Note: it may or may not be the same as working directory!
 * TODO: right now it also adds subdir 'log' to the path, should change it!!!!
 * @param string filename
 * @return
 */
void get_install_path(const char* filename, char *buf, const int bufsize) {
    if(!g_stratum_install_path || !strlen(g_stratum_install_path)) {
        readlink("/proc/self/exe", g_stratum_install_path, g_stratum_install_path_bufsize);
        // Here comes neat trick to get dirname from path(source: https://stackoverflow.com/questions/3071665/getting-a-directory-name-from-a-filename):
        char *p = strrchr(g_stratum_install_path, '/');
        if (p) p[0] = 0;

//        printf(g_stratum_install_path);

        const char *temporary_solution_subdir_for_logs_should_remove_this = "log";
        sprintf(g_stratum_install_path, "%s/%s", g_stratum_install_path, temporary_solution_subdir_for_logs_should_remove_this);

        //maybe do mkdir on the path
        char mkdir_cmd[g_stratum_install_path_bufsize] = {'\0'};
        sprintf(mkdir_cmd, "mkdir -p %s", g_stratum_install_path);
        system(mkdir_cmd);
    }

    sprintf(buf, "%s/%s", g_stratum_install_path, filename);
}
FILE *open_log(const char *log_filename){
    char buf[g_stratum_install_path_bufsize] = {'\0'};// TODO: once malloc and reuse
    get_install_path(log_filename, buf, g_stratum_install_path_bufsize);
    debuglog("Opening '%s' at path [%s]", log_filename, buf);
    FILE * file = fopen(buf, "a");
    if (!file) {
        perror(buf);
        return NULL;
    }
    log_with_timestamp(file, "---- new session started ----");
    return file;
}

void initlog(const char *algo)
{
	char debugfile[2048] = {'\0'};

	sprintf(debugfile, "%s-debug.log", algo);
	g_debuglog = open_log(debugfile);

    g_stratumlog = open_log("stratum.log");
	g_clientlog = open_log("client.log");
	g_rejectlog = open_log("reject.log");
}

void closelogs()
{
	if (g_debuglog) {
		fflush(g_debuglog); fclose(g_debuglog);
	}
	if (g_stratumlog) {
		fflush(g_stratumlog); fclose(g_stratumlog);
	}
	if (g_clientlog) {
		fflush(g_clientlog); fclose(g_clientlog);
	}
	if (g_rejectlog) {
		fflush(g_rejectlog); fclose(g_rejectlog);
	}
}

void __debuglog(const char* filename, int fileline, const char *format, ...)
{
	char buffer[1024];
	va_list args;

	va_start(args, format);
	vsprintf(buffer, format, args);
	va_end(args);

	time_t rawtime;
	struct tm * timeinfo;
	char buffer2[80];

	time(&rawtime);
	timeinfo = localtime(&rawtime);

	strftime(buffer2, 80, "%H:%M:%S", timeinfo);
	printf("[%s][DEBUG] %s @%s:%d\n", buffer2, buffer, filename , fileline);
	fflush(stdout);

	if(g_debuglog)
	{
	    fprintf(g_debuglog, "[%s][DEBUG] %s @%s:%d\n", buffer2, buffer, filename , fileline);
		fflush(g_debuglog);
	}
}

void debuglog_hex(void *data, int len)
{
	uint8_t* const bin = (uint8_t*) data;
	char *hex = (char*) calloc(1, len*2 + 2);
	if (!hex) return;
	for(int i=0; i < len; i++)
		sprintf(hex+strlen(hex), "%02x", bin[i]);
	strcpy(hex+strlen(hex), "\n");
	debuglog(hex);
	free(hex);
}
void stratumlog(const char *format, ...)
{
	char buffer[1024];
	va_list args;

	va_start(args, format);
	vsprintf(buffer, format, args);
	va_end(args);

	time_t rawtime;
	struct tm * timeinfo;
	char buffer2[80];

	time(&rawtime);
	timeinfo = localtime(&rawtime);

	strftime(buffer2, 80, "%H:%M:%S", timeinfo);
	printf("[%s][INFO] %s\n", buffer2, buffer);
	fflush(stdout);

	if(g_debuglog)
	{
	    fprintf(g_debuglog,"[%s][INFO] %s\n", buffer2, buffer);
		fflush(g_debuglog);
	}

	if(g_stratumlog)
	{
	    fprintf(g_stratumlog,"[%s][INFO] %s\n", buffer2, buffer);
        // http://www.cplusplus.com/reference/cstdio/feof/
        // http://www.cplusplus.com/reference/cstdio/ferror/
        if (!g_stratumlog || feof(g_stratumlog) || ferror(g_stratumlog)) {
            if(g_stratumlog) {
                fclose(g_stratumlog);
                g_stratumlog = NULL;
            }
            g_stratumlog = open_log("stratum.log");
        }
	}
}

void stratumlogdate(const char *format, ...)
{
	char buffer[1024];
	char date[16];
	va_list args;
	time_t rawtime;
	struct tm * timeinfo;

	time(&rawtime);
	timeinfo = localtime(&rawtime);
	strftime(date, 16, "%Y-%m-%d", timeinfo);

	va_start(args, format);
	vsprintf(buffer, format, args);
	va_end(args);

	stratumlog("%s %s", date, buffer);
}

void rejectlog(const char *format, ...)
{
	char buffer[1024];
	va_list args;

	va_start(args, format);
	vsnprintf(buffer, 1024-1, format, args);
	va_end(args);

	time_t rawtime;
	struct tm * timeinfo;
	char buffer2[80];

	time(&rawtime);
	timeinfo = localtime(&rawtime);

	strftime(buffer2, 80, "%Y-%m-%d %H:%M:%S", timeinfo);
	printf("%s: %s", buffer2, buffer);

	if(g_rejectlog)
	{
		fprintf(g_rejectlog, "%s: %s", buffer2, buffer);
		if (fflush(g_rejectlog) == EOF) {
			fclose(g_rejectlog);
			g_rejectlog = open_log("reject.log");
		}
	}
}


