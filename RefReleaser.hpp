#ifndef __RefReleaser__
#define __RefReleaser__

#include "log.h"

template <class T>
class RefReleaser
{
public:
	RefReleaser(T **ptr) : m_ptr(ptr) {};
	~RefReleaser() {
		LOG(DEBUG1) << __PRETTY_FUNCTION__ << " releasing";
		if((m_ptr != nullptr) && ((*m_ptr) != nullptr)) {
			(*m_ptr)->Release();
		}
	}

private:
	T **m_ptr;
};

#endif
