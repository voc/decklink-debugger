#ifndef __util__
#define __util__

#include <iterator>
#include <array>

#define UNUSED __attribute__ ((unused))

template <class T, std::size_t N>
std::ostream& operator<<(std::ostream& o, const std::array<T, N>& arr)
{
	std::copy(arr.cbegin(), arr.cend(), std::ostream_iterator<T>(o, "\t"));
	return o;
}

#endif
