//
// Created by cyberarm on 2025-06-27.
//

#include "w3d_mesh.h"

#include <iostream>

W3dMesh::W3dMesh(std::string &container_name, const tinygltf::Model &model, const tinygltf::Mesh &mesh, const ChunkSaveClass &writer)  :
    m_container_name(std::move(container_name)),
    m_gltf_model(model),
    m_gltf_mesh(mesh),
    m_writer(writer)
{
    // Replicate gltf mesh data and do any needed conversions to W3D Engine space

    convert();
    write();
}

// Loaders
bool W3dMesh::convert() {
    add_vertices();
    add_normals();
    add_triangles();
    add_shade_indices();
    add_vertex_material_info();
    add_vertice_materials();
    add_textures();
    add_material_passes();
    add_aabb_tree();

    return false;
}

bool W3dMesh::add_vertices() {
    for (auto &prim : m_gltf_mesh.primitives) {
        if (prim.mode != TINYGLTF_MODE_TRIANGLES) {
            puts("FATAL: mesh is not triangles.");
            break;
        }

        const auto position_index = prim.attributes.at("POSITION");
        const auto &accessor = m_gltf_model.accessors.at(position_index);
        const auto &buffer_view = m_gltf_model.bufferViews.at(accessor.bufferView);
        const auto &buffer = m_gltf_model.buffers.at(buffer_view.buffer);

        assert(accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);
        assert(accessor.type == TINYGLTF_TYPE_VEC3);

        // Loop through positions and reconstruct floats for vect3
        for (size_t i = 0; i < accessor.count * (3 * 4);) {
            IOVector3Struct vertex = {};

            for (size_t j = 0; j < 3; j++) {
                float value;
                const std::array byte_array = {
                    buffer.data.at(accessor.byteOffset + buffer_view.byteOffset + i++),
                    buffer.data.at(accessor.byteOffset + buffer_view.byteOffset + i++),
                    buffer.data.at(accessor.byteOffset + buffer_view.byteOffset + i++),
                    buffer.data.at(accessor.byteOffset + buffer_view.byteOffset + i++)
                };

                memcpy(&value, byte_array.data(), sizeof(float));

                if (j == 0)
                    vertex.X = value;
                else if (j == 1)
                    vertex.Y = value;
                else if (j == 2)
                    vertex.Z = value;
            }

            m_vertices.push_back(vertex);
        }
    }

    std::cout << "primitives: " << m_gltf_mesh.primitives.size() << std::endl;

    return false;
}

bool W3dMesh::add_normals() {
    return false;
}


bool W3dMesh::add_triangles() {
    return false;
}

bool W3dMesh::add_shade_indices() {
    return false;
}

bool W3dMesh::add_vertex_material_info() {
    return false;
}

bool W3dMesh::add_vertice_materials() {
    return false;
}

bool W3dMesh::add_textures() {
    return false;
}

bool W3dMesh::add_material_passes() {
    return false;
}

bool W3dMesh::add_aabb_tree() {
    return false;
}

// WRITERS
bool W3dMesh::write() {
    m_writer.begin_chunk(W3D_CHUNK_MESH);

    write_header();
    write_vertices();
    write_normals();
    write_triangles();
    write_shade_indices();
    write_vertex_material_info();
    write_vertice_materials();
    write_textures();
    write_material_passes();
    write_aabb_tree();

    m_writer.end_chunk(); // W3D_CHUNK_MESH

    return false;
}

bool W3dMesh::write_header() {
    m_header.Version = W3D_CURRENT_MESH_VERSION;
    strcpy(m_header.MeshName, m_gltf_mesh.name.c_str());
    strcpy(m_header.ContainerName, m_container_name.c_str());
    // FIXME: Support user specified options
    m_header.Attributes = W3D_MESH_FLAG_GEOMETRY_TYPE_NORMAL | W3D_MESH_FLAG_COLLISION_TYPE_PHYSICAL | W3D_MESH_FLAG_COLLISION_TYPE_PROJECTILE | W3D_MESH_FLAG_COLLISION_TYPE_CAMERA;
    m_header.NumTris = m_triangles.size();
    m_header.NumVertices = m_vertices.size();
    m_header.NumMaterials = m_vertex_material_info.size(); // FIXME: Check if this is the correct list for this count
    m_header.NumDamageStages = 0;
    m_header.SortLevel = SORT_LEVEL_NONE;
    m_header.PrelitVersion = 0;
    m_header.FutureCounts[0] = 0;
    // FIXME: Dynamically generate value for this flag
    m_header.VertexChannels = W3D_VERTEX_CHANNEL_LOCATION | W3D_VERTEX_CHANNEL_NORMAL;
    // FIXME: Dynamically generate value for this flag
    m_header.FaceChannels = W3D_FACE_CHANNEL_FACE;
    m_header.Min = {0, 0, 0}; // FIXME
    m_header.Max = {0, 0, 0};// FIXME
    m_header.SphCenter = {0, 0, 0}; // FIXME
    m_header.SphRadius = 0; // FIXME

    m_writer.begin_chunk(W3D_CHUNK_MESH_HEADER3);
    m_writer.write(&m_header, sizeof(m_header));
    m_writer.end_chunk(); // W3D_CHUNK_MESH_HEADER3
    return false;
}

bool W3dMesh::write_vertices() {
    if (m_vertices.empty())
        return false;

    m_writer.begin_chunk(W3D_CHUNK_VERTICES);
    for (auto &v : m_vertices)
        m_writer.write(&v, sizeof(IOVector3Struct));
    m_writer.end_chunk(); // W3D_CHUNK_VERTICES

    return true;
}

bool W3dMesh::write_normals() {
    if (m_normals.empty())
        return false;

    m_writer.begin_chunk(W3D_CHUNK_VERTEX_NORMALS);
    for (auto &v : m_normals)
        m_writer.write(&v, sizeof(IOVector3Struct));
    m_writer.end_chunk(); // W3D_CHUNK_VERTEX_NORMALS

    return true;
}

bool W3dMesh::write_triangles() {
    if (m_triangles.empty())
        return false;

    m_writer.begin_chunk(O_W3D_CHUNK_TRIANGLES);
    for (auto &v : m_triangles)
        m_writer.write(&v, sizeof(W3dTriStruct));
    m_writer.end_chunk(); // O_W3D_CHUNK_TRIANGLES

    return true;
}

bool W3dMesh::write_shade_indices() {
    return false;
}

bool W3dMesh::write_vertex_material_info() {
    return false;
}

bool W3dMesh::write_vertice_materials() {
    return false;
}

bool W3dMesh::write_textures() {
    return false;
}

bool W3dMesh::write_material_passes() {
    return false;
}

bool W3dMesh::write_aabb_tree() {
    return false;
}
