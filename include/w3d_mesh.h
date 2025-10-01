//
// Created by cyberarm on 2025-06-27.
//

#pragma once

#include "w3d_file.h"
#include "tiny_gltf.h"
#include "chunkio.h"

class W3dMesh {
private:
    W3dMeshHeader3Struct m_header = {};
    std::vector<IOVector3Struct> m_vertices = {};
    std::vector<IOVector3Struct> m_normals = {};
    std::vector<IOVector3Struct> m_uvs = {};
    std::vector<W3dTriStruct> m_triangles = {};
    std::vector<uint32_t> m_shade_indices = {};
    W3dMaterialInfoStruct m_material_info = {};
    // FIXME: Vertex Materials also probably needs its own struct to tie material name and W3dVertexMaterialStruct together
    std::vector<W3dVertexMaterialStruct> m_vertex_material_info = {};
    std::vector<W3dShaderStruct> m_shaders = {};
    // FIXME: Textures can also have an **OPTIONAL** W3dTextureInfoStruct attached
    //        probably needs its own struct to tie texture name and W3dTextureInfoStruct together
    std::vector<std::string> m_textures = {};
    std::vector<std::string> m_material_passes = {}; // FIXME: Create a class/struct for this to hold everything for it...
    W3dMeshAABTreeHeader m_aabb_tree_header = {};
    std::vector<uint32_t> m_aabbtree_polygon_indices = {};
    std::vector<W3dMeshAABTreeNode> m_aabbtree_nodes = {};

    std::string m_container_name;
    tinygltf::Model m_gltf_model;
    tinygltf::Mesh m_gltf_mesh;
    ChunkSaveClass m_writer;

public:
    explicit W3dMesh(std::string &container_name, const tinygltf::Model &model, const tinygltf::Mesh &mesh, const ChunkSaveClass &writer);

    bool convert();
    bool add_vertices();
    bool add_normals();
    bool add_triangles();
    bool add_shade_indices();
    bool add_vertex_material_info();
    bool add_vertice_materials();
    bool add_textures();
    bool add_material_passes();
    bool add_aabb_tree();

    bool write();
    bool write_header();
    bool write_vertices();
    bool write_normals();
    bool write_triangles();
    bool write_shade_indices();
    bool write_vertex_material_info();
    bool write_vertice_materials();
    bool write_textures();
    bool write_material_passes();
    bool write_aabb_tree();
};