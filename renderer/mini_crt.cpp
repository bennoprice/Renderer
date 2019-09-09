#include <cstddef>
#include "memory_manager.hpp"
#pragma optimize( "", off )

extern "C" void* memset(void* dest, int ch, std::size_t count)
{
	for (auto i = 0ull; i < count; ++i)
		reinterpret_cast<char*>(dest)[i] = ch;
	return dest;
}

extern "C" void* memcpy(void* dest, const void* src, std::size_t count)
{
	for (auto i = 0ull; i < count; ++i)
		reinterpret_cast<char*>(dest)[i] = reinterpret_cast<const char*>(src)[i];
	return dest;
}

extern "C" void* memmove(void* dest, const void* src, std::size_t count)
{
	auto buf = memory::alloc(count);
	memcpy(buf, src, count);
	memcpy(dest, buf, count);
	return dest;
}

namespace std { void _Xlength_error(char const* error) { } }

#pragma optimize( "", on )