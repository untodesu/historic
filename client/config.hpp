/*
 * Copyright (c) 2022 Kirill GPRB
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <common/config.hpp>
#include <common/types.hpp>

class ClientConfig final : public TomlConfig<ClientConfig> {
public:
    void impl_postRead();
    void impl_preWrite();

public:
    struct {
        int width {640};
        int height {480};
        bool fullscreen {false};
        bool vsync {true};
    } window;
};
