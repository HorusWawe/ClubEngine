
#pragma once
#include "Window.hpp"
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <vector>
namespace club {
class Renderer {
public:
 explicit Renderer(const Window& window); ~Renderer();
 Renderer(const Renderer&)=delete; Renderer& operator=(const Renderer&)=delete;
 void beginFrame(); void endFrame(); void waitIdle();
 void drawRoom(const glm::mat4& viewProjection);
private:
 void createInstance(); void createSurface(SDL_Window*); void selectPhysicalDevice(); void createDevice();
 void createSwapchain(); void createRenderPass(); void createDepthResources(); void createFramebuffers();
 void createCommandPool(); void createCommandBuffers(); void createSync(); void createPipeline(); void createRoomMesh();
 uint32_t findMemoryType(uint32_t,VkMemoryPropertyFlags) const; VkShaderModule loadShader(const char*) const;
 void cleanupSwapchain(); void recreateSwapchain();
 SDL_Window* window_{nullptr}; VkInstance instance_{VK_NULL_HANDLE}; VkSurfaceKHR surface_{VK_NULL_HANDLE};
 VkPhysicalDevice physicalDevice_{VK_NULL_HANDLE}; VkDevice device_{VK_NULL_HANDLE};
 VkQueue graphicsQueue_{VK_NULL_HANDLE},presentQueue_{VK_NULL_HANDLE}; uint32_t graphicsFamily_{0},presentFamily_{0};
 VkSwapchainKHR swapchain_{VK_NULL_HANDLE}; VkFormat swapFormat_{VK_FORMAT_UNDEFINED}; VkExtent2D extent_{};
 std::vector<VkImage> images_; std::vector<VkImageView> views_; VkRenderPass renderPass_{VK_NULL_HANDLE};
 VkFormat depthFormat_{VK_FORMAT_UNDEFINED}; VkImage depthImage_{VK_NULL_HANDLE}; VkDeviceMemory depthMemory_{VK_NULL_HANDLE}; VkImageView depthView_{VK_NULL_HANDLE};
 std::vector<VkFramebuffer> framebuffers_; VkCommandPool commandPool_{VK_NULL_HANDLE}; std::vector<VkCommandBuffer> commands_;
 VkPipelineLayout pipelineLayout_{VK_NULL_HANDLE}; VkPipeline pipeline_{VK_NULL_HANDLE};
 VkBuffer vertexBuffer_{VK_NULL_HANDLE}; VkDeviceMemory vertexMemory_{VK_NULL_HANDLE}; uint32_t vertexCount_{0};
 static constexpr uint32_t FramesInFlight=1; VkSemaphore imageAvailable_[FramesInFlight]{}; std::vector<VkSemaphore> presentReady_;
 VkFence inFlight_[FramesInFlight]{}; uint32_t frame_{0},imageIndex_{0}; bool frameStarted_{false};
};
}
