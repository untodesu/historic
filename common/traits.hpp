/*
 * traits.hpp
 * Copyright (c) 2021, Kirill GPRB.
 * All Rights Reserved.
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
