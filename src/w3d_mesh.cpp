//
// Created by cyberarm on 2025-06-27.
//

#include "w3d_mesh.h"

#include <iostream>

#include "SDL3/SDL_endian.h"

W3dMesh::W3dMesh(std::string &container_name, const tinygltf::Model &model, const tinygltf::Mesh &mesh,
                 const ChunkSaveClass &writer) : m_container_name(container_name),
                                                 m_gltf_model(model),
                                                 m_gltf_mesh(mesh),
                                                 m_writer(writer) {
    // Replicate gltf mesh data and do any needed conversions to W3D Engine space

    convert();
    write();
}

// Loaders
bool W3dMesh::convert() {
    add_user_text();
    add_vertices();
    add_vertex_normals();
    add_triangles();
    add_vertex_influences();
    add_vertex_shade_indices();
    add_material_info();
    add_vertex_materials();
    add_textures();
    add_material_passes();
    add_aabb_tree();

    return false;
}

bool W3dMesh::add_user_text() {
    return false;
}

bool W3dMesh::add_vertices() {
    for (auto &prim: m_gltf_mesh.primitives) {
        if (prim.mode != TINYGLTF_MODE_TRIANGLES) {
            puts("FATAL: mesh is not triangles.");
            return false;
        }

        const auto position_index = prim.attributes.at("POSITION");
        const auto &accessor = m_gltf_model.accessors.at(position_index);
        const auto &buffer_view = m_gltf_model.bufferViews.at(accessor.bufferView);
        const auto &buffer = m_gltf_model.buffers.at(buffer_view.buffer);

        assert(accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);
        assert(accessor.type == TINYGLTF_TYPE_VEC3);

        // Set mesh min/max
        m_header.Min = {
            static_cast<float>(accessor.minValues[0]),
            static_cast<float>(accessor.minValues[1]),
            static_cast<float>(accessor.minValues[2])
        };
        m_header.Max = {
            static_cast<float>(accessor.maxValues[0]),
            static_cast<float>(accessor.maxValues[1]),
            static_cast<float>(accessor.maxValues[2])
        };

        // Loop through positions and reconstruct floats for vect3
        for (size_t i = 0; i < accessor.count * (3 * 4);) {
            IOVector3Struct vertex = {};

            for (size_t j = 0; j < 3; j++) {
                float value;
                const std::array byte_array = {
                    // FIXME: Account for possible STRIDE
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
                else
                    vertex.Z = value;
            }

            m_vertices.push_back(vertex);
        }
    }

    return true;
}

bool W3dMesh::add_vertex_normals() {
    for (auto &prim: m_gltf_mesh.primitives) {
        if (prim.mode != TINYGLTF_MODE_TRIANGLES) {
            puts("FATAL: mesh is not triangles.");
            return false;
        }

        const auto normals_index = prim.attributes.at("NORMAL");
        const auto &accessor = m_gltf_model.accessors.at(normals_index);
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
                    // FIXME: Account for possible STRIDE
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
                else
                    vertex.Z = value;
            }

            m_normals.push_back(vertex);
        }
    }

    return true;
}

bool W3dMesh::add_triangles() {
    for (auto &prim: m_gltf_mesh.primitives) {
        if (prim.mode != TINYGLTF_MODE_TRIANGLES) {
            puts("FATAL: mesh is not triangles.");
            return false;
        }

        // Load triangle indices from buffer
        if (prim.indices >= 0) {
            W3dTriStruct triangle = {};
            auto &accessor = m_gltf_model.accessors.at(prim.indices);
            auto &buffer_view = m_gltf_model.bufferViews.at(accessor.bufferView);
            auto &buffer = m_gltf_model.buffers.at(buffer_view.buffer);

            assert(buffer_view.target == TINYGLTF_TARGET_ELEMENT_ARRAY_BUFFER);
            assert(accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT); // uint16_t
            assert(accessor.type == TINYGLTF_TYPE_SCALAR);

            for (size_t i = 0; i < buffer_view.byteLength;) {
                for (size_t j = 0; j < 3; j++) {
                    uint16_t value;
                    const std::array byte_array = {
                        // FIXME: Account for possible STRIDE
                        buffer.data.at(accessor.byteOffset + buffer_view.byteOffset + i++),
                        buffer.data.at(accessor.byteOffset + buffer_view.byteOffset + i++),
                    };

                    memcpy(&value, byte_array.data(), sizeof(uint16_t));

                    // Write vertex index to triangle
                    triangle.Vindex[j] = value;
                }

                // triangle.Attributes =
                m_triangles.push_back(triangle);
            }
        } else {
            // TODO: Load triangles linearly from m_vertices array
        }
    }

    return true;
}

bool W3dMesh::add_vertex_influences() {
    return false;
}

bool W3dMesh::add_vertex_shade_indices() {
    return false;
}

bool W3dMesh::add_material_info() {
    return false;
}

bool W3dMesh::add_vertex_materials() {
    return false;
}

bool W3dMesh::add_shaders() {
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
    write_user_text();
    write_vertices();
    write_vertex_normals();
    write_triangles();
    write_vertex_influences();
    write_vertex_shade_indices();
    write_material_info();
    write_vertex_materials();
    write_shaders();
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
    m_header.Attributes =
            W3D_MESH_FLAG_GEOMETRY_TYPE_NORMAL | W3D_MESH_FLAG_COLLISION_TYPE_PHYSICAL |
            W3D_MESH_FLAG_COLLISION_TYPE_PROJECTILE | W3D_MESH_FLAG_COLLISION_TYPE_CAMERA;
    m_header.NumTris = m_triangles.size();
    m_header.NumVertices = m_vertices.size();
    // FIXME: Check if this is the correct list for this count
    m_header.NumMaterials = m_vertex_material_info.size();
    m_header.NumDamageStages = 0;
    m_header.SortLevel = SORT_LEVEL_NONE;
    m_header.PrelitVersion = 0;
    m_header.FutureCounts[0] = 0;
    // FIXME: Dynamically generate value for this flag
    m_header.VertexChannels = W3D_VERTEX_CHANNEL_LOCATION | W3D_VERTEX_CHANNEL_NORMAL;
    // FIXME: Dynamically generate value for this flag
    m_header.FaceChannels = W3D_FACE_CHANNEL_FACE;

    compute_bounding_sphere();

    m_writer.begin_chunk(W3D_CHUNK_MESH_HEADER3);
    m_writer.write(&m_header, sizeof(m_header));
    m_writer.end_chunk(); // W3D_CHUNK_MESH_HEADER3

    return true;
}

bool W3dMesh::write_user_text() {
    return false;
}

bool W3dMesh::write_vertices() {
    if (m_vertices.empty())
        return false;

    m_writer.begin_chunk(W3D_CHUNK_VERTICES);
    for (auto &v: m_vertices)
        m_writer.write(&v, sizeof(IOVector3Struct));
    m_writer.end_chunk(); // W3D_CHUNK_VERTICES

    return true;
}

bool W3dMesh::write_vertex_normals() {
    if (m_normals.empty())
        return false;

    m_writer.begin_chunk(W3D_CHUNK_VERTEX_NORMALS);
    for (auto &v: m_normals)
        m_writer.write(&v, sizeof(IOVector3Struct));
    m_writer.end_chunk(); // W3D_CHUNK_VERTEX_NORMALS

    return true;
}

bool W3dMesh::write_triangles() {
    if (m_triangles.empty())
        return false;

    m_writer.begin_chunk(W3D_CHUNK_TRIANGLES);
    for (auto &v: m_triangles)
        m_writer.write(&v, sizeof(W3dTriStruct));
    m_writer.end_chunk(); // O_W3D_CHUNK_TRIANGLES

    return true;
}

bool W3dMesh::write_vertex_influences() {
    return false;
}

bool W3dMesh::write_vertex_shade_indices() {
    return false;
}

bool W3dMesh::write_material_info() {
    return false;
}

bool W3dMesh::write_vertex_materials() {
    return false;
}

bool W3dMesh::write_shaders() {
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

// Adapted from westwood's MeshBuilderClass
void W3dMesh::compute_bounding_sphere() {
    int i;
    double dx, dy, dz;

    // bounding sphere
    // Using the algorithm described in Graphics Gems I page 301.
    // This algorithm supposedly generates a bounding sphere within
    // 5% of the optimal one but is much faster and simpler to implement.
    Vector3 xmin(m_vertices[0].X, m_vertices[0].Y, m_vertices[0].Z);
    Vector3 xmax(m_vertices[0].X, m_vertices[0].Y, m_vertices[0].Z);
    Vector3 ymin(m_vertices[0].X, m_vertices[0].Y, m_vertices[0].Z);
    Vector3 ymax(m_vertices[0].X, m_vertices[0].Y, m_vertices[0].Z);
    Vector3 zmin(m_vertices[0].X, m_vertices[0].Y, m_vertices[0].Z);
    Vector3 zmax(m_vertices[0].X, m_vertices[0].Y, m_vertices[0].Z);

    // FIRST PASS:
    // finding the 6 minima and maxima points
    for (i = 1; i < m_vertices.size(); i++) {
        if (m_vertices[i].X < xmin.X) {
            xmin.X = m_vertices[i].X;
            xmin.Y = m_vertices[i].Y;
            xmin.Z = m_vertices[i].Z;
        }
        if (m_vertices[i].X > xmax.X) {
            xmax.X = m_vertices[i].X;
            xmax.Y = m_vertices[i].Y;
            xmax.Z = m_vertices[i].Z;
        }
        if (m_vertices[i].Y < ymin.Y) {
            ymin.X = m_vertices[i].X;
            ymin.Y = m_vertices[i].Y;
            ymin.Z = m_vertices[i].Z;
        }
        if (m_vertices[i].Y > ymax.Y) {
            ymax.X = m_vertices[i].X;
            ymax.Y = m_vertices[i].Y;
            ymax.Z = m_vertices[i].Z;
        }
        if (m_vertices[i].Z < zmin.Z) {
            zmin.X = m_vertices[i].X;
            zmin.Y = m_vertices[i].Y;
            zmin.Z = m_vertices[i].Z;
        }
        if (m_vertices[i].Z > zmax.Z) {
            zmax.X = m_vertices[i].X;
            zmax.Y = m_vertices[i].Y;
            zmax.Z = m_vertices[i].Z;
        }
    }

    // xspan = distance between the 2 points xmin and xmax squared.
    // same goes for yspan and zspan.
    dx = xmax.X - xmin.X;
    dy = xmax.Y - xmin.Y;
    dz = xmax.Z - xmin.Z;
    double xspan = dx * dx + dy * dy + dz * dz;

    dx = ymax.X - ymin.X;
    dy = ymax.Y - ymin.Y;
    dz = ymax.Z - ymin.Z;
    double yspan = dx * dx + dy * dy + dz * dz;

    dx = zmax.X - zmin.X;
    dy = zmax.Y - zmin.Y;
    dz = zmax.Z - zmin.Z;
    double zspan = dx * dx + dy * dy + dz * dz;

    // Set points dia1 and dia2 to the maximally separated pair
    // This will be the diameter of the initial sphere
    Vector3 dia1 = xmin;
    Vector3 dia2 = xmax;
    double maxspan = xspan;

    if (yspan > maxspan) {
        maxspan = yspan;
        dia1 = ymin;
        dia2 = ymax;
    }
    if (zspan > maxspan) {
        maxspan = zspan;
        dia1 = zmin;
        dia2 = zmax;
    }

    // Compute initial center and radius and radius squared
    Vector3 center;
    center.X = (dia1.X + dia2.X) / 2.0f;
    center.Y = (dia1.Y + dia2.Y) / 2.0f;
    center.Z = (dia1.Z + dia2.Z) / 2.0f;

    dx = dia2.X - center.X;
    dy = dia2.Y - center.Y;
    dz = dia2.Z - center.Z;

    double radsqr = dx * dx + dy * dy + dz * dz;
    double radius = sqrt(radsqr);

    // SECOND PASS:
    // Increment current sphere if any points fall outside of it.
    for (i = 0; i < m_vertices.size(); i++) {
        dx = m_vertices[i].X - center.X;
        dy = m_vertices[i].Y - center.Y;
        dz = m_vertices[i].Z - center.Z;

        double testrad2 = dx * dx + dy * dy + dz * dz;

        if (testrad2 > radsqr) {
            // this point was outside the old sphere, compute a new
            // center point and radius which contains this point
            double testrad = sqrt(testrad2);

            // adjust center and radius
            radius = (radius + testrad) / 2.0;
            radsqr = radius * radius;

            double oldtonew = testrad - radius;
            center.X = (radius * center.X + oldtonew * m_vertices[i].X) / testrad;
            center.Y = (radius * center.Y + oldtonew * m_vertices[i].Y) / testrad;
            center.Z = (radius * center.Z + oldtonew * m_vertices[i].Z) / testrad;
        }
    }

    m_header.SphCenter = {center.X, center.Y, center.Z};
    m_header.SphRadius = radius;
}
