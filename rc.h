#ifndef __rc__
#define __rc__

#include <string>
#include <map>

struct rcfile {
	std::string mimetype;
	std::string body;
};

extern std::map<std::string, rcfile> rcs;

#endif
