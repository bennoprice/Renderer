#include <memory>
#include "renderer.hpp"

using present_t = HRESULT(*)(IDXGISwapChain*, UINT, UINT);
std::unique_ptr<rendering::renderer> renderer;

extern "C" present_t __declspec(dllexport) original = 0ull;
extern "C" void __declspec(dllexport) entry(IDXGISwapChain* swapchain, UINT sync_interval, UINT flags)
{
	if (static auto ran = false; !ran)
	{
		ran = true;
		renderer = std::make_unique<rendering::renderer>(swapchain);
	}

	renderer->begin();

	renderer->draw_box({ 50.f, 50.f }, { 50.f, 50.f }, { 1.f, 0.f, 0.f, 1.f });
	renderer->draw_line({ 50.f, 125.f }, { 100.f, 125.f }, { 0.f, 0.f, 1.f, 1.f });
	renderer->draw_filled_box({ 50.f, 150.f }, { 50.f, 50.f }, { 1.f, 0.f, 0.f, 1.f });
	renderer->draw_circle({ 75.f, 250.f }, 25.f, { 0.f, 0.f, 1.f, 1.f });

	renderer->end();

	original(swapchain, sync_interval, flags);
}