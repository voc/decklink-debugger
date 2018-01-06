#ifndef __tostring__
#define __tostring__

#include <string>
#include "DeckLinkAPI.h"

std::string videoConnectionToString(BMDVideoConnection videoConnection);
std::string pixelFormatToString(BMDPixelFormat pixelFormat);
std::string boolToString(bool value);

#endif
