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

bool exportW3DHierarchyModel(tinygltf::Model &model, const std::string& filename, bool optimize_for_terrain = false)
{
    // TODO: Translate GLTF data to W3D data and write it out.
    SDL_IOStream* stream = SDL_IOFromFile(filename.c_str(), "wb");

    auto m_writer = ChunkSaveClass(stream);

    W3dHierarchyModel(model, m_writer, true);

    SDL_CloseIO(stream);
    return  true;

    std::vector<W3dPivotStruct> pivots = {};
    W3dPivotStruct pivot = {};
    uint32_t pivot_parent_idx = 0xffffffff;

    // write RootTransform
    strcpy(pivot.Name, "RootTransform");
    pivot.ParentIdx = pivot_parent_idx;
    pivot.Translation.X = 0;
    pivot.Translation.Y = 0;
    pivot.Translation.Z = 0;
    pivot.EulerAngles.X = 0;
    pivot.EulerAngles.Y = 0;
    pivot.EulerAngles.Z = 1;
    pivot.Rotation.Q[0] = 0;
    pivot.Rotation.Q[1] = 0;
    pivot.Rotation.Q[2] = 0;
    pivot.Rotation.Q[3] = 1;

    pivots.emplace_back(pivot);


    for (auto node : model.nodes)
    {
        if (node.mesh < 0)
            continue;

        tinygltf::Mesh mesh = model.meshes.at(node.mesh);

        // Node's mesh is not a proxy/placeholder/pivot thingy
        if (mesh.name.find('~') == std::string::npos)
            continue;

        strcpy(pivot.Name, mesh.name.c_str());
        pivot.ParentIdx = 0;
        pivot.Translation.X = node.translation.size() > 0 ? node.translation[0] : 0;
        pivot.Translation.Y = node.translation.size() > 0 ? node.translation[1] : 0;
        pivot.Translation.Z = node.translation.size() > 0 ? node.translation[2] : 0;
        pivot.EulerAngles.X = node.rotation.size() > 0 ? node.rotation[0] : 0;
        pivot.EulerAngles.Y = node.rotation.size() > 0 ? node.rotation[1] : 0;
        pivot.EulerAngles.Z = node.rotation.size() > 0 ? node.rotation[2] : 0;
        pivot.Rotation.Q[0] = 0;
        pivot.Rotation.Q[1] = 0;
        pivot.Rotation.Q[2] = 0;
        pivot.Rotation.Q[3] = 1;

        pivots.emplace_back(pivot);
    }

    auto writer = ChunkSaveClass(stream);
    writer.begin_chunk(W3D_CHUNK_HIERARCHY);
    W3dHierarchyStruct hierarchy_header = {};
    writer.begin_chunk(W3D_CHUNK_HIERARCHY_HEADER);
    hierarchy_header.Version = W3D_CURRENT_HTREE_VERSION;
    strcpy(hierarchy_header.Name, "ts_level");
    hierarchy_header.NumPivots = pivots.size();
    W3dVectorStruct v = {0, 0, 0};
    hierarchy_header.Center = v;
    writer.write((uint8_t *) &hierarchy_header, sizeof(W3dHierarchyStruct));
    writer.end_chunk();

    // WRITE PIVOTS!
    writer.begin_chunk(W3D_CHUNK_PIVOTS);
    for(auto piv : pivots)
    {
        writer.write((uint8_t *) &piv, sizeof(W3dPivotStruct));
    }
    writer.end_chunk();
    writer.end_chunk();


    W3dHLodHeaderStruct hLodHeaderStruct = {};
    W3dHLodArrayHeaderStruct hLodArrayHeaderStruct = {};
    writer.begin_chunk(W3D_CHUNK_HLOD);
    writer.begin_chunk(W3D_CHUNK_HLOD_HEADER);
    hLodHeaderStruct.Version = W3D_CURRENT_HLOD_VERSION;
    strcpy(hLodHeaderStruct.Name, "ts_level");
    strcpy(hLodHeaderStruct.HierarchyName, "ts_level");
    hLodHeaderStruct.LodCount = 1;
    writer.write((uint8_t *) &hLodHeaderStruct, sizeof(W3dHLodHeaderStruct));
    writer.end_chunk();
    writer.begin_chunk(W3D_CHUNK_HLOD_PROXY_ARRAY);
    writer.begin_chunk(W3D_CHUNK_HLOD_SUB_OBJECT_ARRAY_HEADER);
    hLodArrayHeaderStruct.ModelCount = pivots.size();
    hLodArrayHeaderStruct.MaxScreenSize = 0;
    writer.write((uint8_t *) &hLodArrayHeaderStruct, sizeof(W3dHLodArrayHeaderStruct));
    writer.end_chunk();
    W3dHLodSubObjectStruct w3DHLodSubObjectStruct = {};
    size_t i = 0;
    for(auto piv : pivots)
    {
        // Skip root transform
        if (i == 0)
        {
            i++;
            continue;
        }

        writer.begin_chunk(W3D_CHUNK_HLOD_SUB_OBJECT);
        std::string name = piv.Name;
        size_t tilde_index = name.find('~');
        printf("tilde: %zu (%s)\n", tilde_index, name.c_str());
        if (tilde_index)
        {
            name = name.substr(0, tilde_index);
        }
        strcpy(w3DHLodSubObjectStruct.Name, name.c_str());
        w3DHLodSubObjectStruct.BoneIndex = i;
        writer.write((uint8_t *) &w3DHLodSubObjectStruct, sizeof(W3dHLodSubObjectStruct));
        writer.end_chunk();

        i++;
    }
    writer.end_chunk();
    writer.end_chunk();

    SDL_CloseIO(stream);

    return true;
}

bool loadModel(tinygltf::Model &model, const std::string& filename) {
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

int main(int, char**)
{
    tinygltf::Model model;
    if (loadModel(model, GLTF_FILENAME)) {
        for (const auto& mesh: model.meshes) {
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
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD))
    {
        printf("Error: SDL_Init(): %s\n", SDL_GetError());
        return -1;
    }

    // Create SDL window graphics context
    float main_scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
    SDL_WindowFlags window_flags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN | SDL_WINDOW_HIGH_PIXEL_DENSITY;
    SDL_Window* window = SDL_CreateWindow(APP_TITLE, (int)(1280 * main_scale), (int)(720 * main_scale), window_flags);
    if (window == nullptr)
    {
        printf("Error: SDL_CreateWindow(): %s\n", SDL_GetError());
        return -1;
    }
    SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    SDL_ShowWindow(window);

    // Create GPU Device
    SDL_GPUDevice* gpu_device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_DXIL | SDL_GPU_SHADERFORMAT_METALLIB,true,nullptr);
    if (gpu_device == nullptr)
    {
        printf("Error: SDL_CreateGPUDevice(): %s\n", SDL_GetError());
        return -1;
    }

    // Claim window for GPU Device
    if (!SDL_ClaimWindowForGPUDevice(gpu_device, window))
    {
        printf("Error: SDL_ClaimWindowForGPUDevice(): %s\n", SDL_GetError());
        return -1;
    }
    SDL_SetGPUSwapchainParameters(gpu_device, window, SDL_GPU_SWAPCHAINCOMPOSITION_SDR, SDL_GPU_PRESENTMODE_MAILBOX);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup scaling
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(main_scale);        // Bake a fixed style scale. (until we have a solution for dynamic style scaling, changing this requires resetting Style + calling this again)
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
    while (!done)
    {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        // [If using SDL_MAIN_USE_CALLBACKS: call ImGui_ImplSDL3_ProcessEvent() from your SDL_AppEvent() function]
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL3_ProcessEvent(&event);
            if (event.type == SDL_EVENT_QUIT)
                done = true;
            if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED && event.window.windowID == SDL_GetWindowID(window))
                done = true;
        }

        SDL_Delay(16);

        // [If using SDL_MAIN_USE_CALLBACKS: all code below would likely be your SDL_AppIterate() function]
        if (SDL_GetWindowFlags(window) & SDL_WINDOW_MINIMIZED)
        {
            continue;
        }

        // Start the Dear ImGui frame
        ImGui_ImplSDLGPU3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
        if (!show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
        {
            static float f = 0.0f;
            static int counter = 0;

            ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

            ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
            ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
            ImGui::Checkbox("Another Window", &show_another_window);

            ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
            ImGui::ColorEdit4("clear color", (float*)&clear_color); // Edit 3 floats representing a color

            if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
                counter++;
            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
            ImGui::End();
        }

        // 3. Show another simple window.
        if (show_another_window)
        {
            ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
            ImGui::Text("Hello from another window!");
            if (ImGui::Button("Close Me"))
                show_another_window = false;
            ImGui::End();
        }

        // Rendering
        ImGui::Render();
        ImDrawData* draw_data = ImGui::GetDrawData();
        const bool is_minimized = (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f);

        SDL_GPUCommandBuffer* command_buffer = SDL_AcquireGPUCommandBuffer(gpu_device); // Acquire a GPU command buffer

        SDL_GPUTexture* swapchain_texture;
        SDL_AcquireGPUSwapchainTexture(command_buffer, window, &swapchain_texture, nullptr, nullptr); // Acquire a swapchain texture

        if (swapchain_texture != nullptr && !is_minimized)
        {
            // This is mandatory: call ImGui_ImplSDLGPU3_PrepareDrawData() to upload the vertex/index buffer!
            ImGui_ImplSDLGPU3_PrepareDrawData(draw_data, command_buffer);

            // Setup and start a render pass
            SDL_GPUColorTargetInfo target_info = {};
            target_info.texture = swapchain_texture;
            target_info.clear_color = SDL_FColor { clear_color.x, clear_color.y, clear_color.z, clear_color.w };
            target_info.load_op = SDL_GPU_LOADOP_CLEAR;
            target_info.store_op = SDL_GPU_STOREOP_STORE;
            target_info.mip_level = 0;
            target_info.layer_or_depth_plane = 0;
            target_info.cycle = false;
            SDL_GPURenderPass* render_pass = SDL_BeginGPURenderPass(command_buffer, &target_info, 1, nullptr);

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
