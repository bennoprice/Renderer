#include <array>
#include "renderer.hpp"
#include "shaders.hpp"
#pragma comment (lib, "d3d11.lib")

namespace rendering
{
	renderer::renderer(ID3D11Device* device)
		: renderer(device, get_device_context(device))
	{ }

	renderer::renderer(IDXGISwapChain* swapchain)
		: renderer(get_device(swapchain))
	{ }

	renderer::renderer(ID3D11Device* device, ID3D11DeviceContext* device_context)
		: _device(device)
		, _device_context(device_context)
	{
		// create shaders
		_device->CreateVertexShader(shader::vertex, sizeof(shader::vertex), nullptr, &_vertex_shader);
		_device->CreatePixelShader(shader::pixel, sizeof(shader::pixel), nullptr, &_pixel_shader);

		// create input layout
		std::array<D3D11_INPUT_ELEMENT_DESC, 2> ied
		{{
			{"POSITION", 0u, DXGI_FORMAT_R32G32_FLOAT, 0u, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0u},
			{"COLOUR", 0u, DXGI_FORMAT_R32G32B32A32_FLOAT, 0u, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0u}
		}};
		_device->CreateInputLayout(ied.data(), ied.size(), shader::vertex, sizeof(shader::vertex), &_input_layout);

		// create vertex buffer
		D3D11_BUFFER_DESC bd = { };
		bd.Usage = D3D11_USAGE_DYNAMIC;
		bd.ByteWidth = max_vertices * sizeof(vertex);
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		_device->CreateBuffer(&bd, nullptr, &_vertex_buffer);

		// get viewport
		UINT viewport_num = 1u;
		_device_context->RSGetViewports(&viewport_num, &_viewport);
	}

	renderer::~renderer() noexcept
	{
		_input_layout->Release();
		_vertex_shader->Release();
		_pixel_shader->Release();
		_vertex_buffer->Release();
	}

	void* renderer::operator new(std::size_t size)
	{
		return memory::alloc(size);
	}

	ID3D11Device* renderer::get_device(IDXGISwapChain* swapchain) const
	{
		ID3D11Device* device;
		swapchain->GetDevice(__uuidof(ID3D11Device), reinterpret_cast<void**>(&device));
		return device;
	}

	ID3D11DeviceContext* renderer::get_device_context(ID3D11Device* device) const
	{
		ID3D11DeviceContext* context;
		device->GetImmediateContext(&context);
		return context;
	}

	void renderer::begin() const
	{
		// set shaders
		_device_context->VSSetShader(_vertex_shader, nullptr, 0u);
		_device_context->PSSetShader(_pixel_shader, nullptr, 0u);

		// set input layout
		_device_context->IASetInputLayout(_input_layout);

		// set vertex buffer
		UINT stride = sizeof(vertex);
		UINT offset = 0u;
		_device_context->IASetVertexBuffers(0u, 1u, &_vertex_buffer, &stride, &offset);
	}

	void renderer::end()
	{
		draw();
	}

	void renderer::draw()
	{
		if (_vertices.size() <= 0u)
			return;

		// copy vertices to vram
		D3D11_MAPPED_SUBRESOURCE ms;
		_device_context->Map(_vertex_buffer, 0u, D3D11_MAP_WRITE_DISCARD, 0u, &ms);
		memcpy(ms.pData, _vertices.data(), _vertices.size() * sizeof(vertex));
		_device_context->Unmap(_vertex_buffer, 0u);

		// draw batches
		std::size_t offset = 0ull;
		for (auto batch : _batches)
		{
			_device_context->IASetPrimitiveTopology(batch.topology);
			_device_context->Draw(batch.count, offset);

			offset += batch.count;
		}

		// clear vertices and batches
		_vertices.clear();
		_batches.clear();
	}

	void renderer::draw_filled_box(vec2 pos, vec2 dimensions, colour colour)
	{
		add_vertices<4>(
		{{
			{ pos.x, pos.y, colour },
			{ pos.x, pos.y + dimensions.y, colour },
			{ pos.x + dimensions.x, pos.y, colour },
			{ pos.x + dimensions.x, pos.y + dimensions.y, colour },
		}},
		D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	}

	void renderer::draw_line(vec2 start, vec2 end, colour colour)
	{
		add_vertices<2>(
		{{
			{ start.x, start.y, colour },
			{ end.x, end.y, colour },
		}},
		D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	}

	void renderer::draw_box(vec2 pos, vec2 dimensions, colour colour)
	{
		add_vertices<5>(
		{{
			{ pos, colour },
			{ pos.x, pos.y + dimensions.y, colour},
			{ pos + dimensions, colour},
			{ pos.x + dimensions.x, pos.y, colour},
			{ pos, colour }
		}},
		D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
	}

	/*void renderer::draw_circle(vec2 pos, float radius, colour colour)
	{
		constexpr auto segments = 256u;

		std::array<vertex, segments + 1> vertices;
		for (auto i = 0u; i <= segments; ++i)
		{
			float theta = 2.f * pi<float> * static_cast<float>(i) / static_cast<float>(segments);
			vertices[i] = { pos.x + radius * std::cos(theta), pos.y + radius * std::sin(theta), colour };
		}
		add_vertices(vertices, D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
	}*/
}