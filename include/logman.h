#ifndef NODEPROBE_LOGMAN_H
#define NODEPROBE_LOGMAN_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <sys/mman.h>
#include <semaphore.h>

#define LOGERROR_TEXT "\nError(logger) : %s, %s\n\n"
#define LOGFILE_NAMESIZE 32

#define TRACE "TRACE"
#define DEBUG "DEBUG"
#define WARNN "WARNN"
#define ERROR "ERROR"

#define LEVELSIZE 6
#define DATESIZE 11
#define CLOCKSIZE 9
#define MODSIZE 32
#define POSSIZE 32
#define MSGSIZE 512

#define append_log(log)\
	do {if (logc.pos == logc.size) {msync(logc.buffer, logc.size, MS_SYNC); logc.pos = 0;} memcpy((void *) ((char *) logc.buffer + logc.pos), (void *) &log, sizeof(logman_msg_t)); logc.pos += sizeof(logman_msg_t);} while (0)

typedef struct {
	FILE *logfile;
	void *buffer;
	size_t pos;
	size_t size;
	unsigned long logcount;
	sem_t sem;
} logman_context_t;

typedef struct {
	char level[LEVELSIZE];
	char date[DATESIZE];
	char clock[CLOCKSIZE];
	char mod[MODSIZE];
	char pos[POSSIZE];
	char msg[MSGSIZE];
} logman_msg_t;

int logman_create_context(int fd, size_t count);

int logman_delete_context(void);

void logging(const char *level, const char *mod, const char *pos, const char *fmt, ...);

#endif
