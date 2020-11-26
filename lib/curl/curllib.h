#ifndef CURLLIB
#define CURLLIB
#include <stddef.h>

struct MemoryStruct {
	char *memory;
	size_t size;
};

struct MemoryStruct http(char *url);

#endif
