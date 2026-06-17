#ifndef NODEPROBE_CHECKS_H
#define NODEPROBE_CHECKS_H

#include "stats.h"
#if defined(LOG_TRACE) || defined(LOG_DEBUG) || defined(LOG_WARNN) || defined(LOG_ERROR)
#include "logman.h"
#endif
#include <unistd.h>

#ifdef LOG_TRACE
	#define LOGT(...) logging(TRACE, __FILE__, __func__, __VA_ARGS__)
#else
	#define LOGT(...)
#endif

#ifdef LOG_DEBUG
	#define LOGD(...) logging(DEBUG, __FILE__, __func__, __VA_ARGS__)
#else
	#define LOGD(...)
#endif

#ifdef LOG_WARNN
	#define LOGW(...) logging(WARNN, __FILE__, __func__, __VA_ARGS__)
#else
	#define LOGW(...)
#endif

#ifdef LOG_ERROR
	#define LOGE(...) logging(ERROR, __FILE__, __func__, __VA_ARGS__)
#else
	#define LOGE(...)
#endif

#define CHECK_STAT(stat, ...)\
	do {if (stat != SUCCESS) {LOGE("(FAILURE)" " " __VA_ARGS__); return _stat = stat;}} while(0)

#define CHECK_STAT_FREE(stat, ptr, ...)\
	do {if (stat != SUCCESS) {LOGE("(FAILURE)" " " __VA_ARGS__); free((void *) ptr); ptr = NULL; return _stat = stat;}} while(0);

#define CHECK_STAT_CLOSE(stat, ffd, ...)\
	do {if (stat != SUCCESS) {LOGE("(FAILURE)" " " __VA_ARGS__); close(ffd); return _stat = stat;}} while(0)

#define CHECK_EQUAL(val0, val1, stat, ...)\
	do {if (val0 != val1) {LOGE("(" #stat ")" " " __VA_ARGS__); return _stat = stat;}} while(0)

#define CHECK_EQUAL_FREE(val0, val1, stat, ptr, ...)\
	do {if (val0 != val1) {LOGE("(" #stat ")" " " __VA_ARGS__); free((void *) ptr); ptr = NULL; return _stat = stat;}} while(0)

#define CHECK_EQUAL_CLOSE(val0, val1, stat, ffd, ...)\
	do {if (val0 != val1) {LOGE("(" #stat ")" " " __VA_ARGS__); close(ffd); return _stat = stat;}} while(0)

#define CHECK_NOTEQUAL(val0, val1, stat, ...)\
	do {if (val0 == val1) {LOGE("(" #stat ")" " " __VA_ARGS__); return _stat = stat;}} while(0)

#define CHECK_NOTEQUAL_FREE(val0, val1, stat, ptr, ...)\
	do {if (val0 == val1) {LOGE("(" #stat ")" " " __VA_ARGS__); free((void *) ptr); ptr = NULL; return _stat = stat;}} while(0)

#define CHECK_NOTEQUAL_CLOSE(val0, val1, stat, ffd, ...)\
	do {if (val0 == val1) {LOGE("(" #stat ")" " " __VA_ARGS__); close(ffd); return _stat = stat;}} while(0)


#endif
