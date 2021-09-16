/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <memory>
#include <type_traits>

template<typename T>
struct NoPointer {
    using type = T;
};
template<typename T>
struct NoPointer<std::shared_ptr<T>> {
    using type = T;
};

template<typename T>
using no_pointer = typename NoPointer<T>::type;
