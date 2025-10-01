//
// Created by cyberarm on 2025-06-30.
//

#pragma once

#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE

#include <iostream>
#include <string>
#include <format>

#include "w3d_file.h"
#include "chunkio.h"
#include "w3d_hierarchy_model.h"

#include "tiny_gltf.h"

#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlgpu3.h"

#include <SDL3/SDL.h>

#define APP_TITLE "GLTF to W3D Converter | ALPHA QUALITY SOFTWARE USE AT YOUR OWN RISK!"
#define GLTF_FILENAME "C:/Users/cyber/Nextcloud/Documents/Blender/W3D/tsr_basic_test_map.glb"
// #define GLTF_FILENAME "D:/W3DHub/games/tiberian-sun-reborn/LevelEdit/TS_Level/ts_level.glb"
#define W3D_FILENAME "D:/W3DHub/games/tiberian-sun-reborn/LevelEdit/TS_Level/ts_level.w3d"

class Window {
private:
    float m_main_scale = 1.f;
    SDL_Window *m_window = nullptr;
    SDL_GPUDevice *m_gpu_device = nullptr;

    ImGuiIO *m_io = nullptr;
    ImGuiStyle *m_style = nullptr;
    ImVec4 m_clear_color{0.75f, 0.25f, 0.f, 1.f};

    bool m_done = false;

    tinygltf::Model m_model = tinygltf::Model();

    bool init_sdl();

    bool init_imgui();

public:
    Window();

    ~Window();

    int init();

    void run();

    void update();

    void draw();

    void draw_main_window();

    void draw_intro_window();

    bool load_model(const std::string &filename);

    bool export_w3d_hierarchy_model(const std::string &container_name, const std::string &filename,
                                    bool optimize_for_terrain);

    static bool intern_fake_image_loader(tinygltf::Image *image, const int image_idx,
                                         std::string * error, std::string * warn, int req_width, int req_height,
                                         const unsigned char *bytes, int size, void * user_data) {
        return true;
    }
};
