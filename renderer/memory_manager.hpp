#pragma once
#include <cstddef>

namespace memory
{
	constexpr auto buffer_size = 0x10000;
	void* alloc(std::size_t size);
	void free(void* data);

	template<typename T>
	class allocator
	{
	public:
		using value_type = T;
		using size_type = size_t;
		using difference_type = ptrdiff_t;

		T* allocate(std::size_t size)
		{
			return reinterpret_cast<T*>(memory::alloc(size * sizeof(T)));
		}

		void deallocate(T* ptr, std::size_t size)
		{
			memory::free(ptr);
		}
	};
}