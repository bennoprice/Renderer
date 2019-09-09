#pragma once
#include <vector>
#include <d3d11.h>
#include "memory_manager.hpp"

namespace rendering
{
	constexpr auto max_vertices = 1000;
	template<typename T> constexpr T pi = 3.14159265359;

	struct colour
	{
		float r, g, b, a;
	};

	struct vec2
	{
		float x, y;

		vec2 operator+(vec2 vec) const
		{
			return { x + vec.x, y + vec.y };
		}
	};

	struct vertex
	{
		vec2 pos;
		colour colour;
	};

	struct batch
	{
		explicit batch(std::size_t count, D3D11_PRIMITIVE_TOPOLOGY topology)
			: count(count)
			, topology(topology)
		{ }

		std::size_t count;
		D3D11_PRIMITIVE_TOPOLOGY topology;
	};

	class renderer
	{
	public:
		explicit renderer(ID3D11Device* device);
		explicit renderer(IDXGISwapChain* swapchain);
		explicit renderer(ID3D11Device* device, ID3D11DeviceContext* device_context);
		~renderer() noexcept;

		void* operator new(std::size_t size);

		void begin() const;
		void draw();
		void end();

		template<std::size_t N>
		void add_vertices(std::array<vertex, N> vertices, D3D11_PRIMITIVE_TOPOLOGY topology)
		{
			// translate pixel coords to standard coords
			for (auto& vertex : vertices)
			{
				vertex.pos.x = (vertex.pos.x / (_viewport.Width / 2.f)) - 1.f;
				vertex.pos.y = (vertex.pos.y / (_viewport.Height / 2.f)) - 1.f;
			}

			if (_vertices.size() + N > max_vertices)
				draw();

			_vertices.resize(_vertices.size() + N);
			memcpy(&_vertices[_vertices.size() - N], vertices.data(), N * sizeof(vertex));

			_batches.emplace_back(N, topology);
		}

		void draw_filled_box(vec2 pos, vec2 dimensions, colour colour);
		void draw_box(vec2 pos, vec2 dimensions, colour colour);
		//void draw_circle(vec2 pos, float radius, colour colour);
		void draw_line(vec2 start, vec2 end, colour colour);
	private:
		ID3D11Device* get_device(IDXGISwapChain* swapchain) const;
		ID3D11DeviceContext* get_device_context(ID3D11Device* device) const;

		ID3D11Device* _device;
		ID3D11DeviceContext* _device_context;
		ID3D11VertexShader* _vertex_shader;
		ID3D11PixelShader* _pixel_shader;
		ID3D11InputLayout* _input_layout;
		ID3D11Buffer* _vertex_buffer;
		D3D11_VIEWPORT _viewport;

		std::vector<vertex, memory::allocator<vertex>> _vertices;
		std::vector<batch, memory::allocator<batch>> _batches;
	};
}