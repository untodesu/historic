/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <uvre/uvre.hpp>

struct GenericMeshComponent {
    uvre::Buffer *vbo;
    uvre::Buffer *ibo;
    size_t nv;
};
