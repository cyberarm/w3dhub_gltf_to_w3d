#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE

#include <iostream>
#include <string>

#include "w3d_file.h"
#include "chunkio.h"
#include "w3d_hierarchy_model.h"

#include "tiny_gltf.h"

#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlgpu3.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#define APP_TITLE "GLTF to W3D Converter | ALPHA QUALITY SOFTWARE USE AT YOUR OWN RISK!"
#define GLTF_FILENAME "D:/W3DHub/games/tiberian-sun-reborn/LevelEdit/TS_Level/ts_level.glb"
#define W3D_FILENAME "D:/W3DHub/games/tiberian-sun-reborn/LevelEdit/TS_Level/ts_level.w3d"

bool exportW3DHierarchyModel(tinygltf::Model &model, const std::string &filename, bool optimize_for_terrain = false) {
    SDL_IOStream *stream = SDL_IOFromFile(filename.c_str(), "wb");

    auto m_writer = ChunkSaveClass(stream);

    W3dHierarchyModel(model, m_writer, true);

    SDL_CloseIO(stream);
    return true;
}

bool loadModel(tinygltf::Model &model, const std::string &filename) {
    tinygltf::TinyGLTF loader;
    std::string err;
    std::string warn;

    bool result = false;
    if (filename.ends_with(".glb"))
        result = loader.LoadBinaryFromFile(&model, &err, &warn, filename);
    if (filename.ends_with(".gltf"))
        result = loader.LoadASCIIFromFile(&model, &err, &warn, filename);
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

int main(int, char **) {
    tinygltf::Model model;
    if (loadModel(model, GLTF_FILENAME)) {
        for (const auto &mesh: model.meshes) {
            std::cout << "    Mesh: " << mesh.name << std::endl;
        }
        for (auto node: model.nodes) {
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

        exportW3DHierarchyModel(model, W3D_FILENAME, true);
    }

    // Setup SDL
    // [If using SDL_MAIN_USE_CALLBACKS: all code below until the main loop starts would likely be your SDL_AppInit() function]
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD)) {
        printf("Error: SDL_Init(): %s\n", SDL_GetError());
        return -1;
    }

    // Create SDL window graphics context
    float main_scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
    SDL_WindowFlags window_flags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN | SDL_WINDOW_HIGH_PIXEL_DENSITY;
    SDL_Window *window = SDL_CreateWindow(APP_TITLE, (int) (1280 * main_scale), (int) (720 * main_scale), window_flags);
    if (window == nullptr) {
        printf("Error: SDL_CreateWindow(): %s\n", SDL_GetError());
        return -1;
    }
    SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    SDL_ShowWindow(window);

    // Create GPU Device
    SDL_GPUDevice *gpu_device = SDL_CreateGPUDevice(
            SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_DXIL | SDL_GPU_SHADERFORMAT_METALLIB, true, nullptr);
    if (gpu_device == nullptr) {
        printf("Error: SDL_CreateGPUDevice(): %s\n", SDL_GetError());
        return -1;
    }

    // Claim window for GPU Device
    if (!SDL_ClaimWindowForGPUDevice(gpu_device, window)) {
        printf("Error: SDL_ClaimWindowForGPUDevice(): %s\n", SDL_GetError());
        return -1;
    }
    SDL_SetGPUSwapchainParameters(gpu_device, window, SDL_GPU_SWAPCHAINCOMPOSITION_SDR, SDL_GPU_PRESENTMODE_MAILBOX);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void) io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    // FIXME: Dynamically choose font based on platform and whether the font path can be found
    io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/Arial.ttf");

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup scaling
    ImGuiStyle &style = ImGui::GetStyle();
    style.ScaleAllSizes(
            main_scale);        // Bake a fixed style scale. (until we have a solution for dynamic style scaling, changing this requires resetting Style + calling this again)
    style.FontScaleDpi = main_scale;        // Set initial font scale. (using io.ConfigDpiScaleFonts=true makes this unnecessary. We leave both here for documentation purpose)

    // Setup Platform/Renderer backends
    ImGui_ImplSDL3_InitForSDLGPU(window);
    ImGui_ImplSDLGPU3_InitInfo init_info = {};
    init_info.Device = gpu_device;
    init_info.ColorTargetFormat = SDL_GetGPUSwapchainTextureFormat(gpu_device, window);
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

    // Our state
    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // Main loop
    bool done = false;
    while (!done) {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        // [If using SDL_MAIN_USE_CALLBACKS: call ImGui_ImplSDL3_ProcessEvent() from your SDL_AppEvent() function]
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL3_ProcessEvent(&event);
            if (event.type == SDL_EVENT_QUIT)
                done = true;
            if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED && event.window.windowID == SDL_GetWindowID(window))
                done = true;
        }

        SDL_Delay(16);

        // [If using SDL_MAIN_USE_CALLBACKS: all code below would likely be your SDL_AppIterate() function]
        if (SDL_GetWindowFlags(window) & SDL_WINDOW_MINIMIZED) {
            continue;
        }

        // Start the Dear ImGui frame
        ImGui_ImplSDLGPU3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        {
            static float f = 0.0f;
            static int counter = 0;

            ImGui::SetNextWindowSize(io.DisplaySize);
            ImGui::SetNextWindowPos({0, 0});
            ImGui::Begin("Hello, world!", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
                                                   ImGuiWindowFlags_NoSavedSettings);                          // Create a window called "Hello, world!" and append into it.

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
            ImGui::BeginTable("TABLE", 2, ImGuiTableFlags_BordersInnerV, {0,
                                                                          io.DisplaySize.y}); // FIXME: Height doesn't compensate for outer elements/window padding
            ImGui::TableNextColumn();
            ImGui::Text("GLTF / GLB");
            ImGui::Text("DRAG and DROP .gltf or .glb file here");
            ImGui::Text("NOTE: Embedded images will not be used.");
            ImGui::Spacing();
            ImGui::Text("\nTODO: Show TreeView of GLTF contents here once loaded.");

            if (true)
            {
                if (ImGui::TreeNodeEx("Model", ImGuiTreeNodeFlags_DefaultOpen)) {
                    // SCENES
                    if (ImGui::TreeNode("tinygltf_scenes", "Scenes - %d", model.scenes.size())) {
                        for (const auto &scene: model.scenes) {
                            if (ImGui::TreeNode(scene.name.c_str())) {
                                ImGui::TreePop();
                            }
                        }
                        ImGui::TreePop();
                    }

                    // NODES
                    if (ImGui::TreeNode("tinygltf_nodes", "Nodes/Objects - %d", model.nodes.size())) {
                        for (const auto &node: model.nodes) {
                            if (ImGui::TreeNode(node.name.c_str())) {
                                ImGui::TreePop();
                            }
                        }
                        ImGui::TreePop();
                    }

                    // MESHES
                    if (ImGui::TreeNode("tinygltf_meshes", "Meshes - %d", model.meshes.size())) {
                        for (const auto &mesh: model.meshes) {
                            if (ImGui::TreeNode(mesh.name.c_str())) {
                                ImGui::TreePop();
                            }
                        }
                        ImGui::TreePop();
                    }

                    // IMAGES
                    if (ImGui::TreeNode("tinygltf_images", "Images - %d", model.images.size())) {
                        for (const auto &image: model.images) {
                            if (ImGui::TreeNode(image.name.c_str())) {
                                ImGui::TreePop();
                            }
                        }
                        ImGui::TreePop();
                    }

                    // TEXTURES
                    if (ImGui::TreeNode("tinygltf_textures", "Textures - %d", model.textures.size())) {
                        for (const auto &texture: model.textures) {
                            if (ImGui::TreeNode(texture.name.c_str())) {
                                ImGui::TreePop();
                            }
                        }
                        ImGui::TreePop();
                    }

                    // MATERIALS
                    if (ImGui::TreeNode("tinygltf_materials", "Materials - %d", model.materials.size())) {
                        for (const auto &material: model.materials) {
                            if (ImGui::TreeNode(material.name.c_str())) {
                                ImGui::Text("Double Sided: %s", material.doubleSided ? "True" : "False");

                                ImGui::TreePop();
                            }
                        }
                        ImGui::TreePop();
                    }

                    // CAMERAS
                    if (ImGui::TreeNode("tinygltf_cameras", "Cameras - %d", model.cameras.size())) {
                        for (const auto &camera: model.cameras) {
                            if (ImGui::TreeNode(camera.name.c_str())) {
                                ImGui::TreePop();
                            }
                        }
                        ImGui::TreePop();
                    }

                    // LIGHTS
                    if (ImGui::TreeNode("tinygltf_lights", "Lights - %d", model.lights.size())) {
                        for (const auto &light: model.lights) {
                            if (ImGui::TreeNode(light.name.c_str())) {
                                ImGui::TreePop();
                            }
                        }
                        ImGui::TreePop();
                    }
                    ImGui::TreePop();
                }
            }

            ImGui::TableNextColumn();
            ImGui::Text("W3D Export Options");
            static char buf[16] = "";
            ImGui::InputTextWithHint("", "Leave blank if unsure", buf, 16);
            ImGui::SameLine();
            ImGui::Text("Container Name");
            const char *listbox_items[] = {"Hierarchy Model", "Mesh"};
            static int listbox_item_current = 0;
            ImGui::Combo("W3D Type", &listbox_item_current, listbox_items, 2);
            static bool opt_terrain = false;
            ImGui::Checkbox("Optimize for Terrain", &opt_terrain);

            // Export Button
            // NOTE: We manually move imgui's 'cursor' so it MUST be the last element of the 'window' created
            //       to ensure proper auto element placement.
            float button_width = std::max(ImGui::CalcTextSize("Export").x + style.FramePadding.x * 2.f, 128.f);
            float button_height = ImGui::CalcTextSize("Export").y + style.FramePadding.y * 2.f;
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + ImGui::GetContentRegionAvail().y - button_height);
            ImGui::ProgressBar(0.25f, {ImGui::GetContentRegionAvail().x - (button_width + style.ItemSpacing.x), 0});
            ImGui::SameLine();
            ImGui::Button("Export", {button_width, 0});
            ImGui::EndTable();

            ImGui::End();
        }

        // Rendering
        ImGui::Render();
        ImDrawData *draw_data = ImGui::GetDrawData();
        const bool is_minimized = (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f);

        SDL_GPUCommandBuffer *command_buffer = SDL_AcquireGPUCommandBuffer(gpu_device); // Acquire a GPU command buffer

        SDL_GPUTexture *swapchain_texture;
        SDL_AcquireGPUSwapchainTexture(command_buffer, window, &swapchain_texture, nullptr,
                                       nullptr); // Acquire a swapchain texture

        if (swapchain_texture != nullptr && !is_minimized) {
            // This is mandatory: call ImGui_ImplSDLGPU3_PrepareDrawData() to upload the vertex/index buffer!
            ImGui_ImplSDLGPU3_PrepareDrawData(draw_data, command_buffer);

            // Setup and start a render pass
            SDL_GPUColorTargetInfo target_info = {};
            target_info.texture = swapchain_texture;
            target_info.clear_color = SDL_FColor{clear_color.x, clear_color.y, clear_color.z, clear_color.w};
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

    // Cleanup
    // [If using SDL_MAIN_USE_CALLBACKS: all code below would likely be your SDL_AppQuit() function]
    SDL_WaitForGPUIdle(gpu_device);
    ImGui_ImplSDL3_Shutdown();
    ImGui_ImplSDLGPU3_Shutdown();
    ImGui::DestroyContext();

    SDL_ReleaseWindowFromGPUDevice(gpu_device, window);
    SDL_DestroyGPUDevice(gpu_device);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
