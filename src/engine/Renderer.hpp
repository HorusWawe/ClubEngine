#pragma once
#include "Window.hpp"
#include <vulkan/vulkan.h>
#include <vector>
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
 void createSwapchain();
 void createRenderPass();
 void createFramebuffers();
 void createCommandPool();
 void createCommandBuffers();
 void createSync();
 void cleanupSwapchain();
 void recreateSwapchain();
 VkInstance instance_{VK_NULL_HANDLE};
 VkSurfaceKHR surface_{VK_NULL_HANDLE};
 VkPhysicalDevice physicalDevice_{VK_NULL_HANDLE};
 VkDevice device_{VK_NULL_HANDLE};
 VkQueue graphicsQueue_{VK_NULL_HANDLE},presentQueue_{VK_NULL_HANDLE};
 uint32_t graphicsFamily_{0},presentFamily_{0};
 VkSwapchainKHR swapchain_{VK_NULL_HANDLE};
 VkFormat swapFormat_{VK_FORMAT_UNDEFINED};
 VkExtent2D extent_{};
 std::vector<VkImage> images_;
 std::vector<VkImageView> views_;
 VkRenderPass renderPass_{VK_NULL_HANDLE};
 std::vector<VkFramebuffer> framebuffers_;
 VkCommandPool commandPool_{VK_NULL_HANDLE};
 std::vector<VkCommandBuffer> commands_;
 static constexpr uint32_t FramesInFlight=2;
 VkSemaphore imageAvailable_[FramesInFlight]{};
 VkSemaphore renderFinished_[FramesInFlight]{};
 VkFence inFlight_[FramesInFlight]{};
 uint32_t frame_{0};
 uint32_t imageIndex_{0};
 bool frameStarted_{false};
};
}
