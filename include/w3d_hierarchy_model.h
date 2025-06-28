//
// Created by cyberarm on 2025-06-27.
//

#pragma once

#include "w3d_file.h"
#include "chunkio.h"
#include "tiny_gltf.h"

#include "w3d_pivot.h"

class W3dHierarchyModel
{
private:
    tinygltf::Model m_model;
    ChunkSaveClass m_writer;
    bool m_optimize_for_terrain = false;
    std::vector<W3dPivot> m_meshes = {};
    std::vector<W3dPivot> m_pivots = {};
    std::unordered_map<std::string, uint8_t> m_proxy_counter = {};

    // For showing progress in ImGui
    bool m_result = false;
    uint32_t m_task_stages = 10;
    uint32_t m_task_current_stage = 0;

    // Constantly const constants
    // TODO: Make static constants?
    const W3dVectorStruct m_origin = {0.f, 0.f, 0.f};
    const W3dPivotFixupStruct m_identity_matrix = {
            1, 0, 0,
            0, 1, 0,
            0, 0, 1,
            0, 0, 0
    };
    public:
    W3dHierarchyModel(tinygltf::Model model, ChunkSaveClass csave, bool optimize_for_terrain = false);
    ~W3dHierarchyModel();

    bool convert();
    bool add_root_transform();
    bool add_pivots();
    bool add_proxies();

    bool write();
    bool write_hierarchy_header();
    bool write_pivots();
    bool write_pivot_fixups();
    bool write_meshes();
    bool write_hierarchical_level_of_detail();

    bool write_mesh(tinygltf::Mesh &mesh) { return false; };
};
