#pragma once
#include "Window.hpp"
#include <vulkan/vulkan.h>
namespace club {
class Renderer {
public:
 explicit Renderer(const Window& window);
 ~Renderer();
 Renderer(const Renderer&)=delete;
 Renderer& operator=(const Renderer&)=delete;
 void beginFrame();
 void endFrame();
 void waitIdle();
private:
 void createInstance();
 void createSurface(SDL_Window*);
 void selectPhysicalDevice();
 void createDevice();
 VkInstance instance_{VK_NULL_HANDLE};
 VkSurfaceKHR surface_{VK_NULL_HANDLE};
 VkPhysicalDevice physicalDevice_{VK_NULL_HANDLE};
 VkDevice device_{VK_NULL_HANDLE};
 VkQueue graphicsQueue_{VK_NULL_HANDLE},presentQueue_{VK_NULL_HANDLE};
 uint32_t graphicsFamily_{0},presentFamily_{0};
};
}
