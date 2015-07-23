/*
 Copyright 2013 Michael Pavone
 This file is part of BlastEm. 
 BlastEm is free software distributed under the terms of the GNU General Public License version 3 or greater. See COPYING for full license text.
*/
#include <sys/mman.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>

#include "mem.h"
#ifndef MAP_ANONYMOUS
#define MAP_ANONYMOUS MAP_ANON
#endif

#ifdef __amd64
int hint_location;
#define PAGE_MASK     (getpagesize() - 1)
#define round_page(x) ((((uintptr_t)(x)) + PAGE_MASK) & ~(PAGE_MASK))
#endif

void * alloc_code(size_t *size)
{
	static char *map_hint = 0;
#if defined(__amd64)
	// Try to request one that is close to our memory location if we're in high memory.
	// We use a dummy global variable to give us a good location to start from.
	if (!map_hint)
	{
		if ((uintptr_t) &hint_location > 0xFFFFFFFFULL)
			map_hint = (char*)round_page(&hint_location) - 0x20000000; // 0.5gb lower than our approximate location
		else
			map_hint = (char*)0x20000000; // 0.5GB mark in memory
	}
	else if ((uintptr_t) map_hint > 0xFFFFFFFFULL)
	{
		map_hint -= round_page(*size); /* round down to the next page if we're in high memory */
	}
#endif

	*size = round_page(*size);
	uint8_t *ret = mmap(map_hint, *size, PROT_EXEC | PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS
#if defined(MAP_32BIT) && defined(__amd64)
			| ((uintptr_t)map_hint == 0 ? MAP_32BIT : 0)
#endif
			, -1, 0);

	if (ret == MAP_FAILED) {
		perror("alloc_code");
		return NULL;
	}
#if defined(__amd64)
	if ((uintptr_t) map_hint <= 0xFFFFFFFF)
		map_hint += round_page(*size); /* round up if we're below 32-bit mark, probably allocating sequentially */
#endif

	return ret;
}

