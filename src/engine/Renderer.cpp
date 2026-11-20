#include "Renderer.hpp"
#include <SDL3/SDL_vulkan.h>
#include <algorithm>
#include <array>
#include <limits>
#include <stdexcept>
#include <string>
namespace club {
namespace {
void check(VkResult r,const char* op){if(r!=VK_SUCCESS)throw std::runtime_error(std::string(op)+" failed ("+std::to_string(static_cast<int>(r))+")");}
struct Support { VkSurfaceCapabilitiesKHR caps{}; std::vector<VkSurfaceFormatKHR> formats; std::vector<VkPresentModeKHR> modes; };
}
Renderer::Renderer(const Window& w){
 createInstance(); createSurface(w.native()); selectPhysicalDevice(); createDevice();
 createSwapchain(); createRenderPass(); createFramebuffers(); createCommandPool(); createCommandBuffers(); createSync();
}
Renderer::~Renderer(){
 if(device_) vkDeviceWaitIdle(device_);
 for(uint32_t i=0;i<FramesInFlight;++i){if(imageAvailable_[i])vkDestroySemaphore(device_,imageAvailable_[i],nullptr);if(renderFinished_[i])vkDestroySemaphore(device_,renderFinished_[i],nullptr);if(inFlight_[i])vkDestroyFence(device_,inFlight_[i],nullptr);}
 if(commandPool_)vkDestroyCommandPool(device_,commandPool_,nullptr);
 cleanupSwapchain();
 if(device_)vkDestroyDevice(device_,nullptr);
 if(surface_)vkDestroySurfaceKHR(instance_,surface_,nullptr);
 if(instance_)vkDestroyInstance(instance_,nullptr);
}
void Renderer::createInstance(){
 unsigned count=0; const char* const* exts=SDL_Vulkan_GetInstanceExtensions(&count);
 if(!exts)throw std::runtime_error(SDL_GetError());
 VkApplicationInfo app{VK_STRUCTURE_TYPE_APPLICATION_INFO}; app.pApplicationName="ClubEngine";app.applicationVersion=VK_MAKE_API_VERSION(0,0,1,0);app.pEngineName="ClubEngine";app.engineVersion=VK_MAKE_API_VERSION(0,0,1,0);app.apiVersion=VK_API_VERSION_1_3;
 VkInstanceCreateInfo ci{VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};ci.pApplicationInfo=&app;ci.enabledExtensionCount=count;ci.ppEnabledExtensionNames=exts;check(vkCreateInstance(&ci,nullptr,&instance_),"vkCreateInstance");
}
void Renderer::createSurface(SDL_Window* w){if(!SDL_Vulkan_CreateSurface(w,instance_,nullptr,&surface_))throw std::runtime_error(std::string("Surface: ")+SDL_GetError());}
void Renderer::selectPhysicalDevice(){
 uint32_t n=0;check(vkEnumeratePhysicalDevices(instance_,&n,nullptr),"enumerate GPUs");if(!n)throw std::runtime_error("No Vulkan GPU found");std::vector<VkPhysicalDevice> ds(n);check(vkEnumeratePhysicalDevices(instance_,&n,ds.data()),"enumerate GPUs");
 for(auto d:ds){
  VkPhysicalDeviceProperties p{};vkGetPhysicalDeviceProperties(d,&p);if(p.apiVersion<VK_API_VERSION_1_3)continue;
  uint32_t fc=0;vkGetPhysicalDeviceQueueFamilyProperties(d,&fc,nullptr);std::vector<VkQueueFamilyProperties> fs(fc);vkGetPhysicalDeviceQueueFamilyProperties(d,&fc,fs.data());
  bool g=false,pr=false;uint32_t gi=0,pi=0;
  for(uint32_t i=0;i<fc;++i){if(fs[i].queueFlags&VK_QUEUE_GRAPHICS_BIT){g=true;gi=i;}VkBool32 ok=VK_FALSE;check(vkGetPhysicalDeviceSurfaceSupportKHR(d,i,surface_,&ok),"surface support");if(ok){pr=true;pi=i;}}
  if(!g||!pr)continue;
  uint32_t ec=0;vkEnumerateDeviceExtensionProperties(d,nullptr,&ec,nullptr);std::vector<VkExtensionProperties> es(ec);vkEnumerateDeviceExtensionProperties(d,nullptr,&ec,es.data());
  bool swap=std::any_of(es.begin(),es.end(),[](const auto& e){return std::string(e.extensionName)==VK_KHR_SWAPCHAIN_EXTENSION_NAME;});if(!swap)continue;
  uint32_t fc2=0,mc=0;vkGetPhysicalDeviceSurfaceFormatsKHR(d,surface_,&fc2,nullptr);vkGetPhysicalDeviceSurfacePresentModesKHR(d,surface_,&mc,nullptr);if(!fc2||!mc)continue;
  physicalDevice_=d;graphicsFamily_=gi;presentFamily_=pi;break;
 }
 if(!physicalDevice_)throw std::runtime_error("No Vulkan 1.3 GPU supports swapchain presentation");
}
void Renderer::createDevice(){
 float priority=1.0f;std::array<uint32_t,2> ids{graphicsFamily_,presentFamily_};std::vector<VkDeviceQueueCreateInfo> qs;
 for(size_t i=0;i<ids.size();++i){if(i&&ids[i]==ids[0])continue;VkDeviceQueueCreateInfo q{VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};q.queueFamilyIndex=ids[i];q.queueCount=1;q.pQueuePriorities=&priority;qs.push_back(q);}
 const char* ext[]={VK_KHR_SWAPCHAIN_EXTENSION_NAME};VkPhysicalDeviceFeatures f{};VkDeviceCreateInfo ci{VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};ci.queueCreateInfoCount=static_cast<uint32_t>(qs.size());ci.pQueueCreateInfos=qs.data();ci.pEnabledFeatures=&f;ci.enabledExtensionCount=1;ci.ppEnabledExtensionNames=ext;check(vkCreateDevice(physicalDevice_,&ci,nullptr,&device_),"vkCreateDevice");vkGetDeviceQueue(device_,graphicsFamily_,0,&graphicsQueue_);vkGetDeviceQueue(device_,presentFamily_,0,&presentQueue_);
}
void Renderer::createSwapchain(){
 Support s;check(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice_,surface_,&s.caps),"surface capabilities");uint32_t n=0;vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice_,surface_,&n,nullptr);s.formats.resize(n);vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice_,surface_,&n,s.formats.data());vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice_,surface_,&n,nullptr);s.modes.resize(n);vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice_,surface_,&n,s.modes.data());
 VkSurfaceFormatKHR fmt=s.formats.front();for(auto f:s.formats)if(f.format==VK_FORMAT_B8G8R8A8_SRGB&&f.colorSpace==VK_COLOR_SPACE_SRGB_NONLINEAR_KHR){fmt=f;break;}swapFormat_=fmt.format;
 if(s.caps.currentExtent.width!=UINT32_MAX)extent_=s.caps.currentExtent;else{int w=0,h=0;SDL_GetWindowSizeInPixels(SDL_GetWindowFromID(1),&w,&h);extent_.width=std::clamp(static_cast<uint32_t>(std::max(w,1)),s.caps.minImageExtent.width,s.caps.maxImageExtent.width);extent_.height=std::clamp(static_cast<uint32_t>(std::max(h,1)),s.caps.minImageExtent.height,s.caps.maxImageExtent.height);}
 uint32_t count=s.caps.minImageCount+1;if(s.caps.maxImageCount&&count>s.caps.maxImageCount)count=s.caps.maxImageCount;
 VkSwapchainCreateInfoKHR ci{VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};ci.surface=surface_;ci.minImageCount=count;ci.imageFormat=fmt.format;ci.imageColorSpace=fmt.colorSpace;ci.imageExtent=extent_;ci.imageArrayLayers=1;ci.imageUsage=VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
 uint32_t qids[]={graphicsFamily_,presentFamily_};if(graphicsFamily_!=presentFamily_){ci.imageSharingMode=VK_SHARING_MODE_CONCURRENT;ci.queueFamilyIndexCount=2;ci.pQueueFamilyIndices=qids;}else ci.imageSharingMode=VK_SHARING_MODE_EXCLUSIVE;
 ci.preTransform=s.caps.currentTransform;ci.compositeAlpha=VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;ci.presentMode=VK_PRESENT_MODE_FIFO_KHR;ci.clipped=VK_TRUE;check(vkCreateSwapchainKHR(device_,&ci,nullptr,&swapchain_),"vkCreateSwapchainKHR");
 vkGetSwapchainImagesKHR(device_,swapchain_,&count,nullptr);images_.resize(count);vkGetSwapchainImagesKHR(device_,swapchain_,&count,images_.data());views_.resize(count);
 for(size_t i=0;i<images_.size();++i){VkImageViewCreateInfo vi{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};vi.image=images_[i];vi.viewType=VK_IMAGE_VIEW_TYPE_2D;vi.format=swapFormat_;vi.subresourceRange={VK_IMAGE_ASPECT_COLOR_BIT,0,1,0,1};check(vkCreateImageView(device_,&vi,nullptr,&views_[i]),"vkCreateImageView");}
}
void Renderer::createRenderPass(){VkAttachmentDescription a{};a.format=swapFormat_;a.samples=VK_SAMPLE_COUNT_1_BIT;a.loadOp=VK_ATTACHMENT_LOAD_OP_CLEAR;a.storeOp=VK_ATTACHMENT_STORE_OP_STORE;a.stencilLoadOp=VK_ATTACHMENT_LOAD_OP_DONT_CARE;a.stencilStoreOp=VK_ATTACHMENT_STORE_OP_DONT_CARE;a.initialLayout=VK_IMAGE_LAYOUT_UNDEFINED;a.finalLayout=VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;VkAttachmentReference ref{0,VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};VkSubpassDescription sub{};sub.pipelineBindPoint=VK_PIPELINE_BIND_POINT_GRAPHICS;sub.colorAttachmentCount=1;sub.pColorAttachments=&ref;VkSubpassDependency dep{};dep.srcSubpass=VK_SUBPASS_EXTERNAL;dep.dstSubpass=0;dep.srcStageMask=VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;dep.dstStageMask=VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;dep.dstAccessMask=VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;VkRenderPassCreateInfo ci{VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};ci.attachmentCount=1;ci.pAttachments=&a;ci.subpassCount=1;ci.pSubpasses=&sub;ci.dependencyCount=1;ci.pDependencies=&dep;check(vkCreateRenderPass(device_,&ci,nullptr,&renderPass_),"vkCreateRenderPass");}
void Renderer::createFramebuffers(){framebuffers_.resize(views_.size());for(size_t i=0;i<views_.size();++i){VkFramebufferCreateInfo ci{VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};ci.renderPass=renderPass_;ci.attachmentCount=1;ci.pAttachments=&views_[i];ci.width=extent_.width;ci.height=extent_.height;ci.layers=1;check(vkCreateFramebuffer(device_,&ci,nullptr,&framebuffers_[i]),"vkCreateFramebuffer");}}
void Renderer::createCommandPool(){VkCommandPoolCreateInfo ci{VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};ci.flags=VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;ci.queueFamilyIndex=graphicsFamily_;check(vkCreateCommandPool(device_,&ci,nullptr,&commandPool_),"vkCreateCommandPool");}
void Renderer::createCommandBuffers(){commands_.resize(FramesInFlight);VkCommandBufferAllocateInfo ai{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};ai.commandPool=commandPool_;ai.level=VK_COMMAND_BUFFER_LEVEL_PRIMARY;ai.commandBufferCount=FramesInFlight;check(vkAllocateCommandBuffers(device_,&ai,commands_.data()),"vkAllocateCommandBuffers");}
void Renderer::createSync(){for(uint32_t i=0;i<FramesInFlight;++i){VkSemaphoreCreateInfo si{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};VkFenceCreateInfo fi{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};fi.flags=VK_FENCE_CREATE_SIGNALED_BIT;check(vkCreateSemaphore(device_,&si,nullptr,&imageAvailable_[i]),"create semaphore");check(vkCreateSemaphore(device_,&si,nullptr,&renderFinished_[i]),"create semaphore");check(vkCreateFence(device_,&fi,nullptr,&inFlight_[i]),"create fence");}}
void Renderer::beginFrame(){
 check(vkWaitForFences(device_,1,&inFlight_[frame_],VK_TRUE,UINT64_MAX),"wait frame fence");
 VkResult r=vkAcquireNextImageKHR(device_,swapchain_,UINT64_MAX,imageAvailable_[frame_],VK_NULL_HANDLE,&imageIndex_);if(r==VK_ERROR_OUT_OF_DATE_KHR){recreateSwapchain();return;}if(r!=VK_SUCCESS&&r!=VK_SUBOPTIMAL_KHR)check(r,"acquire swapchain image");
 check(vkResetFences(device_,1,&inFlight_[frame_]),"reset fence");check(vkResetCommandBuffer(commands_[frame_],0),"reset command buffer");
 VkCommandBufferBeginInfo bi{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};check(vkBeginCommandBuffer(commands_[frame_],&bi),"begin command buffer");
 VkClearValue clear{};clear.color={{0.035f,0.025f,0.065f,1.0f}};VkRenderPassBeginInfo rp{VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};rp.renderPass=renderPass_;rp.framebuffer=framebuffers_[imageIndex_];rp.renderArea={{0,0},extent_};rp.clearValueCount=1;rp.pClearValues=&clear;vkCmdBeginRenderPass(commands_[frame_],&rp,VK_SUBPASS_CONTENTS_INLINE);frameStarted_=true;
}
void Renderer::endFrame(){if(!frameStarted_)return;vkCmdEndRenderPass(commands_[frame_]);check(vkEndCommandBuffer(commands_[frame_]),"end command buffer");VkPipelineStageFlags wait=VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;VkSubmitInfo si{VK_STRUCTURE_TYPE_SUBMIT_INFO};si.waitSemaphoreCount=1;si.pWaitSemaphores=&imageAvailable_[frame_];si.pWaitDstStageMask=&wait;si.commandBufferCount=1;si.pCommandBuffers=&commands_[frame_];si.signalSemaphoreCount=1;si.pSignalSemaphores=&renderFinished_[frame_];check(vkQueueSubmit(graphicsQueue_,1,&si,inFlight_[frame_]),"queue submit");VkPresentInfoKHR pi{VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};pi.waitSemaphoreCount=1;pi.pWaitSemaphores=&renderFinished_[frame_];pi.swapchainCount=1;pi.pSwapchains=&swapchain_;pi.pImageIndices=&imageIndex_;VkResult r=vkQueuePresentKHR(presentQueue_,&pi);if(r==VK_ERROR_OUT_OF_DATE_KHR||r==VK_SUBOPTIMAL_KHR)recreateSwapchain();else check(r,"queue present");frame_=(frame_+1)%FramesInFlight;frameStarted_=false;}
void Renderer::cleanupSwapchain(){for(auto f:framebuffers_)vkDestroyFramebuffer(device_,f,nullptr);framebuffers_.clear();if(renderPass_)vkDestroyRenderPass(device_,renderPass_,nullptr);renderPass_=VK_NULL_HANDLE;for(auto v:views_)vkDestroyImageView(device_,v,nullptr);views_.clear();if(swapchain_)vkDestroySwapchainKHR(device_,swapchain_,nullptr);swapchain_=VK_NULL_HANDLE;images_.clear();}
void Renderer::recreateSwapchain(){int w=0,h=0;SDL_Window* win=SDL_GetWindowFromID(1);while((w==0||h==0)&&win){SDL_GetWindowSizeInPixels(win,&w,&h);SDL_Delay(16);}if(!device_)return;vkDeviceWaitIdle(device_);cleanupSwapchain();createSwapchain();createRenderPass();createFramebuffers();}
void Renderer::waitIdle(){if(device_)check(vkDeviceWaitIdle(device_),"device wait idle");}
}
