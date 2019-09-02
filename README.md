# Renderer
 an importless, fast, and lightweight d3d11 renderer.
- no imports
- primitive batching
- precompiled shaders (removes need for d3dcompile)
- simple interface
- simple shader
- proper cleanup in destructor to prevent memory leaks
- multiple constructors which accept either a swapchain or device
- pixel coord to standard coord translation (without writing a custom vertex shader and mapping the projection matrix into a gpu register)

```cpp
renderer->begin();

renderer->draw_box({ 50.f, 50.f }, { 50.f, 50.f }, { 1.f, 0.f, 0.f, 1.f });
renderer->draw_line({ 50.f, 125.f }, { 100.f, 125.f }, { 0.f, 0.f, 1.f, 1.f });
renderer->draw_filled_box({ 50.f, 150.f }, { 50.f, 50.f }, { 1.f, 0.f, 0.f, 1.f });
renderer->draw_circle({ 75.f, 250.f }, 25.f, { 0.f, 0.f, 1.f, 1.f });

renderer->end();
```

![example](https://i.gyazo.com/41d82bf5aebc0c6375cdfb7c548c078f.png)
