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
			return "UNDEFINED";
	}
}
