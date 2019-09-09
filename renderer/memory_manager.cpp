#include <cstdint>
#include <cstring>
#include "memory_manager.hpp"

namespace memory
{
	struct block
	{
		std::size_t size;
		block* next;
	};

	std::uint8_t buffer[buffer_size];

	void* alloc(std::size_t size)
	{
		if (size == 0)
			return nullptr;

		auto current = reinterpret_cast<block*>(buffer);

		while (true)
		{
			if (current->size == 0ull && (!current->next || ((uint64_t)current->next - ((uint64_t)current + sizeof(block))) >= size))
				break;
			current = current->next;
		}

		if ((uint64_t)current + sizeof(block) + size > (uint64_t)buffer + buffer_size)
			return nullptr;

		current->size = size;

		auto next = current->next;
		current->next = reinterpret_cast<block*>((uint64_t)current + sizeof(block) + size);
		if (!current->next->next)
			current->next->next = next;

		return reinterpret_cast<void*>((uint64_t)current + sizeof(block));
	}

	void free(void* data)
	{
		auto current = reinterpret_cast<block*>((uint64_t)data - sizeof(block));
		memset(data, 0, current->size);
		current->size = 0;
	}
}