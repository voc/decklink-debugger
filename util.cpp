#include "util.h"

void throwIfNotOk(HRESULT result, const char* message) {
	if(result != S_OK) {
		throw message;
	}
}
