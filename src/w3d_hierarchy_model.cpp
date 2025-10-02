//
// Created by cyberarm on 2025-06-27.
//

#include "w3d_hierarchy_model.h"

#include <format>
#include <iostream>
#include <utility>

W3dHierarchyModel::W3dHierarchyModel(const std::string &container_name, tinygltf::Model *model, const ChunkSaveClass &writer, const bool optimize_for_terrain) :
        m_container_name(container_name),
        m_model(model),
        m_writer(writer),
        m_optimize_for_terrain(optimize_for_terrain) {
    // Collect Meshes
    //      Collect Vertices (Position, UV, etc.)
    //      Collect Materials
    //      Collect Axis Aligned Bounding Box(es)
    // Collect Proxies
    convert();

    // Write out file
    m_result = write();
}

W3dHierarchyModel::~W3dHierarchyModel() {}

bool W3dHierarchyModel::convert() {
    add_root_transform();
    add_pivots();

    return true;
}

bool W3dHierarchyModel::add_root_transform() {
    W3dPivotStruct pivot = {
            .Name="RootTransform",
            .ParentIdx=0xffffffff,
            .Translation={0, 0, 0},
            .EulerAngles={0, 0, 0},
            .Rotation={0, 0, 0, 1}
    };

    m_pivots.emplace_back(pivot);

    return true;
}

bool W3dHierarchyModel::add_pivots() {
    add_proxies();

    return true;
}

bool W3dHierarchyModel::add_proxies() {
    for (auto node : m_model->nodes)
    {
        // Node's mesh is not a proxy/placeholder/pivot thingy
        if (node.name.find('~') == std::string::npos)
            continue;

        W3dPivotStruct pivot = {};

        strcpy(pivot.Name, node.name.c_str());
        pivot.ParentIdx = 0;
        pivot.Translation.X = !node.translation.empty() ? node.translation[0] : 0;
        pivot.Translation.Y = !node.translation.empty() ? node.translation[1] : 0;
        pivot.Translation.Z = !node.translation.empty() ? node.translation[2] : 0;
        pivot.EulerAngles.X = !node.rotation.empty() ? node.rotation[0] : 0; // FIXME: Convert Quat to Euler?
        pivot.EulerAngles.Y = !node.rotation.empty() ? node.rotation[1] : 0; // FIXME: Convert Quat to Euler?
        pivot.EulerAngles.Z = !node.rotation.empty() ? node.rotation[2] : 0; // FIXME: Convert Quat to Euler?
        pivot.Rotation.Q[0] = !node.rotation.empty() ? node.rotation[0] : 0;
        pivot.Rotation.Q[1] = !node.rotation.empty() ? node.rotation[1] : 0;
        pivot.Rotation.Q[2] = !node.rotation.empty() ? node.rotation[2] : 0;
        pivot.Rotation.Q[3] = !node.rotation.empty() ? node.rotation[3] : 1;

        m_pivots.emplace_back(W3dPivot{.m_data=pivot, .m_proxy=true});
    }

    return true;
}

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
            .Version=W3D_CURRENT_HTREE_VERSION,
            .Name="",
            .NumPivots=static_cast<uint32_t>(m_pivots.size()),
            .Center=m_origin
    };
    std::strcpy(header.Name, m_container_name.c_str());

    m_writer.begin_chunk(W3D_CHUNK_HIERARCHY_HEADER);
    m_writer.write(&header, sizeof(W3dHierarchyStruct));
    m_writer.end_chunk();

    return true;
}

bool W3dHierarchyModel::write_pivots() {
    m_writer.begin_chunk(W3D_CHUNK_PIVOTS);
    for(auto piv : m_pivots)
    {
        // FIXME: Automagically rename proxies so that their numbered distinctly
        m_writer.write(&piv.data(), sizeof(W3dPivotStruct));
    }
    m_writer.end_chunk();

    return true;
}

bool W3dHierarchyModel::write_pivot_fixups() {
    m_writer.begin_chunk(W3D_CHUNK_PIVOT_FIXUPS);
    for(auto piv : m_pivots)
    {
        m_writer.write(&m_identity_matrix, sizeof(W3dPivotFixupStruct));
    }
    m_writer.end_chunk();

    return true;
}

bool W3dHierarchyModel::write_meshes() {
    for (const auto node : m_model->nodes)
    {
        if (node.mesh < 0)
            continue;

        // Don't store proxy mesh data
        if (node.name.find('~') != std::string::npos)
            continue;

        const auto &mesh = m_model->meshes.at(node.mesh);

        W3dPivotStruct pivot = {};
        pivot.ParentIdx = 0;
        strcpy(pivot.Name, std::format("{}.{}",m_container_name.c_str(), node.name.c_str()).c_str());
        m_meshes.emplace_back(pivot);

        write_mesh(mesh);
    }

    return true;
}

bool W3dHierarchyModel::write_hierarchical_level_of_detail() {
    W3dHLodHeaderStruct header {
        .Version=W3D_CURRENT_HLOD_VERSION,
        .LodCount=1
    };
    std::strcpy(header.Name, m_container_name.c_str());
    std::strcpy(header.HierarchyName, m_container_name.c_str());

    m_writer.begin_chunk(W3D_CHUNK_HLOD);
    m_writer.begin_chunk(W3D_CHUNK_HLOD_HEADER);
    m_writer.write(&header, sizeof(W3dHLodHeaderStruct));
    m_writer.end_chunk();

    m_writer.begin_chunk(W3D_CHUNK_HLOD_LOD_ARRAY);
    m_writer.begin_chunk(W3D_CHUNK_HLOD_SUB_OBJECT_ARRAY_HEADER);
    W3dHLodArrayHeaderStruct hlod_array_header = {
        .ModelCount=static_cast<uint32_t>(m_meshes.size()),
        .MaxScreenSize=1.0f
};
    m_writer.write(&hlod_array_header, sizeof(W3dHLodArrayHeaderStruct));
    m_writer.end_chunk();

    size_t i = 0;
    for(auto piv : m_meshes)
    {
        m_writer.begin_chunk(W3D_CHUNK_HLOD_SUB_OBJECT);
        std::string name = piv.data().Name;
        W3dHLodSubObjectStruct hlod_subobject_struct {};
        hlod_subobject_struct.BoneIndex = i;
        strcpy(hlod_subobject_struct.Name, name.c_str());
        m_writer.write(&hlod_subobject_struct, sizeof(W3dHLodSubObjectStruct));
        m_writer.end_chunk(); // W3D_CHUNK_HLOD_SUB_OBJECT

        i++;
    }
    m_writer.end_chunk(); // W3D_CHUNK_HLOD_LOD_ARRAY

    m_writer.begin_chunk(W3D_CHUNK_HLOD_PROXY_ARRAY);
    m_writer.begin_chunk(W3D_CHUNK_HLOD_SUB_OBJECT_ARRAY_HEADER);
    hlod_array_header = {
            .ModelCount=static_cast<uint32_t>(m_pivots.size()),
            .MaxScreenSize=0
    };
    m_writer.write(&hlod_array_header, sizeof(W3dHLodArrayHeaderStruct));
    m_writer.end_chunk(); // W3D_CHUNK_HLOD_SUB_OBJECT_ARRAY_HEADER

    i = 0;
    for(auto piv : m_pivots)
    {
        // Skip non proxies
        if (!piv.is_proxy())
        {
            i++;
            continue;
        }

        m_writer.begin_chunk(W3D_CHUNK_HLOD_SUB_OBJECT);
        std::string name = piv.data().Name;
        size_t tilde_index = name.find('~');
        // std::cout << "tilde: " << tilde_index << " (" << name.c_str() << ")" << std::endl;
        if (tilde_index)
        {
            name = name.substr(0, tilde_index);
        }
        W3dHLodSubObjectStruct hlod_subobject_struct {};
        hlod_subobject_struct.BoneIndex = i;
        strcpy(hlod_subobject_struct.Name, name.c_str());
        m_writer.write(&hlod_subobject_struct, sizeof(W3dHLodSubObjectStruct));
        m_writer.end_chunk(); // W3D_CHUNK_HLOD_SUB_OBJECT

        i++;
    }

    m_writer.end_chunk(); // W3D_CHUNK_HLOD_PROXY_ARRAY
    m_writer.end_chunk(); // W3D_CHUNK_HLOD

    return true;
}

bool W3dHierarchyModel::write_mesh(const tinygltf::Mesh &mesh) {
    std::cout << "WRITING MESH: " << mesh.name.c_str() << std::endl;
    W3dMesh w3d_mesh(m_container_name, *m_model, mesh, m_writer);

    return true;
}
