#ifndef NODEPROBE_TYPES_H
#define NODEPROBE_TYPES_H

#include "stats.h"
#include "checks.h"
#include "headers.h"
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>

#define CONVERT_TIMESPEC(_tp) ((int) _tp.tv_sec * 1000 + (int) ((unsigned long) _tp.tv_nsec / 1000000))

#endif
