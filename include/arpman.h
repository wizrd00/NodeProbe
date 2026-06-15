#ifndef NODEPROBE_ARPMAN_H
#define NODEPROBE_ARPMAN_H

#include "types.h"
#include <stdlib.h>

typedef struct {
	int sock_fd;
} arpman_context_t;

status_t arpman_create_context(arpman_context_t *restrict context);

status_t arpman_delete_context(arpman_context_t *restrict context);

#endif
