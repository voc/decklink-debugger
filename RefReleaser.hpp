#ifndef __RefReleaser__
#define __RefReleaser__

#include "log.h"

template <class T>
class RefReleaser
{
public:
	RefReleaser(T **ptr) : m_ptr(ptr) {};
	~RefReleaser() {
		if((m_ptr != nullptr) && ((*m_ptr) != nullptr)) {
			ULONG newRefCount = (*m_ptr)->Release();
			if(newRefCount == 0) {
				LOG(DEBUG3) << __PRETTY_FUNCTION__ << " released & deleted";
			}
			else {
				LOG(DEBUG3) << __PRETTY_FUNCTION__ << " released";
			}
		}
	}

private:
	T **m_ptr;
};

#endif
