#include "tostring.h"

std::string videoConnectionToString(BMDVideoConnection videoConnection)
{
	switch(videoConnection)
	{
		case bmdVideoConnectionSDI:
			return "SDI";

		case bmdVideoConnectionHDMI:
			return "HDMI";

		case bmdVideoConnectionOpticalSDI:
			return "OpticalSDI";

		default:
			return "";
	}
}

std::string pixelFormatToString(BMDPixelFormat pixelFormat)
{
	switch(pixelFormat)
	{
		case bmdFormat8BitYUV:
			return "8 Bit YUV";

		case bmdFormat10BitYUV:
			return "10 Bit YUV";

		case bmdFormat8BitARGB:
			return "8 Bit ARGB";

		case bmdFormat8BitBGRA:
			return "8 Bit BGRA";

		case bmdFormat10BitRGB:
			return "Big-endian RGB 10-bit per component with SMPTE video levels (64-960). Packed as 2:10:10:10";

		case bmdFormat12BitRGB:
			return "Big-endian RGB 12-bit per component with full range (0-4095). Packed as 12-bit per component";

		case bmdFormat12BitRGBLE:
			return "Little-endian RGB 12-bit per component with full range (0-4095). Packed as 12-bit per component";

		case bmdFormat10BitRGBXLE:
			return "Little-endian 10-bit RGB with SMPTE video levels (64-940)";

		case bmdFormat10BitRGBX:
			return "Big-endian 10-bit RGB with SMPTE video levels (64-940)";

		case bmdFormatH265:
			return "HEVC/h.265";

		case bmdFormatDNxHR:
			return "DNxHR";

		default:
			return "";
	}
}

std::string boolToString(bool value)
{
	return value ? "Yes" : "No";
}
