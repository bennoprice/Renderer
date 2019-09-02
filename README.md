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
