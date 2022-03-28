/*
 * Copyright (c) 2022 Kirill GPRB
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <inttypes.h>
#include <stddef.h>

// float-sized types
using matrix4f_t = glm::mat<4, 4, float, glm::packed_highp>;
using quaternf_t = glm::qua<float, glm::packed_highp>;
using vector2f_t = glm::vec<2, float, glm::packed_highp>;
using vector3f_t = glm::vec<3, float, glm::packed_highp>;
using vector4f_t = glm::vec<4, float, glm::packed_highp>;

// integer-sized types (world coordinates)
using vector3i16_t = glm::vec<3, int16_t, glm::packed_highp>;
using vector3i32_t = glm::vec<3, int32_t, glm::packed_highp>;
using vector3i64_t = glm::vec<3, int64_t, glm::packed_highp>;

// types for readability
using angle_t = float;
using hash_t = size_t;

// zero and null values for types
constexpr static const vector2f_t VECTOR2F_ZERO = { 0.0f, 0.0f };
constexpr static const vector3f_t VECTOR3F_ZERO = { 0.0f, 0.0f, 0.0f };
constexpr static const vector4f_t VECTOR4F_ZERO = { 0.0f, 0.0f, 0.0f, 0.0f };
constexpr static const angle_t ANGLE_ZERO = 0.0f;
constexpr static const hash_t HASH_NULL = 0;

// identity values - multiplying by
// this value shouldn't change anything
constexpr static const matrix4f_t MATRIX4F_IDENT = glm::identity<matrix4f_t>();
constexpr static const quaternf_t QUATERNF_IDENT = glm::identity<quaternf_t>();
constexpr static const vector2f_t VECTOR2F_IDENT = { 1.0f, 1.0f };
constexpr static const vector3f_t VECTOR3F_IDENT = { 1.0f, 1.0f, 1.0f };
constexpr static const vector4f_t VECTOR4F_IDENT = { 1.0f, 1.0f, 1.0f, 1.0f };

// direction vectors for 2D space
constexpr static const vector2f_t VECTOR2F_RIGHT = { 1.0f, 0.0f };
constexpr static const vector2f_t VECTOR2F_UP = { 0.0f, -1.0f };

// direction vectors for 3D space
constexpr static const vector3f_t VECTOR3F_FORWARD = { 0.0f, 0.0f, -1.0f };
constexpr static const vector3f_t VECTOR3F_RIGHT = { 1.0f, 0.0f, 0.0f };
constexpr static const vector3f_t VECTOR3F_UP = { 0.0f, 1.0f, 0.0f };

// commonly used angles
constexpr static const angle_t ANGLE_89D = glm::radians<angle_t>(89.0f);
constexpr static const angle_t ANGLE_90D = glm::radians<angle_t>(90.0f);
constexpr static const angle_t ANGLE_180D = glm::radians<angle_t>(180.0f);
constexpr static const angle_t ANGLE_360D = glm::radians<angle_t>(360.0f);
