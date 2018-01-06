#ifndef __tostring__
#define __tostring__

#include <string>
#include "ProberState.h"
#include "DeckLinkAPI.h"

std::string proberStateToString(ProberState proberState);
std::string videoConnectionToString(BMDVideoConnection videoConnection);

#endif
