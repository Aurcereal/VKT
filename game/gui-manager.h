#pragma once

#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_vulkan.h"

#include "defines.h"

#include <iostream>
#include <vector>
#include <functional>

// TODO: move to cpp
static void CheckVKResult(VkResult err)
{
    if (err == 0)
        return;
    fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
    if (err < 0)
        abort();
}

class GUIManager {
public:
    static void Initialize(GLFWwindow*, VkInstance, VkPhysicalDevice, VkDevice, uint32_t qFamily, VkQueue, VkDescriptorPool, uint32_t framesInFlight, VkFormat colorFormat, VkFormat depthFormat);
    static void MainLoop();
    static void Shutdown(VulkanReferences&);

    static void RegisterUIFunction(const std::function<void()>&);
private:
    static std::vector<std::function<void()>> uiFunctions;
};