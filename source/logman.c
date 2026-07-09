#include "logman.h"

logman_context_t logc = {
	.logfile = NULL,
	.buffer = NULL,
	.pos = 0UL,
	.size = 0UL,
	.logcount = 0UL
};

static char *sstrncpy(char *dst, const char *src, size_t dsize)
{
	size_t dlen;
	for (dlen = 0; (dlen < dsize) && (src[dlen] != '\0'); dlen++);
	memcpy(dst, src, dlen);
	dst[(dsize == dlen) ? dsize - 1 : dlen] = '\0';
	return dst;
}

static int create_logfile(int fd)
{
	logc.logfile = fdopen(fd, "w+");
	if (logc.logfile == NULL)
		return -1;
	if (ftruncate(fileno(logc.logfile), (off_t) logc.size) == -1)
		return -1;
	return 0;
}

static int map_logfile(int fd)
{
	logc.buffer = mmap(NULL, logc.size, PROT_WRITE | PROT_READ, MAP_SHARED, fd, 0);
	if (logc.buffer == MAP_FAILED)
		return -1;
	logc.pos = logc.logcount = 0;
	return 0;
}

int logman_create_context(int fd, size_t count)
{
	logc.size = count * sizeof(logman_msg_t);
	if (create_logfile(fd) == -1) {
		fprintf(stderr, "function %s() failed at line %d; %s\n", __func__, __LINE__, strerror(errno));
		return -1;
	}
	if (map_logfile(fd) == -1) {
		fprintf(stderr, "function %s() failed at line %d; %s\n", __func__, __LINE__, strerror(errno));
		return -1;
	}
	if (sem_init(&logc.sem, 0, 1) == -1) {
		fprintf(stderr, "function %s() failed at line %d; %s\n", __func__, __LINE__, strerror(errno));
		return -1;
	}
	return 0;
}

int logman_delete_context(void)
{
	return (fclose(logc.logfile) + munmap(logc.buffer, logc.size) == 0) ? 0 : -1;
}

void logging(const char *level, const char *mod, const char *pos, const char *fmt, ...)
{
	if (logc.logfile == NULL)
		return;
	logman_msg_t logmsg;
	time_t logtime = time(NULL);
	struct tm *ltime = localtime(&logtime);
	char msg[MSGSIZE];
	va_list ap;
	if (ltime == NULL)
		return;
	sstrncpy(logmsg.level, level, LEVELSIZE);
	strftime(logmsg.date, DATESIZE, "%Y-%m-%d", ltime);
	strftime(logmsg.clock, CLOCKSIZE, "%H:%M:%S", ltime);
	sstrncpy(logmsg.mod, mod, MODSIZE);
	sstrncpy(logmsg.pos, pos, POSSIZE);
	va_start(ap, fmt);
	vsnprintf(msg, sizeof(msg), fmt, ap);
	sstrncpy(logmsg.msg, msg, MSGSIZE);
	va_end(ap);
	logc.logcount++;
	if (sem_wait(&logc.sem) == -1)
		return;
	append_log(logmsg);
	if (sem_post(&logc.sem) == -1)
		fprintf(stderr, "[CRITICAL ERROR] sem_post() failed\n");
	return;
}
