#ifndef CBAPI_H
#define CBAPI_H

#include <unistd.h>
#include <string.h>

#define CBAPI_BUFFER_SIZE 1024
#define CBAPI_BUFFER_TEXT_BEGIN 0

extern char colorbuf[CBAPI_BUFFER_SIZE];

#define cbGetLocation() ( colorbuf + CBAPI_BUFFER_TEXT_BEGIN )
#define cbGetPointer() ( cbGetLocation() + strlen(cbGetLocation()) )
#define cbGetSize() ( CBAPI_BUFFER_SIZE - CBAPI_BUFFER_TEXT_BEGIN )

#define cbPushBuffer() do { \
	colorbuf[CBAPI_BUFFER_TEXT_BEGIN] = '\0'; \
} while (0)

#define cbColorv(col) do { \
	strcpy(cbGetPointer(), "\x1B[" col "m"); \
} while(0)

#define cbprintf(...) do { \
	char *currentPtr = cbGetPointer(); \
	snprintf(currentPtr, cbGetLocation() + cbGetSize() - currentPtr, __VA_ARGS__); \
} while (0)

#define cbPopBuffer() do { \
	strcpy(cbGetPointer(), "\x1B[0m"); \
	printf("%s", colorbuf); \
} while (0)

#endif
