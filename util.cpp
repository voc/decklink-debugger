#include "util.h"

void throwIfNotOk(HRESULT result, const char* message)
{
	if(result != S_OK) {
		throw message;
	}
}

void throwIfNull(void* ptr, const char* message)
{
	if(ptr == nullptr) {
		throw message;
	}
}
