#include "Renderer.hpp"
#include <SDL3/SDL_vulkan.h>
#include <stdexcept>
#include <string>
#include <vector>
namespace club {
namespace { void checkVk(VkResult r,const char* op) { if(r!=VK_SUCCESS) throw std::runtime_error(std::string(op)+" failed: "+std::to_string(static_cast<int>(r))); } }
Renderer::Renderer(const Window& w) { createInstance(); createSurface(w.native()); selectPhysicalDevice(); createDevice(); }
Renderer::~Renderer() {
 if(device_) { vkDeviceWaitIdle(device_); vkDestroyDevice(device_,nullptr); }
 if(surface_&&instance_) vkDestroySurfaceKHR(instance_,surface_,nullptr);
 if(instance_) vkDestroyInstance(instance_,nullptr);
}
void Renderer::createInstance() {
 unsigned int count=0;
 const char* const* exts=SDL_Vulkan_GetInstanceExtensions(&count);
 if(!exts) throw std::runtime_error(SDL_GetError());
 VkApplicationInfo app{VK_STRUCTURE_TYPE_APPLICATION_INFO};
 app.pApplicationName="ClubEngine"; app.applicationVersion=VK_MAKE_API_VERSION(0,0,1,0);
 app.pEngineName="ClubEngine"; app.engineVersion=VK_MAKE_API_VERSION(0,0,1,0); app.apiVersion=VK_API_VERSION_1_3;
 VkInstanceCreateInfo ci{VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
 ci.pApplicationInfo=&app; ci.enabledExtensionCount=count; ci.ppEnabledExtensionNames=exts;
 checkVk(vkCreateInstance(&ci,nullptr,&instance_),"vkCreateInstance");
}
void Renderer::createSurface(SDL_Window* w) {
 if(!SDL_Vulkan_CreateSurface(w,instance_,nullptr,&surface_)) throw std::runtime_error(std::string("SDL_Vulkan_CreateSurface: ")+SDL_GetError());
}
void Renderer::selectPhysicalDevice() {
 uint32_t n=0; checkVk(vkEnumeratePhysicalDevices(instance_,&n,nullptr),"enumerate GPUs");
 if(!n) throw std::runtime_error("No Vulkan GPU found");
 std::vector<VkPhysicalDevice> ds(n); checkVk(vkEnumeratePhysicalDevices(instance_,&n,ds.data()),"enumerate GPUs");
 for(auto d:ds) {
  VkPhysicalDeviceProperties p{}; vkGetPhysicalDeviceProperties(d,&p);
  if(p.apiVersion<VK_API_VERSION_1_3) continue;
  uint32_t fc=0; vkGetPhysicalDeviceQueueFamilyProperties(d,&fc,nullptr);
  std::vector<VkQueueFamilyProperties> fs(fc); vkGetPhysicalDeviceQueueFamilyProperties(d,&fc,fs.data());
  bool gf=false,pf=false; uint32_t gi=0,pi=0;
  for(uint32_t i=0;i<fc;++i) {
   if(fs[i].queueFlags&VK_QUEUE_GRAPHICS_BIT) { gf=true; gi=i; }
   VkBool32 supported=VK_FALSE; checkVk(vkGetPhysicalDeviceSurfaceSupportKHR(d,i,surface_,&supported),"surface support");
   if(supported) { pf=true; pi=i; }
  }
  if(gf&&pf) { physicalDevice_=d; graphicsFamily_=gi; presentFamily_=pi; break; }
 }
 if(!physicalDevice_) throw std::runtime_error("No Vulkan 1.3 GPU with graphics and present queues");
}
void Renderer::createDevice() {
 float priority=1.0f; uint32_t ids[]={graphicsFamily_,presentFamily_};
 std::vector<VkDeviceQueueCreateInfo> qs;
 for(uint32_t i=0;i<2;++i) {
  if(i==1&&ids[1]==ids[0]) continue;
  VkDeviceQueueCreateInfo q{VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
  q.queueFamilyIndex=ids[i]; q.queueCount=1; q.pQueuePriorities=&priority; qs.push_back(q);
 }
 const char* ext[]={VK_KHR_SWAPCHAIN_EXTENSION_NAME};
 VkPhysicalDeviceFeatures features{};
 VkDeviceCreateInfo ci{VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
 ci.queueCreateInfoCount=static_cast<uint32_t>(qs.size()); ci.pQueueCreateInfos=qs.data();
 ci.pEnabledFeatures=&features; ci.enabledExtensionCount=1; ci.ppEnabledExtensionNames=ext;
 checkVk(vkCreateDevice(physicalDevice_,&ci,nullptr,&device_),"vkCreateDevice");
 vkGetDeviceQueue(device_,graphicsFamily_,0,&graphicsQueue_);
 vkGetDeviceQueue(device_,presentFamily_,0,&presentQueue_);
}
void Renderer::beginFrame() {}
void Renderer::endFrame() {}
void Renderer::waitIdle() { if(device_) checkVk(vkDeviceWaitIdle(device_),"vkDeviceWaitIdle"); }
}
