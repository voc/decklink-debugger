#ifndef __RefDeleter__
#define __RefDeleter__

#include "log.h"

template <class T>
class RefDeleter
{
public:
	RefDeleter(T **ptr) : m_ptr(ptr) {};
	~RefDeleter() {
		LOG(DEBUG1) << __PRETTY_FUNCTION__ << " deleting";
		if((m_ptr != nullptr) && ((*m_ptr) != nullptr)) {
			delete (*m_ptr);
			(*m_ptr) = nullptr;
		}
	}

private:
	T **m_ptr;
};

#endif
