//
// Created by cyberarm on 2025-06-27.
//

#include "w3d_hierarchy_model.h"

W3dHierarchyModel::W3dHierarchyModel(tinygltf::Model model, ChunkSaveClass writer, bool optimize_for_terrain) :
        m_model(model),
        m_writer(writer),
        m_optimize_for_terrain(optimize_for_terrain) {
    // Collect Meshes
    //      Collect Vertices (Position, UV, etc.)
    //      Collect Materials
    //      Collect Axis Aligned Bounding Box(es)
    // Collect Proxies

    // Write out file
    m_result = write();
}

W3dHierarchyModel::~W3dHierarchyModel() {}

bool W3dHierarchyModel::write() {
    m_writer.begin_chunk(W3D_CHUNK_HIERARCHY);
    write_hierarchy_header();
    write_pivots();
    write_pivot_fixups();
    m_writer.end_chunk();

    write_meshes();
    write_hierarchical_level_of_detail();

    return true;
}

bool W3dHierarchyModel::write_hierarchy_header() {
    W3dHierarchyStruct header = {
            W3D_CURRENT_HTREE_VERSION,
            "TS_Level", // FIXME: Get filename/htree name (i.e. Container Name)
            static_cast<uint32_t>(m_pivots.size()),
            m_origin
    };

    m_writer.begin_chunk(W3D_CHUNK_HIERARCHY_HEADER);
    m_writer.write(&header, sizeof(W3dHierarchyStruct));
    m_writer.end_chunk();

    return true;
}

bool W3dHierarchyModel::write_pivots() {
    return true;
}

bool W3dHierarchyModel::write_meshes() {
    for (auto mesh: m_model.meshes) {
        // Don't store proxy mesh data
        if (mesh.name.find('~'))
            continue;

        write_mesh(mesh);
    }
//        W3dMeshHeader3Struct
    return true;
}
