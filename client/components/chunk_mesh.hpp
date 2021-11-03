/*
 * chunk_mesh.hpp
 * Copyright (c) 2021, Kirill GPRB.
 * All Rights Reserved.
 */
#pragma once
#include <client/render/gl/drawcmd.hpp>
#include <client/render/gl/vertexarray.hpp>

struct ChunkFlaggedForMeshingComponent final {};
struct ChunkMeshComponent final {
    gl::Buffer ibo, vbo;
    gl::VertexArray vao;
    gl::DrawCommand cmd;
};
