#ifndef NODEPROBE_STATS_H
#define NODEPROBE_STATS_H

typedef enum {
	SUCCESS,
	FAILURE,
	ERRSOCK,
	ERRBIND,
	ERRCLOS
} status_t;

#endif
