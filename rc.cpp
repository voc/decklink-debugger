#include "rc.h"

#include "rc.hex/no-capture.png.hex"
#include "rc.hex/style.css.hex"

std::map<std::string, rcfile> rcs = {
	{
		"no-capture.png",
		{"image/png", std::string((const char*)rc_no_capture_png, rc_no_capture_png_len)}
	},
	{
		"style.css",
		{"text/css", std::string((const char*)rc_style_css, rc_style_css_len)}
	},
};
