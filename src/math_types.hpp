/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <stddef.h>
#include <stdint.h>

using float2_t = glm::vec<2, float, glm::packed_highp>;
using float3_t = glm::vec<3, float, glm::packed_highp>;
using float4_t = glm::vec<4, float, glm::packed_highp>;
using float4x4_t = glm::mat<4, 4, float, glm::packed_highp>;
using floatquat_t = glm::qua<float, glm::packed_highp>;

