/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <math_constexpr.hpp>
#include <math_types.hpp>

constexpr static const size_t CHUNK_SIZE = 16;
constexpr static const size_t CHUNK_AREA = CHUNK_SIZE * CHUNK_SIZE;
constexpr static const size_t CHUNK_VOLUME = CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE;
constexpr static const size_t CHUNK_BITSHIFT = math::log2(CHUNK_SIZE);

using chunkpos_t = glm::vec<3, int64_t, glm::packed_highp>;
using voxelpos_t = glm::vec<3, int64_t, glm::packed_highp>;
using localpos_t = glm::vec<3, uint16_t, glm::packed_highp>;
using voxelidx_t = size_t;
using voxel_t = uint8_t;
