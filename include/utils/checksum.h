#ifndef NODEPROBE_CHECKSUM_H
#define NODEPROBE_CHECKSUM_H

#include <stddef.h>
#include <stdint.h>

static inline uint16_t checksum(uint8_t *data, size_t size)
{
	register uint32_t result = 0;
	while (size > 1) {
		result += (((uint16_t) *data) << 8) | ((uint16_t) *(data + 1));
		data += 2;
		size -= 2;
	}
	if (size == 1)
		result += ((uint16_t) *data) << 8;
	while ((result >> 16) != 0)
		result = (result >> 16) + (result & 0xffff);
	return ~((uint16_t) result);
}

#endif
