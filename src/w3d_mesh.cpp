//
// Created by cyberarm on 2025-06-27.
//

#include "w3d_mesh.h"

W3dMesh::W3dMesh(tinygltf::Model &model, tinygltf::Mesh &mesh)  : m_gltf_model(model), m_gltf_mesh(mesh)
{
    // Replicate gltf mesh data and do any needed conversions to W3D Engine space
}
