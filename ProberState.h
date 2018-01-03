#ifndef __ProberState__
#define __ProberState__

enum ProberState {
	UNDEFINED,
	SEARCHING_FOR_SIGNAL,
	SIGNAL_DETECTED,
};
static const char* ProberStateNames[] = {
	"UNDEFINED",
	"SEARCHING_FOR_SIGNAL",
	"SIGNAL_DETECTED",
};

#endif
