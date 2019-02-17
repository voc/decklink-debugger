#ifndef __util__
#define __util__

#include "DeckLinkAPI.h"

#define UNUSED __attribute__ ((unused))

void throwIfNotOk(HRESULT result, const char* message);

#endif
