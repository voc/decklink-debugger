#include "tostring.h"

std::string proberStateToString(ProberState proberState)
{
	switch(proberState)
	{
		case SEARCHING_FOR_SIGNAL:
			return "SEARCHING_FOR_SIGNAL";

		case SIGNAL_DETECTED:
			return "SIGNAL_DETECTED";

		default:
			return "UNDEFINED";
	}
}

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
