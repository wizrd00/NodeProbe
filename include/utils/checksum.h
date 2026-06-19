#ifndef NODEPROBE_CHECKSUM_H
#define NODEPROBE_CHECKSUM_H

#include <stddef.h>
#include <stdint.h>

static inline uint16_t checksum(uint8_t *data, size_t size)
{
	uint16_t chksum = 0;
	uint64_t tmp = 0;
	for (size_t i = 0; i < size / 2; i++)
		tmp += ((uint64_t) data[i * 2] << 8) + (uint64_t) data[i * 2 + 1];
	if (size % 2 != 0)
		tmp += (uint64_t) (data[size - 1] << 8);
	while (tmp > 0) {
		chksum += (uint64_t) (tmp & 0xffff);
		tmp >>= 16;
	}
	return (~chksum);
}

#endif
