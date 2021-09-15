/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <uvre/uvre.hpp>
#include <memory>

struct GenericMeshComponent final {
    std::shared_ptr<uvre::Buffer> vbo;
    std::shared_ptr<uvre::Buffer> ibo;
    std::shared_ptr<uvre::Texture> tex;
    size_t nv;
};
