#include "gui-manager.h"

void GUIManager::Initialize(
    GLFWwindow* window, VkInstance instance,
    VkPhysicalDevice physicalDevice, VkDevice device,
    uint32_t queueFamily, VkQueue queue,
    VkDescriptorPool descriptorPool, uint32_t framesInFlight,
    VkFormat colorFormat, VkFormat depthFormat)
{
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForVulkan(window, true);
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.UseDynamicRendering = true;
    init_info.Instance = instance;
    init_info.PhysicalDevice = physicalDevice;
    init_info.Device = device;
    init_info.QueueFamily = queueFamily;
    init_info.Queue = queue;
    init_info.PipelineCache = VK_NULL_HANDLE;
    init_info.DescriptorPool = descriptorPool;
    init_info.MinImageCount = framesInFlight;
    init_info.ImageCount = framesInFlight;
    init_info.Allocator = nullptr;
    init_info.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    init_info.PipelineInfoMain.PipelineRenderingCreateInfo = { .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR };
    init_info.PipelineInfoMain.PipelineRenderingCreateInfo.colorAttachmentCount = 1; // TODO: deferred rendering hopefully doesnt need change
    init_info.PipelineInfoMain.PipelineRenderingCreateInfo.pColorAttachmentFormats = &colorFormat;
    init_info.PipelineInfoMain.PipelineRenderingCreateInfo.depthAttachmentFormat = depthFormat;
    init_info.CheckVkResultFn = CheckVKResult;
    ImGui_ImplVulkan_Init(&init_info);
}

void GUIManager::MainLoop() {
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::ShowDemoWindow();

    //ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.
    //ImGui::Text("This is some useful text.");
    //ImGui::End();

    ImGui::Render();

    //ImDrawData* draw_data = ImGui::GetDrawData();
    //const bool is_minimized = (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f);
    //if (!is_minimized)
    //{
    //    wd->ClearValue.color.float32[0] = clear_color.x * clear_color.w;
    //    wd->ClearValue.color.float32[1] = clear_color.y * clear_color.w;
    //    wd->ClearValue.color.float32[2] = clear_color.z * clear_color.w;
    //    wd->ClearValue.color.float32[3] = clear_color.w;
    //    FrameRender(wd, draw_data);
    //    FramePresent(wd);
    //}
}

void GUIManager::Shutdown(VulkanReferences& ref) {
    ref.device.waitIdle();
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}