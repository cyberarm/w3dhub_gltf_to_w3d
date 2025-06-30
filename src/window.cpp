//
// Created by cyberarm on 2025-06-30.
//

#include "window.h"

Window::Window() {
    if (load_model(GLTF_FILENAME)) {
        for (const auto &mesh: m_model.meshes) {
            std::cout << "    Mesh: " << mesh.name << std::endl;
        }
        for (auto node: m_model.nodes) {
            std::cout << "    Node: " << node.name << std::endl;
            if (!node.translation.empty())
                std::cout << "       Position [OpenGL/Vulkan]: " << "(" << node.translation.at(0) << ", "
                          << node.translation.at(1) << ", " << node.translation.at(2) << ")" << std::endl;
            if (!node.scale.empty())
                std::cout << "       Scale [OpenGL/Vulkan]: " << "(" << node.scale.at(0) << ", "
                          << node.scale.at(1) << ", " << node.scale.at(2) << ")" << std::endl;
            if (node.translation.empty())
                std::cout << "       Position [OpenGL/Vulkan]: " << "origin (0, 0, 0)" << std::endl;
            if (node.scale.empty())
                std::cout << "       scale [OpenGL/Vulkan]: " << "none (1, 1, 1)" << std::endl;
        }

        export_w3d_hierarchy_model( W3D_FILENAME, true);
    }
}

Window::~Window() {
    // Cleanup
    // [If using SDL_MAIN_USE_CALLBACKS: all code below would likely be your SDL_AppQuit() function]
    SDL_WaitForGPUIdle(m_gpu_device);
    ImGui_ImplSDL3_Shutdown();
    ImGui_ImplSDLGPU3_Shutdown();
    ImGui::DestroyContext();

    SDL_ReleaseWindowFromGPUDevice(m_gpu_device, m_window);
    SDL_DestroyGPUDevice(m_gpu_device);
    SDL_DestroyWindow(m_window);
    SDL_Quit();
}

bool Window::init_sdl() {
    // Setup SDL
    // [If using SDL_MAIN_USE_CALLBACKS: all code below until the main loop starts would likely be your SDL_AppInit() function]
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD)) {
        printf("Error: SDL_Init(): %s\n", SDL_GetError());
        return false;
    }

    // Create SDL m_window graphics context
    m_main_scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
    SDL_WindowFlags window_flags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN | SDL_WINDOW_HIGH_PIXEL_DENSITY;
    m_window = SDL_CreateWindow(APP_TITLE, (int) (1280 * m_main_scale), (int) (720 * m_main_scale), window_flags);
    if (m_window == nullptr) {
        printf("Error: SDL_CreateWindow(): %s\n", SDL_GetError());
        return false;
    }
    SDL_SetWindowPosition(m_window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    SDL_ShowWindow(m_window);

    // Create GPU Device
    m_gpu_device = SDL_CreateGPUDevice(
            SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_DXIL | SDL_GPU_SHADERFORMAT_METALLIB, true, nullptr);
    if (m_gpu_device == nullptr) {
        printf("Error: SDL_CreateGPUDevice(): %s\n", SDL_GetError());
        return false;
    }

    // Claim m_window for GPU Device
    if (!SDL_ClaimWindowForGPUDevice(m_gpu_device, m_window)) {
        printf("Error: SDL_ClaimWindowForGPUDevice(): %s\n", SDL_GetError());
        return false;
    }
    SDL_SetGPUSwapchainParameters(m_gpu_device, m_window, SDL_GPU_SWAPCHAINCOMPOSITION_SDR,
                                  SDL_GPU_PRESENTMODE_MAILBOX);

    return true;
}

bool Window::init_imgui() {
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    m_io = &ImGui::GetIO();
    (void) m_io;
    m_io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    m_io->ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup scaling
    m_style = &ImGui::GetStyle();
    m_style->ScaleAllSizes(
            m_main_scale);        // Bake a fixed style scale. (until we have a solution for dynamic style scaling, changing this requires resetting Style + calling this again)
    m_style->FontScaleDpi = m_main_scale;        // Set initial font scale. (using io.ConfigDpiScaleFonts=true makes this unnecessary. We leave both here for documentation purpose)

    // Setup Platform/Renderer backends
    ImGui_ImplSDL3_InitForSDLGPU(m_window);
    ImGui_ImplSDLGPU3_InitInfo init_info = {};
    init_info.Device = m_gpu_device;
    init_info.ColorTargetFormat = SDL_GetGPUSwapchainTextureFormat(m_gpu_device, m_window);
    init_info.MSAASamples = SDL_GPU_SAMPLECOUNT_1;
    ImGui_ImplSDLGPU3_Init(&init_info);

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return a nullptr. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality font rendering.
    // - read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    //style.FontSizeBase = 20.0f;
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf");
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf");
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf");
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf");
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf");
    //IM_ASSERT(font != nullptr);
    // FIXME: Dynamically choose font based on platform and whether the font path can be found
    m_io->Fonts->AddFontFromFileTTF("C:/Windows/Fonts/Arial.ttf");

    return true;
};

int Window::init() {
    init_sdl();
    init_imgui();

    return true;
}

void Window::run() {
    // Main loop
    while (!m_done) {
        update();
    }
}

void Window::update() {
    // Poll and handle events (inputs, m_window resize, etc.)
    // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
    // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
    // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
    // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
    // [If using SDL_MAIN_USE_CALLBACKS: call ImGui_ImplSDL3_ProcessEvent() from your SDL_AppEvent() function]
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        ImGui_ImplSDL3_ProcessEvent(&event);
        if (event.type == SDL_EVENT_QUIT)
            m_done = true;
        if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED && event.window.windowID == SDL_GetWindowID(m_window))
            m_done = true;
    }

    SDL_Delay(16);

    // [If using SDL_MAIN_USE_CALLBACKS: all code below would likely be your SDL_AppIterate() function]
    if (SDL_GetWindowFlags(m_window) & SDL_WINDOW_MINIMIZED) {
        return;
    }

    draw();
}

void Window::draw() {
    // Start the Dear ImGui frame
    ImGui_ImplSDLGPU3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    draw_main_window();

    // Rendering
    ImGui::Render();
    ImDrawData *draw_data = ImGui::GetDrawData();
    const bool is_minimized = (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f);

    SDL_GPUCommandBuffer *command_buffer = SDL_AcquireGPUCommandBuffer(m_gpu_device); // Acquire a GPU command buffer

    SDL_GPUTexture *swapchain_texture;
    SDL_AcquireGPUSwapchainTexture(command_buffer, m_window, &swapchain_texture, nullptr,
                                   nullptr); // Acquire a swapchain texture

    if (swapchain_texture != nullptr && !is_minimized) {
        // This is mandatory: call ImGui_ImplSDLGPU3_PrepareDrawData() to upload the vertex/index buffer!
        ImGui_ImplSDLGPU3_PrepareDrawData(draw_data, command_buffer);

        // Setup and start a render pass
        SDL_GPUColorTargetInfo target_info = {};
        target_info.texture = swapchain_texture;
        target_info.clear_color = SDL_FColor{m_clear_color.x, m_clear_color.y, m_clear_color.z, m_clear_color.w};
        target_info.load_op = SDL_GPU_LOADOP_CLEAR;
        target_info.store_op = SDL_GPU_STOREOP_STORE;
        target_info.mip_level = 0;
        target_info.layer_or_depth_plane = 0;
        target_info.cycle = false;
        SDL_GPURenderPass *render_pass = SDL_BeginGPURenderPass(command_buffer, &target_info, 1, nullptr);

        // Render ImGui
        ImGui_ImplSDLGPU3_RenderDrawData(draw_data, command_buffer, render_pass);

        SDL_EndGPURenderPass(render_pass);
    }

    // Submit the command buffer
    SDL_SubmitGPUCommandBuffer(command_buffer);
}

void Window::draw_main_window() {
    static float f = 0.0f;
    static int counter = 0;

    ImGui::SetNextWindowSize(m_io->DisplaySize);
    ImGui::SetNextWindowPos({0, 0});
    ImGui::Begin("##", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
                                ImGuiWindowFlags_NoSavedSettings);
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / m_io.Framerate, m_io.Framerate);
    if (ImGui::BeginChild("GLTF", {ImGui::GetContentRegionAvail().x / 2, ImGui::GetContentRegionAvail().y},
                          ImGuiWindowFlags_NoDecoration)) {
        ImGui::Text("GLTF / GLB");
        ImGui::Text("DRAG and DROP .gltf or .glb file here");
        ImGui::Text("NOTE: Embedded images will not be used.");
        ImGui::Spacing();
        ImGui::Text("\nTODO: Show TreeView of GLTF contents here once loaded.");

        if (!m_model.nodes.empty()) {
            size_t i = 0;
            if (ImGui::TreeNodeEx("Model", ImGuiTreeNodeFlags_DefaultOpen)) {
                // SCENES
                if (ImGui::TreeNode("tinygltf_scenes", "Scenes - %d", m_model.scenes.size())) {
                    for (const auto &scene: m_model.scenes) {
                        if (ImGui::TreeNode(scene.name.c_str())) {
                            ImGui::TreePop();
                        }
                    }
                    ImGui::TreePop();
                }

                // NODES
                if (ImGui::TreeNode("tinygltf_nodes", "Nodes/Objects - %d", m_model.nodes.size())) {
                    i = 0;
                    for (const auto &node: m_model.nodes) {
                        std::string node_id = std::format("gltf_node:{}:{}", node.name, i);
                        if (ImGui::TreeNode(node_id.c_str(), "%d. %s", i, node.name.c_str())) {
                            ImGui::TreePop();
                        }

                        i++;
                    }
                    ImGui::TreePop();
                }

                // MESHES
                if (ImGui::TreeNode("tinygltf_meshes", "Meshes - %d", m_model.meshes.size())) {
                    for (const auto &mesh: m_model.meshes) {
                        if (ImGui::TreeNode(mesh.name.c_str())) {
                            ImGui::TreePop();
                        }
                    }
                    ImGui::TreePop();
                }

                // IMAGES
                if (ImGui::TreeNode("tinygltf_images", "Images - %d", m_model.images.size())) {
                    for (const auto &image: m_model.images) {
                        if (ImGui::TreeNode(image.name.c_str())) {
                            ImGui::TreePop();
                        }
                    }
                    ImGui::TreePop();
                }

                // TEXTURES
                if (ImGui::TreeNode("tinygltf_textures", "Textures - %d", m_model.textures.size())) {
                    for (const auto &texture: m_model.textures) {
                        if (ImGui::TreeNode(texture.name.c_str())) {
                            ImGui::TreePop();
                        }
                    }
                    ImGui::TreePop();
                }

                // MATERIALS
                if (ImGui::TreeNode("tinygltf_materials", "Materials - %d", m_model.materials.size())) {
                    for (const auto &material: m_model.materials) {
                        if (ImGui::TreeNode(material.name.c_str())) {
                            ImGui::Text("Double Sided: %s", material.doubleSided ? "True" : "False");

                            ImGui::TreePop();
                        }
                    }
                    ImGui::TreePop();
                }

                // CAMERAS
                if (ImGui::TreeNode("tinygltf_cameras", "Cameras - %d", m_model.cameras.size())) {
                    for (const auto &camera: m_model.cameras) {
                        if (ImGui::TreeNode(camera.name.c_str())) {
                            ImGui::TreePop();
                        }
                    }
                    ImGui::TreePop();
                }

                // LIGHTS
                if (ImGui::TreeNode("tinygltf_lights", "Lights - %d", m_model.lights.size())) {
                    for (const auto &light: m_model.lights) {
                        if (ImGui::TreeNode(light.name.c_str())) {
                            ImGui::TextColored(
                                    {
                                            static_cast<float>(light.color.at(0)),
                                            static_cast<float>(light.color.at(1)),
                                            static_cast<float>(light.color.at(2)), 1}, "COLOR");
                            ImGui::TreePop();
                        }
                    }
                    ImGui::TreePop();
                }
                ImGui::TreePop();
            }
        }
    }
    ImGui::EndChild();

    ImGui::SameLine();
    // NOTE: We use the FULL `ImGui::GetContentRegionAvail` X space here since we used HALF of it on the previous m_window
    if (ImGui::BeginChild("w3d_export_window", {ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y},
                          ImGuiWindowFlags_NoDecoration)) {
        ImGui::Text("W3D Export Options");
        static char buf[16] = "";
        ImGui::InputTextWithHint("##", "Leave blank if unsure", buf, 16);
        ImGui::SameLine();
        ImGui::Text("Container Name");
        const char *listbox_items[] = {"Hierarchy Model", "Mesh"};
        static int listbox_item_current = 0;
        ImGui::Combo("W3D Type", &listbox_item_current, listbox_items, 2);
        static bool opt_terrain = false;
        ImGui::Checkbox("Optimize for Terrain", &opt_terrain);

        // Export Button
        // NOTE: We manually move imgui's 'cursor' so it MUST be the last element of the 'm_window' created
        //       to ensure proper auto element placement.
        float button_width = std::max(ImGui::CalcTextSize("Export").x + m_style->FramePadding.x * 2.f, 128.f);
        float button_height = ImGui::CalcTextSize("Export").y + m_style->FramePadding.y * 2.f;
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + ImGui::GetContentRegionAvail().y - button_height);
        ImGui::ProgressBar(0.25f, {ImGui::GetContentRegionAvail().x - (button_width + m_style->ItemSpacing.x), 0});
        ImGui::SameLine();
        ImGui::Button("Export", {button_width, 0});
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
            ImGui::SetTooltip("File will be saved to: ./{CONTAINER_NAME}.w3d");
    }
    ImGui::EndChild();

    ImGui::End();
}

void Window::draw_intro_window() {}

bool Window::export_w3d_hierarchy_model(const std::string &filename,
                                        bool optimize_for_terrain = false) {
    SDL_IOStream *stream = SDL_IOFromFile(filename.c_str(), "wb");

    auto m_writer = ChunkSaveClass(stream);

    W3dHierarchyModel(&m_model, m_writer, optimize_for_terrain);

    SDL_CloseIO(stream);
    return true;
}

bool Window::load_model(const std::string &filename) {
    tinygltf::TinyGLTF loader;
    std::string err;
    std::string warn;

    bool result = false;
    if (filename.ends_with(".glb"))
        result = loader.LoadBinaryFromFile(&m_model, &err, &warn, filename);
    if (filename.ends_with(".gltf"))
        result = loader.LoadASCIIFromFile(&m_model, &err, &warn, filename);
    if (!warn.empty()) {
        std::cout << "WARN: " << warn << std::endl;
    }

    if (!err.empty()) {
        std::cout << "ERR: " << err << std::endl;
    }

    if (!result)
        std::cout << "Failed to load glTF: " << filename << std::endl;
    else
        std::cout << "Loaded glTF: " << filename << std::endl;

    return result;
}