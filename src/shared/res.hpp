/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <math/types.hpp>
#include <traits.hpp>
#include <string>
#include <utility>
#include <vector>

namespace res
{
using priority_t = uint8_t;
constexpr static const priority_t ONE_SHOT = 0x00;
constexpr static const priority_t PRECACHE = 0xFF;

template<typename T>
struct Resource final {
    hash_t hash { HASH_ZERO };
    priority_t priority { ONE_SHOT };
    std::shared_ptr<no_pointer<T>> ptr;
};

template<typename T>
struct ResourceList final {
public:
    using vector_type = std::vector<Resource<T>>;
    vector_type data;

    inline Resource<T> *find(const hash_t hash)
    {
        for(typename vector_type::iterator it = data.begin(); it != data.end(); it++) {
            if(it->hash != hash)
                continue;
            return &(*it);
        }

        return nullptr;
    }

    inline size_t cleanup(priority_t priority)
    {
        size_t count = 0;
        for(typename vector_type::const_iterator it = data.cbegin(); it != data.cend(); it++) {
            if(it->priority > priority || it->ptr.use_count() > 1)
                continue;
            it = data.erase(it);
            count++;
        }

        return count;
    }
};

template<typename T>
size_t cleanup(priority_t priority);
template<typename T>
std::shared_ptr<no_pointer<T>> load(const std::string &name, priority_t priority);
template<typename T>
std::shared_ptr<no_pointer<T>> find(const std::string &name, bool complain);
} // namespace res
