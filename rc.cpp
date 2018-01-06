#include "rc.h"

#include "rc.hex/style.css.hex"

std::map<std::string, rcfile> rcs = {
	{
		"style.css",
		{"text/css", std::string((const char*)rc_style_css, rc_style_css_len)}
	},
};
