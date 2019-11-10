#include "renderer.hpp"
#include "shaders.hpp"
#pragma comment (lib, "d3d11.lib")

namespace rendering
{
	renderer::renderer(IDXGISwapChain* swapchain)
		: renderer(swapchain, get_device(swapchain))
	{ }

	renderer::renderer(IDXGISwapChain* swapchain, ID3D11Device* device)
		: renderer(swapchain, device, get_device_context(device))
	{ }

	renderer::renderer(IDXGISwapChain* swapchain, ID3D11Device* device, ID3D11DeviceContext* device_context)
		: _device_context(device_context)
	{
		// create shaders
		device->CreateVertexShader(shader::vertex, sizeof(shader::vertex), nullptr, &_vertex_shader);
		device->CreatePixelShader(shader::pixel, sizeof(shader::pixel), nullptr, &_pixel_shader);

		// create input layout
		std::array<D3D11_INPUT_ELEMENT_DESC, 2> ied
		{{
			{"POSITION", 0u, DXGI_FORMAT_R32G32_FLOAT, 0u, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0u},
			{"COLOUR", 0u, DXGI_FORMAT_R32G32B32A32_FLOAT, 0u, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0u}
		}};
		device->CreateInputLayout(ied.data(), ied.size(), shader::vertex, sizeof(shader::vertex), &_input_layout);

		// create vertex buffer
		D3D11_BUFFER_DESC buffer_desc = { };
		buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
		buffer_desc.ByteWidth = max_vertices * sizeof(vertex);
		buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		device->CreateBuffer(&buffer_desc, nullptr, &_vertex_buffer);

		// create backbuffer view
		ID3D11Texture2D* backbuffer;
		swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backbuffer));
		device->CreateRenderTargetView(backbuffer, nullptr, &_backbuffer_view);
		backbuffer->Release();

		// create screen projection buffer
		D3D11_BUFFER_DESC projection_buffer_desc;
		projection_buffer_desc.ByteWidth = sizeof(matrix4x4);
		projection_buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
		projection_buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		projection_buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		projection_buffer_desc.MiscFlags = 0;
		device->CreateBuffer(&projection_buffer_desc, nullptr, &_projection_buffer);

		// map projection matrix into constant vram buffer
		D3D11_VIEWPORT viewport;
		UINT viewport_num = 1;
		_device_context->RSGetViewports(&viewport_num, &viewport);

		float L = viewport.TopLeftX;
		float R = viewport.TopLeftX + viewport.Width;
		float T = viewport.TopLeftY;
		float B = viewport.TopLeftY + viewport.Height;

		matrix4x4 matrix
        {{
            { 2.0f/(R-L),   0.0f,           0.0f,       0.0f },
            { 0.0f,         2.0f/(T-B),     0.0f,       0.0f },
            { 0.0f,         0.0f,           0.5f,       0.0f },
            { (R+L)/(L-R),  (T+B)/(B-T),    0.5f,       1.0f },
        }};

		D3D11_MAPPED_SUBRESOURCE ms;
		_device_context->Map(_projection_buffer, 0u, D3D11_MAP_WRITE_DISCARD, 0u, &ms);
		std::memcpy(ms.pData, matrix.data(), sizeof(matrix));
		_device_context->Unmap(_projection_buffer, 0u);
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

	void renderer::begin()
	{
		// backup render state
		_state_saver.backup(_device_context);

		// set shaders
		_device_context->VSSetShader(_vertex_shader, nullptr, 0u);
		_device_context->PSSetShader(_pixel_shader, nullptr, 0u);

		// set projection buffer
		_device_context->VSSetConstantBuffers(0u, 1u, &_projection_buffer);

		// set vertex buffer
		UINT stride = sizeof(vertex);
		UINT offset = 0u;
		_device_context->IASetVertexBuffers(0u, 1u, &_vertex_buffer, &stride, &offset);

		// set render target
		_device_context->OMSetRenderTargets(1, &_backbuffer_view, nullptr);

		// set input layout
		_device_context->IASetInputLayout(_input_layout);
	}

	void renderer::end()
	{
		draw();

		// restore render state
		_state_saver.restore();
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
			{ pos, colour },
			{ pos.x + dimensions.x, pos.y, colour },
			{ pos.x, pos.y + dimensions.y, colour },
			{ pos + dimensions, colour },
		}},
		D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	}

	void renderer::draw_line(vec2 start, vec2 end, colour colour)
	{
		add_vertices<2>(
		{{
			{ start, colour },
			{ end, colour }
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

	void state_saver::backup(ID3D11DeviceContext* device_context)
	{
		_device_context = device_context;

		_device_context->IAGetPrimitiveTopology(&_primitive_topology);
		_device_context->VSGetShader(&_vertex_shader.shader, _vertex_shader.instances, &_vertex_shader.instance_count);
		_device_context->PSGetShader(&_pixel_shader.shader, _pixel_shader.instances, &_pixel_shader.instance_count);
		_device_context->VSGetConstantBuffers(0u, 1u, &_constant_buffer);
		_device_context->IAGetVertexBuffers(0u, 1u, &_vertex_buffer.buffer, &_vertex_buffer.stride, &_vertex_buffer.offset);
		_device_context->IAGetInputLayout(&_input_layout);
	}
	
	void state_saver::restore()
	{
		_device_context->IASetPrimitiveTopology(_primitive_topology);
		_device_context->VSSetShader(_vertex_shader.shader, _vertex_shader.instances, _vertex_shader.instance_count);
		_device_context->PSSetShader(_pixel_shader.shader, _pixel_shader.instances, _pixel_shader.instance_count);
		_device_context->VSSetConstantBuffers(0u, 1u, &_constant_buffer);
		_device_context->IASetVertexBuffers(0u, 1u, &_vertex_buffer.buffer, &_vertex_buffer.stride, &_vertex_buffer.offset);
		_device_context->IASetInputLayout(_input_layout);

		if (_vertex_shader.shader) _vertex_shader.shader->Release();
		if (_pixel_shader.shader) _pixel_shader.shader->Release();
		if (_constant_buffer) _constant_buffer->Release();
		if (_vertex_buffer.buffer) _vertex_buffer.buffer->Release();
		if (_input_layout) _input_layout->Release();
	}
}